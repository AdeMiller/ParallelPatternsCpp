//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include "stdafx.h"

#include "AgentBase.h"
#include "BitmapUtilities.h"
#include "ImageInfo.h"
#include "utilities.h"

namespace ImagePipeline
{
    using namespace BitmapUtilities;

    ImageInfo::ImageInfo(int sequenceNumber, const wstring& fileName, Bitmap* const originalImage, const LARGE_INTEGER& clockOffset) :
        m_sequenceNumber(sequenceNumber),
        m_fileName(fileName),
        m_pBitmap(nullptr),
        m_currentImagePerformance(sequenceNumber)
    {
        m_pBitmap = shared_ptr<Bitmap>(new Bitmap(originalImage->GetWidth(), originalImage->GetHeight(), PixelFormat24bppRGB));
        CopyBitmap(originalImage, m_pBitmap.get());
        m_currentImagePerformance.SetClockOffset(clockOffset);
    }

    void ImageInfo::ResizeImage(const SIZE& size)
    {
        if (nullptr == m_pBitmap.get())
        {
            m_pBitmap = shared_ptr<Bitmap>(new Bitmap(size.cx, size.cy, PixelFormat24bppRGB));
            return;
        }
        shared_ptr<Bitmap> pNewBitmap = shared_ptr<Bitmap>(new Bitmap(size.cx, size.cy, PixelFormat24bppRGB)); 
        Graphics graphics(pNewBitmap.get());
        graphics.DrawImage(m_pBitmap.get(), 0, 0, size.cx, size.cy);
        m_pBitmap = shared_ptr<Bitmap>(pNewBitmap);
    }

    void ImageInfo::PhaseStart(int phase)
    {
        m_currentImagePerformance.SetStartTick(phase);
    }

    void ImageInfo::PhaseEnd(int phase)
    {
        m_currentImagePerformance.SetEndTick(phase);
    }

    // Special case for first phase which creates ImageInfo after starting.
    void ImageInfo::PhaseEnd(int phase, const LARGE_INTEGER& start)
    {
        m_currentImagePerformance.SetStartTick(phase, start);
        m_currentImagePerformance.SetEndTick(phase);
    }

    ImageInfoPtr CreateImage(const wstring& filePath, int sequence, const LARGE_INTEGER& clockOffset)
    {
        LARGE_INTEGER start;
        QueryPerformanceCounter(&start);

        unique_ptr<Bitmap> temp = unique_ptr<Bitmap>(new Bitmap(filePath.c_str()));
        if (temp->GetWidth() == 0 || temp->GetHeight() == 0)
            AfxThrowFileException(CFileException::invalidFile, 0, filePath.c_str());
        unique_ptr<Bitmap> img = unique_ptr<Bitmap>(temp->Clone(0, 0, temp->GetWidth(), temp->GetHeight(), PixelFormat24bppRGB));
        size_t i = filePath.find_last_of(L"/\\");
        wstring name = filePath.substr(i + 1);
        ImageInfoPtr pInfo = ImageInfoPtr(new ImageInfo(sequence, name, img.get(), clockOffset));

        pInfo->PhaseEnd(kLoad, start);
        return pInfo;
    }

    void ScaleImage(ImageInfoPtr pInfo, const SIZE& size)
    {
        assert(nullptr != pInfo);

        pInfo->PhaseStart(kScale);

        pInfo->ResizeImage(size);
        Pen pen(Color::Black, 15.0);
        Graphics graphics(pInfo->GetBitmapPtr());
        graphics.DrawRectangle(&pen, 0, 0, size.cx, size.cy);

        pInfo->PhaseEnd(kScale);
    }

    UINT AddPixelNoise(UINT pixel, double noiseAmount)
    {
        byte A = pixel;
        byte R = pixel >> RED_SHIFT;
        byte G = pixel >> GREEN_SHIFT;
        byte B = pixel >> BLUE_SHIFT;

        int newR = R + (int)NextGaussianValue(0.0, noiseAmount);
        int newG = G + (int)NextGaussianValue(0.0, noiseAmount);
        int newB = B + (int)NextGaussianValue(0.0, noiseAmount);

        R = max(0, min(newR, 255));
        G = max(0, min(newG, 255));
        B = max(0, min(newB, 255));
        return Color::MakeARGB(A, R, G, B);
    }

    void FilterImage(ImageInfoPtr pInfo, double noiseAmount)
    {
        assert(nullptr != pInfo);

        pInfo->PhaseStart(kFilter);

        Bitmap* bitmap = pInfo->GetBitmapPtr();
        UINT width = bitmap->GetWidth();
        UINT height = bitmap->GetHeight();
        Rect rect(0, 0, width, height);
        BitmapData bitmapData;
        Status st = bitmap->LockBits(
            &rect,
            ImageLockModeWrite,
            PixelFormat24bppRGB,
            &bitmapData);
        assert(st == Ok);

        UINT* pixels = (UINT*)bitmapData.Scan0;

        for (UINT y = 0; y < bitmap->GetHeight(); y++)
        {
            for (UINT x = 0; x < bitmap->GetWidth(); x++)
            {
                UINT index = y * bitmapData.Stride / 4 + x;
                UINT cur = pixels[index];
                byte A = cur;
                byte R = cur >> RED_SHIFT;
                byte G = cur >> GREEN_SHIFT;
                byte B = cur >> BLUE_SHIFT;

                pixels[index] = AddPixelNoise(pixels[index], noiseAmount);
            }
        }

        bitmap->UnlockBits(&bitmapData);

        pInfo->PhaseEnd(kFilter);
    }

    void DisplayImage(ImageInfoPtr pInfo, AgentBase* const agent)
    {
        assert(nullptr != pInfo);

        pInfo->PhaseStart(kDisplay);

        agent->SetCurrentImage(pInfo);

        // Display phase ends in ImagePipelineDlg::OnPaint after the image has been drawn.
    }
};
