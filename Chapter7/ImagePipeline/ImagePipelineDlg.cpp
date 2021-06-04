//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include "stdafx.h"
#include <string>
#include <Gdiplus.h>
#include <assert.h>

#include "SampleUtilities.h"
#include "ImagePipelineApp.h"
#include "ImagePipelineDlg.h"
#include "ImageAgentSequential.h"
#include "ImageAgentPipelineDataflow.h"
#include "ImageAgentPipelineControlflow.h"
#include "ImageAgentPipelineBalanced.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace ::std;
using namespace ::Gdiplus;
using namespace ::ImagePipeline;

ImagePipelineDlg::ImagePipelineDlg(CWnd* pParent /*=nullptr*/) : CDialogEx(ImagePipelineDlg::IDD, pParent),
    m_agent(nullptr),
    m_cancelMessage(),
    m_agentType(kPipelineDataFlow),
    m_imageName(L""),
    m_pipelinePerformance(),
    m_currentImagePerformance(0)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void ImagePipelineDlg::DDX_TextFormatted(CDataExchange* pDX, int nIDC, double value, LPCTSTR format)
{
    CString tmp;
    tmp.Format(format, value); 
    DDX_Text(pDX, nIDC, tmp);
}

void ImagePipelineDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

    DDX_Radio(pDX, IDC_RADIO_AGENT, (int&)m_agentType);
    CString str(L"");
    if (m_imageName.size() > 0)
        str.Append(&m_imageName[0], m_imageName.size());
    DDX_Text(pDX, IDC_EDIT_IMAGENAME, str);
    DDX_TextFormatted(pDX, IDC_EDIT_IMAGECOUNT, m_pipelinePerformance.GetImageCount(), L"%3.0f");
    DDX_TextFormatted(pDX, IDC_EDIT_LOADTIME, m_pipelinePerformance.GetAveragePhaseTime(kLoad));
    DDX_TextFormatted(pDX, IDC_EDIT_FILTERTIME, m_pipelinePerformance.GetAveragePhaseTime(kFilter));
    DDX_TextFormatted(pDX, IDC_EDIT_RESIZETIME, m_pipelinePerformance.GetAveragePhaseTime(kScale));
    DDX_TextFormatted(pDX, IDC_EDIT_DISPLAYTIME, m_pipelinePerformance.GetAveragePhaseTime(kDisplay));
    DDX_TextFormatted(pDX, IDC_EDIT_QUEUE1_WAIT, m_pipelinePerformance.GetAverageQueueTime(kLoaderToScaler));
    DDX_TextFormatted(pDX, IDC_EDIT_QUEUE2_WAIT, m_pipelinePerformance.GetAverageQueueTime(kScalerToFilterer));
    DDX_TextFormatted(pDX, IDC_EDIT_QUEUE3_WAIT, m_pipelinePerformance.GetAverageQueueTime(kFiltererToDisplayer));
    DDX_TextFormatted(pDX, IDC_EDIT_TIMEPERIMAGE, m_pipelinePerformance.GetTimePerImage());
    DDX_TextFormatted(pDX, IDC_EDIT_QUEUE1_SIZE, GetQueueSize(kLoaderToScaler), L"%3.0f");
    DDX_TextFormatted(pDX, IDC_EDIT_QUEUE2_SIZE, GetQueueSize(kScalerToFilterer), L"%3.0f");
    DDX_TextFormatted(pDX, IDC_EDIT_QUEUE3_SIZE, GetQueueSize(kFiltererToDisplayer), L"%3.0f");
}

BEGIN_MESSAGE_MAP(ImagePipelineDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_START, &ImagePipelineDlg::OnBnClickedButtonStart)
    ON_BN_CLICKED(IDC_BUTTON_STOP, &ImagePipelineDlg::OnBnClickedButtonStop)
    ON_BN_CLICKED(IDCANCEL, &ImagePipelineDlg::OnBnClickedCancel)
    ON_MESSAGE(WM_REPORTERROR, &ImagePipelineDlg::OnReportError)
    ON_MESSAGE(WM_UPDATEWINDOW, &ImagePipelineDlg::OnUpdateWindow)
END_MESSAGE_MAP()

BOOL ImagePipelineDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetIcon(m_hIcon, true);
    SetIcon(m_hIcon, false);
    SetButtonState(false);
    return TRUE;
}

