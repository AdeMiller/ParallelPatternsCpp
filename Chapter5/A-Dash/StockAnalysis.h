//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

// This class is simply a placeholder to show that tasks in the graph can take
// different data types as inputs and outputs. They illustrate data moving through
// the model.

#pragma once

#include <string>
#include <vector>

#include "StockData.h"

using namespace ::std;

namespace BusinessObjects
{
    class StockAnalysis
    {
    private:
        string m_name;
        double m_volatility;

    public:

        StockAnalysis(string name, double volatility) :
            m_name(""),
            m_volatility(0.0)
        {
            m_name = name;
            m_volatility = volatility;
        }
    };

    typedef vector<StockAnalysis> StockAnalysisCollection;

    StockAnalysisCollection RunAnalysis(const StockDataCollection& data)
    {
        return StockAnalysisCollection(); 
    }
}
