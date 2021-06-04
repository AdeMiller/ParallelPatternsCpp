//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <agents.h>
#include <algorithm>
#include <string>

#include "AgentBase.h"
#include "ImagePipelineDlg.h"
#include "ImageInfo.h"
#include "utilities.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The ImageAgentSequential pipeline uses a single control flow agents to process data.
// messages. The first stage of the pipeline is a control flow agent which generates data.

// To shutdown the agent the m_cancelMessage buffer is set to true. Once m_cancelMessage is true the 
// for_each_infinite exits and the agent calls done(). 

// If an exception is thrown then the AgentBase::ShutdownOnError method will notify the UI and 
// send a cancel message to shutdown the agent.

namespace ImagePipeline
{
    using namespace ::std;
    using namespace ::Gdiplus;
    using namespace ::SampleUtilities;

    class ImageAgentSequential : public AgentBase
    {
    private:
        SIZE m_imageDisplaySize;
        double m_noiseLevel;

    public:
        ImageAgentSequential(IImagePipelineDialog* dialog, ISource<bool>& cancel, ITarget<ErrorInfo>& errorTarget, double noiseLevel) : AgentBase(dialog->GetWindow(), cancel, errorTarget),
            m_noiseLevel(noiseLevel)
        {
            m_imageDisplaySize = dialog->GetImageSize();
        }

        // Queue size is meaningless in the sequential case.
        int GetQueueSize(int queue) const { return 0; }
    
        void run()
        {
            vector<wstring> filenames = ListFilesInApplicationDirectory(L"jpg");
            assert(filenames.size() > 1);

            int sequence = kFirstImage;
            LARGE_INTEGER offset;
            QueryPerformanceCounter(&offset);

            for_each_infinite(filenames.cbegin(), filenames.cend(), 
                [this, &sequence, offset](wstring file)->bool
            {
                ImageInfoPtr pInfo = this->LoadImage(sequence++, file, offset);
                this->ScaleImage(pInfo, m_imageDisplaySize);
                this->FilterImage(pInfo, m_noiseLevel);
                this->DisplayImage(pInfo);
                TRACE1("Process image: %d\n", (nullptr == pInfo) ? kLastImageSentinel : pInfo->GetSequence());
                return IsCancellationPending();
            });
            done();
            TRACE("Shutdown complete\n");
        }
    };
};