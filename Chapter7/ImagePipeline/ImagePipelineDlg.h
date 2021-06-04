//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <array>
#include "AgentBase.h"

using namespace ::std;
using namespace ::Gdiplus;
using namespace ::ImagePipeline;

class ImagePipelineDlg : public CDialogEx, IImagePipelineDialog
{
public:
    ImagePipelineDlg(CWnd* pParent = nullptr);

#pragma region IImagePipelineDialog implementation

    SIZE GetImageSize() const
    {
        SIZE size;
        size.cx = m_imageWidth;
        size.cy = m_imageHeight;
        return size;
    }

    HWND GetWindow() const { return this->m_hWnd; }

#pragma endregion

    enum { IDD = IDD_IMAGEPIPELINE_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

private:
    void DDX_TextFormatted(CDataExchange* pDX, int nIDC, double value, LPCTSTR format = _T("%4.1f"));

    int GetQueueSize(int queue) { return (nullptr == m_agent) ? 0 : m_agent->GetQueueSize(queue); }

    enum AgentType
    {
        kSequential = 0,
        kPipelineDataFlow = 1,
        kPipelineControlFlow = 2,
        kBalancedPipeline = 3
    };

    unique_ptr<AgentBase> m_agent;
    overwrite_buffer<bool> m_cancelMessage;
    unbounded_buffer<ErrorInfo> m_errorMessages;
    AgentType m_agentType;

    wstring m_imageName;
    ImagePerformanceData m_currentImagePerformance;
    PipelinePerformanceData m_pipelinePerformance;

    static const int m_imageTop = 7;
    static const int m_imageLeft = 7;
    static const int m_imageWidth = 840;
    static const int m_imageHeight = 700;

    void StopAgent();
    void SetButtonState(bool isRunning);
    void ReportError(const ErrorInfo& error);

protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedButtonStart();
    afx_msg void OnBnClickedButtonStop();
    afx_msg void OnBnClickedCancel();
    afx_msg LRESULT OnReportError(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnUpdateWindow(WPARAM wParam, LPARAM lParam);
};
