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
#include <ppl.h>

#include "AnalysisEngineState.h"

using namespace ::std;
using namespace ::Concurrency;

class ADashDlg : public CDialogEx
{
private: 
    HICON m_hIcon;
    unique_ptr<task_group> m_backgroundWorker;
    AnalysisEngineState m_engineState;

public:
    ADashDlg(CWnd* pParent = nullptr);	
    enum { IDD = IDD_ADATUMDASH_DIALOG };
    bool m_isParallelWorkflow;

private:
    virtual void DoDataExchange(CDataExchange* pDX);

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg LRESULT OnUpdateWindow(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedButtonQuit();
    afx_msg void OnBnClickedButtonCalculate();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedButtonCancel();
    afx_msg void OnBnClickedButtonExit();

    afx_msg void OnBnClickedButtonNyseLoadResult();
    afx_msg void OnBnClickedButtonNasdaqLoadResult();
    afx_msg void OnBnClickedButtonMergeResult();
    afx_msg void OnBnClickedButtonNormalizeResult();
    afx_msg void OnBnClickedButtonAnalyzeResult();
    afx_msg void OnBnClickedButtonModelResult();
    afx_msg void OnBnClickedButtonHistLoadResult();
    afx_msg void OnBnClickedButtonHistNormalizeResult();
    afx_msg void OnBnClickedButtonHistAnalyzeResult();
    afx_msg void OnBnClickedButtonHistModelResult();
    afx_msg void OnBnClickedButtonCompareResult();

    void CancelAnalysis();
};
