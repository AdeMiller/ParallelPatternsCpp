//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include "stdafx.h"
#include "AdatumDashApp.h"
#include "AdatumDashDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(ADashApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

ADashApp::ADashApp()
{
}

ADashApp theApp;

BOOL ADashApp::InitInstance()
{
    CWinApp::InitInstance();
    ADashDlg dlg;
    m_pMainWnd = &dlg;

    dlg.DoModal();
    
    return false;
}
