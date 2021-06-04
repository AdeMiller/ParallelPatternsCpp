//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <vector>
#include <algorithm>
#include <queue>
#include <ppl.h>
#include <agents.h>

#include "AgentBase.h"
#include "PipelineGovernor.h"
#include "ImageInfo.h"
#include "ImagePipelineDlg.h"
#include "utilities.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The ImageAgentPipelineBalanced pipeline uses data flow agents to process message<ImageInfoPtr> 
// messages. The first stage of the pipeline is a control flow agent which generates data.

// To shutdown the pipeline the m_cancelMessage buffer is set to true. Once m_cancelMessage is true the 
// head of the pipeline stops generating data. Each pipeline stage passes messages but does no processing 
// on them and waits for a nullptr message before shutting down. Finally, the head of the pipeline waits 
// for all messages to be processed before shutting down. This ensures that all buffers are completely 
// empty on shutdown.

// If one of the stages throws an exception then the AgentBase::ShutdownOnError method will notify the UI and 
// send a cancel message to shut the pipeline.

namespace std 
{
    // Define ordering for multiplex_buffer.
    template<>
    struct greater<message<ImageInfoPtr>>
    {  
        bool operator()(const message<ImageInfoPtr>& lhs, const message<ImageInfoPtr>& rhs) const
        {  
            // Ensure that empty elements are ordered last.
            if (nullptr == lhs.payload)
                return true;
            if (nullptr == rhs.payload)
                return false;
            return lhs.payload->GetSequence() > rhs.payload->GetSequence();
        }
    };
};

// Functor for ordering ImageInfoPtr. Use by the multiplexer's priority queue.

struct CompareImageInfoPtr
{
    bool operator()(const ImageInfoPtr lhs, const ImageInfoPtr rhs) const
    {
        return (lhs->GetSequence() > rhs->GetSequence());
    }
};

namespace ImagePipeline
{
    using namespace ::std;
    using namespace ::Concurrency;
    using namespace ::PipelineUtilities;

    class ImageAgentPipelineBalanced : public AgentBase
    {
    private:
        unbounded_buffer<ImageInfoPtr> m_forkBuffer;
        unbounded_buffer<ImageInfoPtr> m_multiplexBuffer;
        int m_multiplexSequence;
        priority_queue<ImageInfoPtr, vector<ImageInfoPtr>, CompareImageInfoPtr> m_multiplexQueue;
        int m_imageProcessedCount;
        PipelineGovernor m_governor;
        unique_ptr<transformer<ImageInfoPtr, ImageInfoPtr>> m_scaler;
        vector<unique_ptr<transformer<ImageInfoPtr, ImageInfoPtr>>> m_filterers;
        unique_ptr<call<ImageInfoPtr>> m_multiplexer;
        unique_ptr<call<ImageInfoPtr>> m_displayer;
        vector<LONG> m_queueSizes;
        SIZE m_imageDisplaySize;
        double m_noiseLevel;
        int m_filterTaskCount;
        int m_nextFilter;

    public:
        ImageAgentPipelineBalanced(IImagePipelineDialog* dialog, ISource<bool>& cancel, ITarget<ErrorInfo>& errorTarget, double noiseLevel, int filterTaskCount) : AgentBase(dialog->GetWindow(), cancel, errorTarget),
            m_multiplexSequence(kFirstImage),
            m_imageProcessedCount(0),
            m_governor(GetPipelineCapacity() * 3),
            m_scaler(nullptr),
            m_multiplexer(nullptr),
            m_displayer(nullptr),
            m_noiseLevel(noiseLevel),
            m_filterTaskCount(filterTaskCount),
            m_nextFilter(0)
        {
            m_queueSizes.resize(3, 0);
            m_imageDisplaySize = dialog->GetImageSize();
            m_filterers.resize(filterTaskCount);
        }

        int GetQueueSize(int queue) const { return m_queueSizes[queue]; }

