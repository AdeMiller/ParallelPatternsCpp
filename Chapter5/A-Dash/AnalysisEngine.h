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
#include <vector>
#include <algorithm>
#include <sstream>
#include <ppl.h>

#include "concrt_extras.h"
#include "FuturesExample.h"
#include "SampleUtilities.h"

#include "StockData.h"
#include "MarketRecommendation.h"
#include "AnalysisEngineState.h"

using namespace ::std;
using namespace ::Concurrency;
using namespace ::Concurrency::samples;
using namespace ::BusinessObjects;
using namespace ::SampleUtilities;
using namespace ::FuturesExample;

class AnalysisEngine
{
private:
    double m_speedFactor;

    task_group* m_tasks;

#pragma region Create Sample Data

    StockDataCollection GenerateSecurities(const string& exchange, int size) const
    {
        StockDataCollection result;
        result.resize(size);
        int i = 0;
        generate(result.begin(), result.end(), [&exchange, &i]()->StockData 
        { 
            ostringstream oss;
            oss << exchange << " Stock " << i;
            string name = oss.str();
            vector<double> history;
            history.reserve(3);
            history.push_back(0.0);
            history.push_back(1.0);
            history.push_back(2.0);
            return StockData(name, history); 
        });
        return result;
    }

    StockDataCollection MakeNyseSecurityInfo() const
    {
        return GenerateSecurities("NYSE", 100);
    }

    StockDataCollection MakeNasdaqSecurityInfo() const
    {
        return GenerateSecurities("NASDAQ", 100);
    }

    StockDataCollection MakeFedSecurityInfo() const
    {
        return GenerateSecurities("", 100);
    }

#pragma endregion

#pragma region Analysis Helper Methods

    StockDataCollection LoadNyseData() const
    {
        DoIoIntensiveOperation(2500, *m_tasks);
        if (m_tasks->is_canceling())
            return StockDataCollection();
        return MakeNyseSecurityInfo();
    }

    StockDataCollection LoadNasdaqData() const
    {
        DoIoIntensiveOperation(2500, *m_tasks);
        if (m_tasks->is_canceling())
            return StockDataCollection();
        return MakeNasdaqSecurityInfo();
    }

    StockDataCollection LoadFedHistoricalData() const
    {
        DoIoIntensiveOperation(2500, *m_tasks);
        if (m_tasks->is_canceling())
            return StockDataCollection();
        return MakeFedSecurityInfo();
    }

    StockDataCollection MergeMarketData(const vector<StockDataCollection>& allMarketData) const
    {
        DoCpuIntensiveOperation(static_cast<DWORD>(2000 * m_speedFactor), m_tasks);
        StockDataCollection securities;

        if (!m_tasks->is_canceling())
        {
            for_each(allMarketData.cbegin(), allMarketData.cend(), [&securities](StockDataCollection data) 
            {
                size_t size = securities.size();
                securities.resize(size + data.size());
                copy(data.cbegin(), data.cend(), securities.begin() + size);
            });
        }

        if (m_tasks->is_canceling())
            return StockDataCollection();
        else
            return securities;
    }

    StockDataCollection NormalizeData(const StockDataCollection& marketData) const
    {
        DoCpuIntensiveOperation(static_cast<DWORD>(2000 * m_speedFactor), m_tasks);
        if (m_tasks->is_canceling())
            return StockDataCollection();
        else
            return StockDataCollection(marketData);
    }

    StockAnalysisCollection AnalyzeData(const StockDataCollection& data) const
    {
        if (m_tasks->is_canceling())
            return StockAnalysisCollection();
        return RunAnalysis(data);
    }

    MarketModel RunModel(const StockAnalysisCollection& data) const
    {
        DoCpuIntensiveOperation(static_cast<DWORD>(2000 * m_speedFactor), m_tasks);
        if (m_tasks->is_canceling())
            return MarketModel();
        else
            return CreateModel(data);
    }

    MarketRecommendation CompareModels(vector<MarketModel> models) const
    {
        DoCpuIntensiveOperation(static_cast<DWORD>(2000 * m_speedFactor), m_tasks);
        if (m_tasks->is_canceling())
            return MarketRecommendation(L"");
        else
            return CreateRecommendation(models);
    }

#pragma endregion

public:

    AnalysisEngine(task_group* tasks, double speed = 1.0)
        : m_speedFactor(speed),
        m_tasks(tasks)
    {
    }

    // Analysis Public Methods

