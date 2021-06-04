//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#ifndef __AFXWIN_H__
    #error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"

class ImageBlenderApp : public CWinApp
{
public:
    ImageBlenderApp();
    virtual BOOL InitInstance();

    DECLARE_MESSAGE_MAP()
};

extern ImageBlenderApp theApp;