void ImagePipelineDlg::OnPaint()
{
    if (nullptr != m_agent)
    {
        ImageInfoPtr pInfo = m_agent->GetCurrentImage();
        Bitmap* pImage = pInfo->GetBitmapPtr();
        if (nullptr != pImage)
        {
            CPaintDC dc(this);
            BITMAP bm;
            HDC hdcMem = ::CreateCompatibleDC(dc);
            HBITMAP hb;
            pImage->GetHBITMAP(Color(0,0,0), &hb);
            HBITMAP hbmOld = (HBITMAP)::SelectObject(hdcMem, hb);
            ::GetObject(hb, sizeof(bm), &bm);
            BitBlt(dc, m_imageLeft, m_imageTop, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
            ::SelectObject(hdcMem, hbmOld);
            ::DeleteDC(hdcMem);
            ::DeleteObject(hb);
        }
        m_imageName = pInfo->GetName();
        m_currentImagePerformance = pInfo->GetPerformanceData();
        // The display phase actually ends here. Right before updating the pipeline's performance
        m_currentImagePerformance.SetEndTick(kDisplay);
        m_pipelinePerformance.Update(m_currentImagePerformance);
        UpdateData(false);
    }
    CDialogEx::OnPaint();
}

HCURSOR ImagePipelineDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

// Custom message handlers:

LRESULT ImagePipelineDlg::OnReportError(WPARAM wParam, LPARAM lParam)
{
    ErrorInfo err = receive(m_errorMessages);
    ReportError(err);
    return 0L;
}

LRESULT ImagePipelineDlg::OnUpdateWindow(WPARAM wParam, LPARAM lParam)
{   
    AfxGetMainWnd()->Invalidate(false);

    return 0L;
}

// Button event handlers:

void ImagePipelineDlg::OnBnClickedButtonStart()
{
    const double noiseLevel = 50.0;
    UpdateData(true);
    StopAgent();
    send(m_cancelMessage, false);

    switch(m_agentType)
    {
    case kSequential:
        m_agent = unique_ptr<AgentBase>(static_cast<AgentBase*>(new ImageAgentSequential(this, m_cancelMessage, m_errorMessages, noiseLevel)));
        break;
    case kPipelineDataFlow:
        m_agent = unique_ptr<AgentBase>(static_cast<AgentBase*>(new ImageAgentPipelineDataFlow(this, m_cancelMessage, m_errorMessages, noiseLevel)));
        break;
    case kPipelineControlFlow:
        m_agent = unique_ptr<AgentBase>(static_cast<AgentBase*>(new ImageAgentPipelineControlFlow(this, m_cancelMessage, m_errorMessages, noiseLevel)));
        break;
    case kBalancedPipeline:
        m_agent = unique_ptr<AgentBase>(static_cast<AgentBase*>(new ImageAgentPipelineBalanced(this, m_cancelMessage, m_errorMessages, noiseLevel, 8)));
        break;
    default:
        assert(false);
    }

    m_pipelinePerformance.Reset();
    if (nullptr != m_agent)
    {
        m_agent.get()->start();
        m_pipelinePerformance.Start();
    }
    SetButtonState(true);
}

void ImagePipelineDlg::OnBnClickedButtonStop()
{
    StopAgent();
    SetButtonState(false);
    Invalidate(false);
}

void ImagePipelineDlg::OnBnClickedCancel()
{
    StopAgent();
    CDialogEx::OnCancel();
}

// Helper methods:

void ImagePipelineDlg::StopAgent()
{
    if (nullptr == m_agent)
        return;
    send(m_cancelMessage, true);
    agent::wait(m_agent.get());
    m_agent = nullptr;
}

void ImagePipelineDlg::SetButtonState(bool isRunning)
{
    bool isEnabled = ListFilesInApplicationDirectory(L"jpg").size() > 1;

    GetDlgItem(IDC_BUTTON_START)->EnableWindow(!isRunning && isEnabled);
    GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(isRunning && isEnabled);
    GetDlgItem(IDC_RADIO_AGENT)->EnableWindow(!isRunning && isEnabled);
    GetDlgItem(IDC_RADIO_AGENT1)->EnableWindow(!isRunning && isEnabled);
    GetDlgItem(IDC_RADIO_AGENT2)->EnableWindow(!isRunning && isEnabled);
    GetDlgItem(IDC_RADIO_AGENT3)->EnableWindow(!isRunning && isEnabled);
}

void ImagePipelineDlg::ReportError(const ErrorInfo& error)
{
    array<wstring, 5> phaseNames = { L"loading", L"scaling", L"filtering", L"displaying", L"processing" };

    SetButtonState(false);

    wstring message(L"Error while ");
    message.append(phaseNames[get<0>(error)]).append(L" image");
    wstring imageName = get<1>(error);
    if (!imageName.empty())
        message.append(L" \"").append(imageName).append(L"\"");
    message.append(L"\n\nException message is \"").append(get<2>(error)).append(L"\"");

    TRACE1("Exception: '%s'\n", get<2>(error));
    AfxMessageBox(message.c_str(), MB_ICONERROR | MB_OK);
}
