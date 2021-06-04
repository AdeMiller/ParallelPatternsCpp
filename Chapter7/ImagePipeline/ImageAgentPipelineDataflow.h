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
#include <ppl.h>
#include <agents.h>

#include "AgentBase.h"
#include "ImageInfo.h"
#include "ImagePipelineDlg.h"
#include "PipelineGovernor.h"
#include "utilities.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The ImageAgentPipelineDataFlow pipeline uses data flow agents to process message<ImageInfoPtr> 
// messages. The first stage of the pipeline is a control flow agent which generates data.

// To shutdown the pipeline the m_cancelMessage buffer is set to true. Once m_cancelMessage is true the 
// head of the pipeline stops generating data. Each pipeline stage passes messages but does no processing 
// on them and waits for a nullptr message before shutting down. Finally, the head of the pipeline waits 
// for all messages to be processed before shutting down. This ensures that all buffers are completely 
// empty on shutdown.

// If one of the stages throws an exception then the AgentBase::ShutdownOnError method will notify the UI and 
// send a cancel message to shutdown the pipeline.

// See: http://msdn.microsoft.com/en-us/library/ff601928.aspx

namespace ImagePipeline
{
    using namespace ::std;
    using namespace ::Concurrency;
    using namespace ::PipelineUtilities;

    class ImageAgentPipelineDataFlow : public AgentBase
    {
    private:
        PipelineGovernor m_governor;
        unique_ptr<transformer<ImageInfoPtr, ImageInfoPtr>> m_scaler;
        unique_ptr<transformer<ImageInfoPtr, ImageInfoPtr>> m_filterer;
        unique_ptr<call<ImageInfoPtr>> m_displayer;
        vector<LONG> m_queueSizes;
        SIZE m_imageDisplaySize;
        double m_noiseLevel;

    public:
        ImageAgentPipelineDataFlow(IImagePipelineDialog* const dialog, ISource<bool>& cancel, ITarget<ErrorInfo>& errorTarget, double noiseLevel) : AgentBase(dialog->GetWindow(), cancel, errorTarget),
            m_governor(GetPipelineCapacity()),
            m_scaler(nullptr),
            m_filterer(nullptr),
            m_displayer(nullptr),
            m_noiseLevel(noiseLevel)
        {
            m_queueSizes.resize(3, 0);
            m_imageDisplaySize = dialog->GetImageSize();
        }

        int GetQueueSize(int queue) const { return m_queueSizes[queue]; }

        void Initialize()
        {
            // Define pipeline transforms:

            // The transformers do not throw exceptions. Any exceptions are handled by the ScaleImage, FilterImage and DisplayImage methods.

            m_scaler = unique_ptr<transformer<ImageInfoPtr, ImageInfoPtr>>(
                new transformer<ImageInfoPtr, ImageInfoPtr>(
                [this](ImageInfoPtr pInfo)->ImageInfoPtr
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

            m_filterer = unique_ptr<transformer<ImageInfoPtr, ImageInfoPtr>>(
                new transformer<ImageInfoPtr, ImageInfoPtr>(
                [this](ImageInfoPtr pInfo)->ImageInfoPtr
                {
                    this->FilterImage(pInfo, m_noiseLevel);
                    return pInfo;
                }, 
                nullptr,
                [this](ImageInfoPtr pInfo)->bool
                {
                    InterlockedDecrement(&m_queueSizes[kLoaderToScaler]);
                    InterlockedIncrement(&m_queueSizes[kScalerToFilterer]);
                    return true;
                }
            ));

            m_displayer = unique_ptr<call<ImageInfoPtr>>(
                new call<ImageInfoPtr>(
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

            m_scaler->link_target(m_filterer.get());
            m_filterer->link_target(m_displayer.get());
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
                TRACE1("Load image: %d\n", sequence);
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
