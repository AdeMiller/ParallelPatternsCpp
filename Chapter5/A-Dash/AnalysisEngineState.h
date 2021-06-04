//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <string>
#include <agents.h>
#include <winuser.h>
#include <windef.h>
#include <assert.h>

// Custom message used to inform the dialog that the engine state has changed.

#define WM_UPDATEWINDOW (WM_USER + 1)

using namespace ::std;
using namespace ::Concurrency;

class AnalysisEngineState
{
private:
    HWND m_dialogWindow;
    overwrite_buffer<bool> m_isRunning;
    overwrite_buffer<bool> m_loadNyseCompleted;
    overwrite_buffer<bool> m_loadNasdaqCompleted;
    overwrite_buffer<bool> m_mergedMarketDataCompleted;
    overwrite_buffer<bool> m_loadFedHistoricalDataCompleted;
    overwrite_buffer<bool> m_normalizedMarketDataCompleted;
    overwrite_buffer<bool> m_normalizedHistoricalDataCompleted;
    overwrite_buffer<bool> m_analyzedStockDataCompleted;
    overwrite_buffer<bool> m_analyzedHistoricalDataCompleted;
    overwrite_buffer<bool> m_modelStockDataCompleted;
    overwrite_buffer<bool> m_modelHistoricalDataCompleted;
    overwrite_buffer<bool> m_compareModelsCompleted;
    overwrite_buffer<wstring> m_marketRecommendation;

public:
    AnalysisEngineState() :
        m_dialogWindow(NULL)
    {
        ResetFlags();
    }

    void Init(HWND dialog)
    {
        assert(dialog);
        m_dialogWindow = dialog;
    }

    void Reset()
    {
        assert(m_dialogWindow);
        ResetFlags();
        PostMessageW(m_dialogWindow, WM_UPDATEWINDOW, 0, 0);
    }

    void IsRunning()
    {
        SetBuffer(m_isRunning);
    }
    
    void IsStopped()
    {
        SetBuffer(m_isRunning, false);
    }

    bool GetIsRunning()
    {
        return m_isRunning.value();
    }

    void SetNyseCompleted()
    {
        SetBuffer(m_loadNyseCompleted);
    }

    bool GetNyseCompleted()
    {
        return m_loadNyseCompleted.value();
    }

    void SetNasdaqCompleted()
    {
        SetBuffer(m_loadNasdaqCompleted);
    }

    bool GetNasdaqCompleted()
    {
        return m_loadNasdaqCompleted.value();
    }

    void SetMergedMarketDataCompleted()
    {
        SetBuffer(m_mergedMarketDataCompleted);
    }

    bool GetMergedMarketDataCompleted()
    {
        return m_mergedMarketDataCompleted.value();
    }

    void SetLoadFedHistoricalDataCompleted()
    {
        SetBuffer(m_loadFedHistoricalDataCompleted);
    }

    bool GetLoadFedHistoricalDataCompleted()
    {
        return m_loadFedHistoricalDataCompleted.value();
    }

    void SetNormalizedMarketDataCompleted()
    {
        SetBuffer(m_normalizedMarketDataCompleted);
    }

    bool GetNormalizedMarketDataCompleted()
    {
        return m_normalizedMarketDataCompleted.value();
    }

    void SetNormalizedHistoricalDataCompleted()
    {
        SetBuffer(m_normalizedHistoricalDataCompleted);
    }

    bool GetNormalizedHistoricalDataCompleted()
    {
        return m_normalizedHistoricalDataCompleted.value();
    }

    void SetAnalyzedStockDataCompleted()
    {
        SetBuffer(m_analyzedStockDataCompleted);
    }

    bool GetAnalyzedStockDataCompleted()
    {
        return m_analyzedStockDataCompleted.value();
    }

    void SetAnalyzedHistoricalDataCompleted()
    {
        SetBuffer(m_analyzedHistoricalDataCompleted);
    }

    bool GetAnalyzedHistoricalDataCompleted()
    {
        return m_analyzedHistoricalDataCompleted.value();
    }

    void SetModelStockDataCompleted()
    {
        SetBuffer(m_modelStockDataCompleted);
    }

    bool GetModelStockDataCompleted()
    {
        return m_modelStockDataCompleted.value();
    }

    void SetModelHistoricalDataCompleted()
    {
        SetBuffer(m_modelHistoricalDataCompleted);
    }

    bool GetModelHistoricalDataCompleted()
    {
        return m_modelHistoricalDataCompleted.value();
    }

    void SetCompareModelsCompleted()
    {
        SetBuffer(m_compareModelsCompleted);
    }

    bool GetCompareModelsCompleted()
    {
        return m_compareModelsCompleted.value();
    }

    void SetMarketRecommendation(const wstring& value)
    {
        SetBuffer(m_marketRecommendation, value);
    }

    wstring GetMarketRecommendation()
    {
        if (m_isRunning.value())
            return(L"Calculating...");
        return m_marketRecommendation.value();
    }
private:
    void ResetFlags()
    {
        send(m_isRunning, false);
        send(m_loadNyseCompleted, false);
        send(m_loadNasdaqCompleted, false);
        send(m_mergedMarketDataCompleted, false);
        send(m_loadFedHistoricalDataCompleted, false);
        send(m_normalizedMarketDataCompleted, false);
        send(m_normalizedHistoricalDataCompleted, false);
        send(m_analyzedStockDataCompleted, false);
        send(m_analyzedHistoricalDataCompleted, false);
        send(m_modelStockDataCompleted, false);
        send(m_modelHistoricalDataCompleted, false);
        send(m_compareModelsCompleted, false);
        send(m_marketRecommendation, wstring(L""));
    }

    void SetBuffer(overwrite_buffer<bool>& buffer, bool value = true)
    {
        assert(m_dialogWindow);
        send(buffer, value);
        PostMessageW(m_dialogWindow, WM_UPDATEWINDOW, 0, 0);
    }

    void SetBuffer(overwrite_buffer<wstring>& buffer, wstring value)
    {
        assert(m_dialogWindow);
        send(buffer, value);
        PostMessageW(m_dialogWindow, WM_UPDATEWINDOW, 0, 0);
    }

    // Disable copy constructor and assignment.
    AnalysisEngineState(const AnalysisEngineState&);
    AnalysisEngineState const & operator=(AnalysisEngineState const&);
};