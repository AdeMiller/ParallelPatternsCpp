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

#include "MarketModel.h"

using namespace ::std;

namespace BusinessObjects
{
    class MarketRecommendation
    {
    private:
        wstring m_recommendation;

    public:

        MarketRecommendation(wstring recommendation)
            : m_recommendation(L"")
        {
            m_recommendation = recommendation;
        }

        wstring GetValue() const
        {
            return m_recommendation;
        }
    };

    MarketRecommendation CreateRecommendation(const vector<MarketModel>& models)
    {
        return MarketRecommendation(L"Buy");
    }
}