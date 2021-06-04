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
#include <string>
#include <tuple>
#include <gdiplus.h>

#include "PipelinePerformanceData.h"
#include "utilities.h"

namespace ImagePipeline
{
    class AgentBase;

    using namespace ::std;
    using namespace ::Gdiplus;

    class ImageInfo
    {
    private:
        int m_sequenceNumber;
        wstring m_fileName;
        shared_ptr<Bitmap> m_pBitmap;
        ImagePerformanceData m_currentImagePerformance;

    public:
        ImageInfo(int sequenceNumber, const wstring& fileName, Bitmap* const originalImage, const LARGE_INTEGER& clockOffset);

        Bitmap* GetBitmapPtr() const { return m_pBitmap.get(); }

        wstring GetName() const { return m_fileName; }

        int GetSequence() const { return m_sequenceNumber; }

        ImagePerformanceData GetPerformanceData() const { return m_currentImagePerformance; }

        void ResizeImage(const SIZE& size);

        void PhaseStart(int phase);

        void PhaseEnd(int phase);

        // Special case for first phase which creates ImageInfo after starting.
        void PhaseEnd(int phase, const LARGE_INTEGER& start);

    private:
        // Disable copy constructor and assignment.
        ImageInfo(const ImageInfo& rhs);
        ImageInfo const & operator=(ImageInfo const&);
    };

    typedef shared_ptr<ImageInfo> ImageInfoPtr;

    ImageInfoPtr CreateImage(const wstring& filePath, int sequence, const LARGE_INTEGER& clockOffset);

    void FilterImage(ImageInfoPtr pInfo, double noiseAmount);

    void ScaleImage(ImageInfoPtr pInfo, const SIZE& size);

    void DisplayImage(ImageInfoPtr pInfo, AgentBase* const agent);
};