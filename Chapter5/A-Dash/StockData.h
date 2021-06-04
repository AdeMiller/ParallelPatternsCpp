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

using namespace ::std;

namespace BusinessObjects
{
    class StockData
    {
    private:
        string m_name;
        vector<double> m_priceHistory;

    public:
        StockData() : m_name("")
        {
        }

        StockData(string name, const vector<double>& priceHistory) : m_name("")
        {
            m_name = name;
            m_priceHistory = priceHistory;
        }
    };

    typedef vector<StockData> StockDataCollection;
}
