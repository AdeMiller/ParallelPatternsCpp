//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <memory>
#include <GdiPlus.h>
#include "resource.h"
#include "ApplicationCommandLineInfo.h"

using namespace ::Gdiplus;

class ImageBlenderDlg : public CDialogEx
{
private:
    wstring m_destDir;
    unique_ptr<Bitmap> m_image;

public:
    ImageBlenderDlg(const ApplicationCommandLineInfo& cmdInfo, CWnd* pParent = nullptr);
    enum { IDD = IDD_IMAGEBLENDER_DIALOG };
    const ApplicationCommandLineInfo& m_CmdInfo;

protected:
    HICON m_hIcon;
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

private:
    POINT ImageBlenderDlg::DrawImage();
};
