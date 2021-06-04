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

#include "StockAnalysis.h"

namespace BusinessObjects
{
    class MarketModel
    {
    public:
        MarketModel(void)
        {
        }
    };

    MarketModel CreateModel(const StockAnalysisCollection& data)
    {
        return MarketModel();
    }
}