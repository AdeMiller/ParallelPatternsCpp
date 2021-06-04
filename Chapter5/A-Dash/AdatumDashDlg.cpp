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
#include "afxdialogex.h"

#include "AnalysisEngine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ADashDlg::ADashDlg(CWnd* pParent /*=nullptr*/) : 
    CDialogEx(ADashDlg::IDD, pParent),
    m_engineState(),
    m_isParallelWorkflow(false)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void ADashDlg::DoDataExchange(CDataExchange* pDX)
{
    // Workflow state buttons:

    GetDlgItem(IDC_BUTTON_NYSELOAD)->EnableWindow(m_engineState.GetNyseCompleted());
    GetDlgItem(IDC_BUTTON_NASDAQLOAD)->EnableWindow(m_engineState.GetNasdaqCompleted());
    GetDlgItem(IDC_BUTTON_MERGE)->EnableWindow(m_engineState.GetMergedMarketDataCompleted());
    GetDlgItem(IDC_BUTTON_HISTLOAD)->EnableWindow(m_engineState.GetLoadFedHistoricalDataCompleted());
    GetDlgItem(IDC_BUTTON_NORMALIZE)->EnableWindow(m_engineState.GetNormalizedMarketDataCompleted());
    GetDlgItem(IDC_BUTTON_HISTNORMALIZE)->EnableWindow(m_engineState.GetNormalizedHistoricalDataCompleted());
    GetDlgItem(IDC_BUTTON_HISTANALYZE)->EnableWindow(m_engineState.GetAnalyzedHistoricalDataCompleted());
    GetDlgItem(IDC_BUTTON_ANALYZE)->EnableWindow(m_engineState.GetAnalyzedStockDataCompleted());
    GetDlgItem(IDC_BUTTON_COMPARE)->EnableWindow(m_engineState.GetCompareModelsCompleted());
    GetDlgItem(IDC_BUTTON_MODEL)->EnableWindow(m_engineState.GetModelStockDataCompleted());
    GetDlgItem(IDC_BUTTON_HISTMODEL)->EnableWindow(m_engineState.GetModelHistoricalDataCompleted());

    CString str(L"");
    wstring recommendation = m_engineState.GetMarketRecommendation();
    if (recommendation.size() > 0)
        str.Append(&recommendation[0], recommendation.size());
    DDX_Text(pDX, IDC_EDIT_OUTPUT, str);

    // Application control buttons:

    GetDlgItem(IDC_CHECK_PARALLEL)->EnableWindow(!m_engineState.GetIsRunning());
    GetDlgItem(IDC_BUTTON_CALCULATE)->EnableWindow(!m_engineState.GetIsRunning());
    GetDlgItem(IDC_BUTTON_CANCEL)->EnableWindow(m_engineState.GetIsRunning());
    DDX_Check(pDX, IDC_CHECK_PARALLEL, (int&)m_isParallelWorkflow);
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ADashDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_CALCULATE, &ADashDlg::OnBnClickedButtonCalculate)
    ON_BN_CLICKED(IDC_BUTTON_CANCEL, &ADashDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BUTTON_EXIT, &ADashDlg::OnBnClickedButtonExit)

    ON_BN_CLICKED(IDC_BUTTON_NYSELOAD, &ADashDlg::OnBnClickedButtonNyseLoadResult)
    ON_BN_CLICKED(IDC_BUTTON_NASDAQLOAD, &ADashDlg::OnBnClickedButtonNasdaqLoadResult)
    ON_BN_CLICKED(IDC_BUTTON_MERGE, &ADashDlg::OnBnClickedButtonMergeResult)
    ON_BN_CLICKED(IDC_BUTTON_NORMALIZE, &ADashDlg::OnBnClickedButtonNormalizeResult)
    ON_BN_CLICKED(IDC_BUTTON_ANALYZE, &ADashDlg::OnBnClickedButtonAnalyzeResult)
    ON_BN_CLICKED(IDC_BUTTON_MODEL, &ADashDlg::OnBnClickedButtonModelResult)
    ON_BN_CLICKED(IDC_BUTTON_HISTLOAD, &ADashDlg::OnBnClickedButtonHistLoadResult)
    ON_BN_CLICKED(IDC_BUTTON_HISTNORMALIZE, &ADashDlg::OnBnClickedButtonHistNormalizeResult)
    ON_BN_CLICKED(IDC_BUTTON_HISTANALYZE, &ADashDlg::OnBnClickedButtonHistAnalyzeResult)
    ON_BN_CLICKED(IDC_BUTTON_HISTMODEL, &ADashDlg::OnBnClickedButtonHistModelResult)
    ON_BN_CLICKED(IDC_BUTTON_COMPARE, &ADashDlg::OnBnClickedButtonCompareResult)
    ON_MESSAGE(WM_UPDATEWINDOW, &ADashDlg::OnUpdateWindow)
