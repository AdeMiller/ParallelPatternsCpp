//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include "stdafx.h"
#include <windows.h>
#include "ImageBlenderApp.h"
#include "ImageBlenderDlg.h"
#include "ConsoleOutput.h"
#include "GdiContainer.h"

using namespace ::BitmapUtilities;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(ImageBlenderApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

ImageBlenderApp::ImageBlenderApp()
{
}

ImageBlenderApp theApp;

BOOL CtrlHandler(DWORD fdwCtrlType) 
{
    return true;
}

BOOL ImageBlenderApp::InitInstance()
{
    CWinApp::InitInstance();
    GdiContainer gdi;
    ConsoleOutput console;

    ApplicationCommandLineInfo info;
    ParseCommandLine(info);

    ImageBlenderDlg dlg(info);
    dlg.DoModal();

    return false;
}
