//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <GdiPlus.h>

namespace BitmapUtilities
{
    // RAII container for GDI Plus.

    class GdiContainer
    {
    private:
        GdiplusStartupInput m_gdiplusStartupInput;
        ULONG_PTR m_gdiplusToken;

    public:
        GdiContainer() : m_gdiplusToken(NULL)
        {
            Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, nullptr);
        }

        ~GdiContainer()
        {
            Gdiplus::GdiplusShutdown(m_gdiplusToken);
        }

    private:
        // Hide assignment operator and copy constructor.
        GdiContainer const &operator =(GdiContainer const&);
        GdiContainer(GdiContainer const &);
    };
}