END_MESSAGE_MAP()

BOOL ADashDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetIcon(m_hIcon, true);
    SetIcon(m_hIcon, false);

    m_engineState.Init(this->GetSafeHwnd());
    return true;
}

void ADashDlg::OnPaint()
{
    UpdateData(false);
    CDialogEx::OnPaint();
}

HCURSOR ADashDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

LRESULT ADashDlg::OnUpdateWindow(WPARAM wParam, LPARAM lParam)
{
    AfxGetMainWnd()->Invalidate();
    return 0L;
}

// Main button click event handlers

void ADashDlg::OnBnClickedButtonCalculate()
{
    UpdateData(true);

    m_backgroundWorker = unique_ptr<task_group>(new task_group());
    if (!m_isParallelWorkflow)
    {
        m_backgroundWorker->run([this](){
            AnalysisEngine engine(m_backgroundWorker.get());
            engine.DoAnalysisSequential(m_engineState);
        });
    }
    else
    {
        m_backgroundWorker->run([this](){
            AnalysisEngine engine(m_backgroundWorker.get());
            engine.DoAnalysisParallel(m_engineState);
        });
    }
}

void ADashDlg::OnBnClickedCancel()
{
    CancelAnalysis();
}

void ADashDlg::OnBnClickedButtonExit()
{
    CancelAnalysis();
    CDialogEx::OnCancel();
}

void ADashDlg::CancelAnalysis()
{
    if (nullptr != m_backgroundWorker)
    {
        m_backgroundWorker->cancel();
        m_backgroundWorker->wait();
        m_backgroundWorker = nullptr;
        m_engineState.IsStopped();
        UpdateData(true);
    }
}

#pragma region Workflow progress button click event handlers

void ADashDlg::OnBnClickedButtonNyseLoadResult()
{
    MessageBox(L"View NYSE market data.", L"NYSE");
}

void ADashDlg::OnBnClickedButtonNasdaqLoadResult()
{
    MessageBox(L"View NASDAQ market data.", L"NASDAQ");
}

void ADashDlg::OnBnClickedButtonMergeResult()
{
    MessageBox(L"View merged market data.", L"Merged");
}

void ADashDlg::OnBnClickedButtonNormalizeResult()
{
    MessageBox(L"View normalized market data.", L"Normalized");
}

void ADashDlg::OnBnClickedButtonAnalyzeResult()
{
    MessageBox(L"View market data analysis.", L"Analysis");
}

void ADashDlg::OnBnClickedButtonModelResult()
{
    MessageBox(L"View market model.", L"Model");
}

void ADashDlg::OnBnClickedButtonHistLoadResult()
{
    MessageBox(L"View Fed historical data.", L"Historical");
}

void ADashDlg::OnBnClickedButtonHistNormalizeResult()
{
    MessageBox(L"View normalized Fed historical data.", L"Normalized Historical");
}

void ADashDlg::OnBnClickedButtonHistAnalyzeResult()
{
    MessageBox(L"View historical analysis.", L"Historical Analysis");
}

void ADashDlg::OnBnClickedButtonHistModelResult()
{
    MessageBox(L"View historical model.", L"Historical Model");
}

void ADashDlg::OnBnClickedButtonCompareResult()
{
    MessageBox(L"View comparison and recommendation", L"Comparison");
}

#pragma endregion