        void Initialize()
        {
            // Define pipeline transforms:

            m_scaler = unique_ptr<transformer<ImageInfoPtr, ImageInfoPtr>>(
                new transformer<ImageInfoPtr, ImageInfoPtr>(
                [this](ImageInfoPtr pInfo) ->ImageInfoPtr
                {
                    this->ScaleImage(pInfo, m_imageDisplaySize);
                    return pInfo;
                }, 
                nullptr,
                [this](ImageInfoPtr pInfo)->bool
                {
                    InterlockedIncrement(&m_queueSizes[kLoaderToScaler]);
                    return true;
                }
            ));

            for (int i = 0; i < m_filterTaskCount; ++i)
            {
                m_filterers[i] = unique_ptr<transformer<ImageInfoPtr, ImageInfoPtr>>(
                    new transformer<ImageInfoPtr, ImageInfoPtr>(
                    [this, i](ImageInfoPtr pInfo)->ImageInfoPtr
                    {
                        this->FilterImage(pInfo, m_noiseLevel);
                        return pInfo;
                    }, 
                    nullptr,
                    [this, i](ImageInfoPtr pInfo)->bool
                    {
                        // Use filter to reject some portion of images to load balance across filterers.
                        // This is very simplistic load balancing but without it all messages are accepted by the first transformer.
                        if ((nullptr != pInfo) && (i != m_nextFilter))
                            return false;
                        m_nextFilter = (int)((float(rand()) / RAND_MAX) * m_filterTaskCount);

                        // Now update the queue lengths.
                        InterlockedDecrement(&m_queueSizes[kLoaderToScaler]);
                        InterlockedIncrement(&m_queueSizes[kScalerToFilterer]);
                        return true;
                    }
                ));
            }

            m_multiplexer = unique_ptr<call<ImageInfoPtr>>(new call<ImageInfoPtr>(
                [this](ImageInfoPtr pInfo)
                {
                    m_multiplexQueue.push(pInfo);
                    while ((m_multiplexQueue.size() > 0) && (m_multiplexQueue.top()->GetSequence() == m_multiplexSequence))
                    {
                        TRACE1("Multiplexer: Sending %d\n", m_multiplexQueue.top()->GetSequence());
                        asend(m_multiplexBuffer, m_multiplexQueue.top());
                        m_multiplexQueue.pop();
                        ++m_multiplexSequence;
                    }
                }
            ));

            m_displayer = unique_ptr<call<ImageInfoPtr>>(new call<ImageInfoPtr>(
                [this](ImageInfoPtr pInfo)
                {
                    this->DisplayImage(pInfo);
                    InterlockedDecrement(&m_queueSizes[kFiltererToDisplayer]);
                    m_governor.FreePipelineSlot();
                },
                [this](ImageInfoPtr pInfo)->bool
                {
                    InterlockedDecrement(&m_queueSizes[kScalerToFilterer]);
                    InterlockedIncrement(&m_queueSizes[kFiltererToDisplayer]);
                    return true;
                }
            ));

            // Configure pipeline:

            m_scaler->link_target(&m_forkBuffer);

            for (int i = 0; i < m_filterTaskCount; ++i)
            {
                m_forkBuffer.link_target(m_filterers[i].get());
                m_filterers[i]->link_target(m_multiplexer.get());
            }

            m_multiplexBuffer.link_target(m_displayer.get());
        }

        void run()
        {
            // Initialize the dataflow agents.

            Initialize();

            // Start the control flow agent that is the start of the pipeline.

            vector<wstring> filenames = ListFilesInApplicationDirectory(L"jpg");
            assert(filenames.size() > 1);
            int sequence = kFirstImage;
            LARGE_INTEGER offset;
            QueryPerformanceCounter(&offset);

            for_each_infinite(filenames.cbegin(), filenames.cend(), 
                [this, offset, &sequence](wstring file)->bool
            {
                ImageInfoPtr pInfo = this->LoadImage(sequence++, file, offset);
                if (nullptr == pInfo) 
                    return true;
                // Don't push more data into the pipeline if it is already full to capacity.
                m_governor.WaitForAvailablePipelineSlot();
                asend(m_scaler.get(), pInfo);

                return IsCancellationPending();
            });

            // Wait for pipeline to empty before shutting down.
            TRACE1("Shutting down. Waiting for %d images...\n", sequence);
            m_governor.WaitForEmptyPipeline();
            done();
            TRACE("Shutdown complete\n");
        }
    };
};