    MarketRecommendation DoAnalysisSequential(AnalysisEngineState& engineState) const
    {
        engineState.Reset();
        engineState.IsRunning();
        vector<StockDataCollection> stockDatasets;
        vector<MarketModel> models;

        // Current market data tasks

        stockDatasets.push_back(LoadNyseData());
        if (!m_tasks->is_canceling()) engineState.SetNyseCompleted();

        stockDatasets.push_back(LoadNasdaqData());
        if (!m_tasks->is_canceling()) engineState.SetNasdaqCompleted();

        StockDataCollection mergedMarketData = MergeMarketData(stockDatasets);
        if (!m_tasks->is_canceling()) engineState.SetMergedMarketDataCompleted();

        StockDataCollection normalizedMarketData = NormalizeData(mergedMarketData);
        if (!m_tasks->is_canceling()) engineState.SetNormalizedMarketDataCompleted();

        StockAnalysisCollection analyzedStockData = AnalyzeData(normalizedMarketData);
        if (!m_tasks->is_canceling()) engineState.SetAnalyzedStockDataCompleted();

        models.push_back(RunModel(analyzedStockData));
        if (!m_tasks->is_canceling()) engineState.SetModelStockDataCompleted();

        // Historical data tasks

        StockDataCollection fedHistoricalData = LoadFedHistoricalData();
        if (!m_tasks->is_canceling()) engineState.SetLoadFedHistoricalDataCompleted();

        StockDataCollection normalizedHistoricalData = NormalizeData(fedHistoricalData);
        if (!m_tasks->is_canceling()) engineState.SetNormalizedHistoricalDataCompleted();

        StockAnalysisCollection analyzedHistoricalData = AnalyzeData(normalizedHistoricalData);
        if (!m_tasks->is_canceling()) engineState.SetAnalyzedHistoricalDataCompleted();

        models.push_back(RunModel(analyzedHistoricalData));
        if (!m_tasks->is_canceling()) engineState.SetModelHistoricalDataCompleted();

        // Compare results

        MarketRecommendation result = CompareModels(models);
        if (!m_tasks->is_canceling()) engineState.SetCompareModelsCompleted();
        if (!m_tasks->is_canceling()) engineState.SetMarketRecommendation(result.GetValue());
        engineState.IsStopped();
        return result;
    }

    MarketRecommendation DoAnalysisParallel(AnalysisEngineState& engineState) const
    {
        engineState.Reset();
        engineState.IsRunning();

        // Current market data tasks

        Future<StockDataCollection> future1([this, &engineState]()->StockDataCollection{ 
            scoped_oversubcription_token oversubscribeForIO;
            auto result = LoadNyseData();
            if (!m_tasks->is_canceling()) 
                engineState.SetNyseCompleted();
            return result;
        });

        Future<StockDataCollection> future2([this, &engineState]()->StockDataCollection{ 
            scoped_oversubcription_token oversubscribeForIO;
            auto result = LoadNasdaqData();
            if (!m_tasks->is_canceling()) 
                engineState.SetNasdaqCompleted();
            return result;
        });

        Future<StockDataCollection> future3([this, &engineState, &future1, &future2]()->StockDataCollection{
            vector<StockDataCollection> stockDatasets;
            stockDatasets.push_back(future1.Result());
            stockDatasets.push_back(future2.Result());
            auto result = this->MergeMarketData(stockDatasets);
            if (!m_tasks->is_canceling()) 
                engineState.SetMergedMarketDataCompleted();
            return result;
        });

        Future<StockDataCollection> future4 = Future<StockDataCollection>([this, &engineState, &future3]()->StockDataCollection{
            auto result = NormalizeData(future3.Result());
            if (!m_tasks->is_canceling()) 
                engineState.SetNormalizedMarketDataCompleted();
            return result;
        });

        Future<StockAnalysisCollection> future5 = Future<StockAnalysisCollection>([this, &engineState, &future4]()->StockAnalysisCollection{
            // For the Appendix B: The Parallel Tasks and Parallel Stacks Windows walkthrough set the breakpoint on the following line.
            auto result = AnalyzeData(future4.Result());
            if (!m_tasks->is_canceling()) 
                engineState.SetAnalyzedStockDataCompleted();
            return result;
        });

        Future<MarketModel> future6 = Future<MarketModel>([this, &engineState, &future5]()->MarketModel{
            auto result = RunModel(future5.Result());

            if (!m_tasks->is_canceling()) 
                engineState.SetModelStockDataCompleted();
            return result;
        });

        // Historical data tasks

        Future<StockDataCollection> future7 = Future<StockDataCollection>([this, &engineState]()->StockDataCollection{ 
            scoped_oversubcription_token oversubscribeForIO;
            auto result = LoadFedHistoricalData();
            if (!m_tasks->is_canceling()) 
                engineState.SetLoadFedHistoricalDataCompleted();
            return result;
        });

        Future<StockDataCollection> future8 = Future<StockDataCollection>([this, &engineState, &future7]()->StockDataCollection{
            auto result = NormalizeData(future7.Result());
            if (!m_tasks->is_canceling()) 
                engineState.SetNormalizedHistoricalDataCompleted();
            return result;
        });

        Future<StockAnalysisCollection> future9 = Future<StockAnalysisCollection>([this, &engineState, &future8]()->StockAnalysisCollection{
            auto result = AnalyzeData(future8.Result());
            if (!m_tasks->is_canceling()) 
                engineState.SetAnalyzedHistoricalDataCompleted();
            return result;
        });

        Future<MarketModel> future10 = Future<MarketModel>([this, &engineState, &future9]()->MarketModel{
            auto result = RunModel(future9.Result());
            if (!m_tasks->is_canceling()) 
                engineState.SetModelHistoricalDataCompleted();
            return result;
        });

        // Compare results

        vector<MarketModel> models;
        models.push_back(future6.Result());
        models.push_back(future10.Result());
        MarketRecommendation result = CompareModels(models);
        if (!m_tasks->is_canceling()) 
            engineState.SetCompareModelsCompleted();
        if (!m_tasks->is_canceling()) 
            engineState.SetMarketRecommendation(result.GetValue());
        engineState.IsStopped();
        return result;
    }
};
