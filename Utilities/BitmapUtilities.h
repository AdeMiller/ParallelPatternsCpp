//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include <GdiPlus.h>
#include <string>
#include <assert.h>

#pragma once

namespace BitmapUtilities
{
    using namespace ::std;
    using namespace ::Gdiplus;

    void CopyBitmap(Bitmap* const source, Bitmap* const destination)
    {
        assert(nullptr != source);
        assert(nullptr != destination);
        assert(destination->GetWidth() == source->GetWidth());
        assert(destination->GetHeight() == source->GetHeight());

        Rect rect(0, 0, source->GetWidth(), source->GetHeight());
        BitmapData sourceBitmapData, destBitmapData;

        Status st = source->LockBits(
            &rect,
            ImageLockModeRead,
            PixelFormat32bppARGB,
            &sourceBitmapData);
        assert(Ok == st);

        destination->LockBits(
            &rect,
            ImageLockModeWrite,
            PixelFormat32bppARGB,
            &destBitmapData);
        assert(Ok == st);

        int size = source->GetHeight() * destBitmapData.Stride;

        int ret = memcpy_s(destBitmapData.Scan0, size, sourceBitmapData.Scan0, size);
        assert(0 == ret);
        source->UnlockBits(&sourceBitmapData);
        destination->UnlockBits(&destBitmapData);
    }

    int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
    {
        UINT  num = 0;          // number of image encoders
        UINT  size = 0;         // size of the image encoder array in bytes

        Gdiplus::ImageCodecInfo* pImageCodecInfo = nullptr;

        Gdiplus::GetImageEncodersSize(&num, &size);
        if (size == 0)
            return -1;  // Failure

        pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
        if (nullptr == pImageCodecInfo)
            return -1;  // Failure

        Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

        for (UINT j = 0; j < num; ++j)
        {
            if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
            {
                *pClsid = pImageCodecInfo[j].Clsid;
                free(pImageCodecInfo);
                return j;  // Success
            }    
        }

        free(pImageCodecInfo);
        return -1;  // Failure
    }

    void SaveBitmap(Bitmap* const result, const wstring& destDir, const wstring& fileName) 
    {
        CLSID clsid;
        EncoderParameters encoderParameters;
        encoderParameters.Count = 1;
        encoderParameters.Parameter[0].Guid = EncoderQuality;
        encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
        encoderParameters.Parameter[0].NumberOfValues = 1;
        ULONG quality = 100;
        encoderParameters.Parameter[0].Value = &quality;

        int ret = GetEncoderClsid(L"image/jpeg", &clsid);
        assert(-1 != ret);
        wstring path(destDir + L"\\" + fileName);
        Status st = result->Save(path.c_str(), &clsid, &encoderParameters);
        assert(Ok == st);
    }

    void SetAlpha(Bitmap* const pBitmap, byte alpha)
    {
        assert (pBitmap != nullptr);

        UINT width = pBitmap->GetWidth();
        UINT height = pBitmap->GetHeight();

        Rect rect(0,0,width, height);
        BitmapData bitmapData;

        //Lock the source bitmap for writing
        pBitmap->LockBits(
            &rect,
            ImageLockModeWrite,
            PixelFormat32bppARGB,
            &bitmapData);

        UINT* pixels = (UINT*)bitmapData.Scan0;

        for (UINT y = 0; y < pBitmap->GetHeight(); y++)
        {
            for (UINT x = 0; x < pBitmap->GetWidth(); x++)
            {
                UINT index = y * bitmapData.Stride / 4 + x;
                UINT cur = pixels[index];
                byte R = cur >> RED_SHIFT;
                byte G = cur >> GREEN_SHIFT;
                byte B = cur >> BLUE_SHIFT;
                pixels[index] = Color::MakeARGB(alpha, R, G, B);
            }
        }

        pBitmap->UnlockBits(&bitmapData);
    }

    void SetGray(Bitmap* const pBitmap)
    {
        assert (pBitmap != nullptr);

        UINT width = pBitmap->GetWidth();
        UINT height = pBitmap->GetHeight();

        Rect rect(0,0,width, height);
        BitmapData bitmapData;

        //Lock the source bitmap for writing
        pBitmap->LockBits(
            &rect,
            ImageLockModeWrite,
            PixelFormat32bppARGB,
            &bitmapData);

        UINT* pixels = (UINT*)bitmapData.Scan0;

        for (UINT y = 0; y < pBitmap->GetHeight(); y++)
        {
            for (UINT x = 0; x < pBitmap->GetWidth(); x++)
            {
                UINT index = y * bitmapData.Stride / 4 + x;
                UINT cur = pixels[index];
                byte A = cur;
                byte R = cur >> RED_SHIFT;
                byte G = cur >> GREEN_SHIFT;
                byte B = cur >> BLUE_SHIFT;
                byte luma = (byte)(0.3 * R + 0.56 * G + 0.11 * B);
                pixels[index] = Color::MakeARGB(A, luma, luma, luma);
            }
        }

        pBitmap->UnlockBits(&bitmapData);
    }
}
