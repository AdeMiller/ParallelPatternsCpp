//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include "stdafx.h"
#include <GdiPlus.h>
#include <concrt.h>

#include "ImagePipelineApp.h"
#include "ImagePipelineDlg.h"
#include "GdiContainer.h"

using namespace ::Gdiplus;
using namespace ::Concurrency;
using namespace ::ImagePipeline;
using namespace ::BitmapUtilities;

#pragma comment (lib,"Gdiplus.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(ImagePipelineApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

ImagePipelineApp theApp;

BOOL ImagePipelineApp::InitInstance()
{
#ifdef _DEBUG
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
    CWinApp::InitInstance();

    GdiContainer gdi;

    ImagePipelineDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();

    return FALSE;
}
