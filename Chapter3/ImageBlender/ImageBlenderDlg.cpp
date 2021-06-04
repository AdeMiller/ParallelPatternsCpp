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
#include <string>
#include "afxdialogex.h"

#include "ImageBlenderDlg.h"
#include "ImageBlender.h"

using namespace ::std;
using namespace ::Gdiplus;

ImageBlenderDlg::ImageBlenderDlg(const ApplicationCommandLineInfo& cmdInfo, CWnd* pParent /*=nullptr*/) : 
    CDialogEx(ImageBlenderDlg::IDD, pParent),
    m_CmdInfo(cmdInfo)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

BEGIN_MESSAGE_MAP(ImageBlenderDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()

BOOL ImageBlenderDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetIcon(m_hIcon, true);	
    SetIcon(m_hIcon, false);

    printf("Image Blender Sample\n");
#ifdef _DEBUG
    printf("For most accurate timing results, use Release build.\n");
#endif

    wstring destDir = !m_CmdInfo.DestDir().empty() ? m_CmdInfo.DestDir() : SampleUtilities::GetApplicationDirectory();
    wstring sourceDir = !m_CmdInfo.SourceDir().empty() ? m_CmdInfo.SourceDir() : SampleUtilities::GetApplicationDirectory();
    wstring file1 = !m_CmdInfo.File1().empty() ? m_CmdInfo.File1() : L"flowers.jpg";
    wstring file2 = !m_CmdInfo.File2().empty() ? m_CmdInfo.File2() : L"dog.jpg";

    wstring path1 = sourceDir + L"\\" + file1;
    wstring path2 = sourceDir + L"\\" + file2;

    if (!SampleUtilities::CheckDirectoryExists(sourceDir)) return false;
    if (!SampleUtilities::CheckFileExists(path1)) return false;
    if (!SampleUtilities::CheckFileExists(path2)) return false;
    if (!SampleUtilities::CheckDirectoryExists(m_destDir)) return false;

    ImageBlender::DoWork(path1, path2, destDir);

    wstring path = destDir + L"\\blended_paralleltasks.jpg";
    m_image = unique_ptr<Bitmap>(new Bitmap(path.c_str()));

    if (nullptr != m_image)
    {
        POINT size = DrawImage();
        RECT rect;
        ::GetWindowRect(m_hWnd, &rect);
        ::SetWindowPos(m_hWnd, nullptr, rect.left, rect.top, size.x + 50, size.y + 100, SWP_SHOWWINDOW);
        CWnd::UpdateWindow();
    }

    printf("\nClose image view window to exit program.");
    return true;
}

POINT ImageBlenderDlg::DrawImage()
{
    POINT size;
    size.x = size.y = 0;
    if (nullptr == m_image)
        return size;

    CPaintDC dc(this);
    BITMAP bm;
    HDC hdcMem = ::CreateCompatibleDC(dc);
    HBITMAP hb;
    m_image->GetHBITMAP(Color(0,0,0), &hb);
    HBITMAP hbmOld = (HBITMAP)::SelectObject(hdcMem, hb);
    ::GetObject(hb, sizeof(bm), &bm);
    BitBlt(dc, 7, 7, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
    size.x = bm.bmWidth;
    size.y = bm.bmHeight;
    ::SelectObject(hdcMem, hbmOld);
    ::DeleteDC(hdcMem);
    ::DeleteObject(hb);
    return size;
}

void ImageBlenderDlg::OnPaint()
{
    DrawImage();
    CDialogEx::OnPaint();
}

HCURSOR ImageBlenderDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}
