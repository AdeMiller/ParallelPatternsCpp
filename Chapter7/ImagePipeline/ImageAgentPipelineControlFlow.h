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
#include "PipelineGovernor.h"
#include "ImageInfo.h"
#include "ImagePipelineDlg.h"
#include "utilities.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The ImageAgentPipelineControlFlow pipeline uses control flow agents to process message<ImageInfoPtr> 
// messages.

// To shutdown the pipeline the m_cancelMessage buffer is set to true. Once m_cancelMessage is true the 
// head of the pipeline stops generating data. Each pipeline stage passes messages but does no processing 
// on them and waits for a nullptr message before shutting down. Finally, the head of the pipeline sends 
// a message of nullptr causing each agent's run() method to exit its message processing loop and call 
// done(). This ensures that all buffers are completely empty on shutdown.

// If one of the stages throws an exception then the AgentBase::ShutdownOnError method will notify the UI and 
// send a cancel message to shutdown the pipeline.

namespace ImagePipeline
{
    using namespace ::std;
    using namespace ::Concurrency;
    using namespace ::PipelineUtilities;

    class ImageScalerAgent : public agent
    {
    private:
        const AgentBase* m_pAgent;
        SIZE m_imageDisplaySize;
        vector<LONG>& m_queueSize; 
        ISource<ImageInfoPtr>& m_imageInput;
        ITarget<ImageInfoPtr>& m_imageOutput;

    public:
        ImageScalerAgent(const AgentBase* const pAgent, SIZE imageDisplaySize, vector<LONG>& queueSize, ISource<ImageInfoPtr>& imageInput, ITarget<ImageInfoPtr>& imageOutput) :
            m_pAgent(pAgent),
            m_imageDisplaySize(imageDisplaySize),
            m_queueSize(queueSize),
            m_imageInput(imageInput),
            m_imageOutput(imageOutput)
        {
        }

        void run()
        {
            ImageInfoPtr pInfo = nullptr;
            do
            {
                pInfo = receive(m_imageInput);
                m_pAgent->ScaleImage(pInfo, m_imageDisplaySize);
                asend(m_imageOutput, pInfo);
                InterlockedIncrement(&m_queueSize[kScalerToFilterer]);
                TRACE2("Scale image: %d %s\n", (nullptr == pInfo) ? kLastImageSentinel : pInfo->GetSequence(), m_pAgent->IsCancellationPending() ? L"(skipped)" : L"");
            }
            while(nullptr != pInfo);
            done();
        }
    };

    class ImageFiltererAgent : public agent
    {
    private:
        const AgentBase* const m_pAgent;
        double m_noiseLevel;
        vector<LONG>& m_queueSize; 
        ISource<ImageInfoPtr>& m_imageInput;
        ITarget<ImageInfoPtr>& m_imageOutput;

    public:
        ImageFiltererAgent(const AgentBase* const pAgent, double noiseLevel, vector<LONG>& queueSize, ISource<ImageInfoPtr>& imageInput, ITarget<ImageInfoPtr>& imageOutput) :
            m_pAgent(pAgent),
            m_noiseLevel(noiseLevel),
            m_queueSize(queueSize),
            m_imageInput(imageInput),
            m_imageOutput(imageOutput)
            {
            }

            void run()
            {
                ImageInfoPtr pInfo = nullptr;
                do
                {
                    pInfo = receive(m_imageInput);
                    InterlockedDecrement(&m_queueSize[kScalerToFilterer]);
                    m_pAgent->FilterImage(pInfo, m_noiseLevel);
                    asend(m_imageOutput, pInfo);
                    InterlockedIncrement(&m_queueSize[kFiltererToDisplayer]);
                    TRACE2("Filter image: %d %s\n", (nullptr == pInfo) ? kLastImageSentinel : pInfo->GetSequence(), m_pAgent->IsCancellationPending() ? L"(skipped)" : L"");
                }
                while(nullptr != pInfo);
                done();
            }
    };

    class ImageDisplayAgent : public agent
    {
    private:
        AgentBase* const m_pAgent;
        PipelineGovernor& m_governor;
        vector<LONG>& m_queueSize; 
        ISource<ImageInfoPtr>& m_imageInput;

    public:
        ImageDisplayAgent(AgentBase* const pAgent, PipelineGovernor& governor, vector<LONG>& queueSize, ISource<ImageInfoPtr>& imageInput) :
            m_pAgent(pAgent),
            m_governor(governor),
            m_queueSize(queueSize),
            m_imageInput(imageInput)
        {
        }

        void run()
        {
            ImageInfoPtr pInfo = nullptr;
            int imageProcessedCount = 0;
            do
            {
                pInfo = receive(m_imageInput);
                InterlockedDecrement(&m_queueSize[kFiltererToDisplayer]);
                m_pAgent->DisplayImage(pInfo);
                m_governor.FreePipelineSlot();
            }
            while(nullptr != pInfo);
            done();
        }
    };

    class ImageAgentPipelineControlFlow : public AgentBase
    {
    private:
        PipelineGovernor m_governor;
        vector<LONG> m_queueSizes;
        SIZE m_imageDisplaySize;
        double m_noiseLevel;

    public:
        ImageAgentPipelineControlFlow(IImagePipelineDialog* dialog, ISource<bool>& cancel, ITarget<ErrorInfo>& errorTarget, double noiseLevel) : AgentBase(dialog->GetWindow(), cancel, errorTarget),
            m_governor(GetPipelineCapacity()),
            m_noiseLevel(noiseLevel)
        {
            m_queueSizes.resize(3, 0);
            m_imageDisplaySize = dialog->GetImageSize();
        }

        int GetQueueSize(int queue) const { return m_queueSizes[queue]; }

        void run()
        {
            unbounded_buffer<ImageInfoPtr> buffer1;
            unbounded_buffer<ImageInfoPtr> buffer2;
            unbounded_buffer<ImageInfoPtr> buffer3;

            ImageScalerAgent imageScaler(this, m_imageDisplaySize, m_queueSizes, buffer1, buffer2);
            ImageFiltererAgent imageFilterer(this, m_noiseLevel, m_queueSizes, buffer2, buffer3);
            ImageDisplayAgent imageDisplayer(this, m_governor, m_queueSizes, buffer3);

            imageScaler.start();
            imageFilterer.start();
            imageDisplayer.start();

            vector<wstring> filenames = ListFilesInApplicationDirectory(L"jpg");
            assert(filenames.size() > 1);
            int sequence = kFirstImage;
            LARGE_INTEGER offset;
            QueryPerformanceCounter(&offset);

            for_each_infinite(filenames.cbegin(), filenames.cend(), 
                [this, offset, &buffer1, &sequence](wstring file)->bool
            {
                ImageInfoPtr pInfo = this->LoadImage(sequence++, file, offset);
                if (nullptr == pInfo) 
                    return true;
                // Don't push more data into the pipeline if it is already full to capacity.
                m_governor.WaitForAvailablePipelineSlot();
                asend(buffer1, pInfo);

                return IsCancellationPending();
            });

            // Send final nullptr through the pipeline to ensure all stages shutdown.
            TRACE0("Shutting down...\n");
            m_governor.WaitForEmptyPipeline();
            asend<ImageInfoPtr>(buffer1, nullptr);

            agent* agents[3] = { &imageScaler, &imageFilterer, &imageDisplayer };
            agent::wait_for_all(3, agents);
            done();
            TRACE("Shutdown complete\n");
       }
    };
};
