//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <numeric>
#include <algorithm>
#include <assert.h>

namespace CreditReview
{
    using namespace ::std;

    // Linear trend from slope and intercept. Predict y given any x value using the formula
    // y = slope * x + intercept.

    struct Trend
    {
    private:
        double m_slope;
        double m_intercept;
    public:
        Trend(double slope, double intercept): m_slope(slope), m_intercept(intercept) {}

        inline double Slope() const { return m_slope; }

        inline double Intercept() const { return m_intercept; }
    };

    // Predicts a y value given any x value using the formula y = slope * x + intercept.

    inline double PredictIntercept(const Trend& trend, double ordinate)
    {
        return trend.Slope() * ordinate + trend.Intercept(); 
    }

#pragma region Numerical Routines

    double Average(const vector<double>& in)
    {
        return accumulate(in.cbegin(), in.cend(), 0.0, std::plus<double>()) / static_cast<double>(in.size());
    }

    // Linear regression of (x, y) pairs

    inline Trend Fit(const vector<double>& abscissaValues, const vector<double>& ordinateValues)
    {
        assert(abscissaValues.size() == ordinateValues.size());
        assert(abscissaValues.size() > 1);

        double xx = 0, xy = 0;
        double abscissaMean = Average(abscissaValues);
        double ordinateMean = Average(ordinateValues);

        // calculate the sum of squared differences
        for (size_t i = 0; i < abscissaValues.size(); i++)
        {
            double xi = abscissaValues[i] - abscissaMean;
            xx += xi * xi;
            xy += xi * (ordinateValues[i] - ordinateMean);
        };

        assert(xx != 0.0);

        double slope = xy / xx;
        return Trend(slope, ordinateMean - slope * abscissaMean);
    }

    /// Linear regression with x-values given implicitly by the y-value indices

    inline Trend Fit(const vector<double>& ordinateValues)
    {
        assert(ordinateValues.size() > 0);

        // special case - x values are just the indices of the y's
        // create a vector containing the range of numbers from 0 to n
        vector<double> range(ordinateValues.size());
        int i = 0;
        generate(range.begin(), range.end(), [&i](){return ++i;});
        return Fit(range, ordinateValues);
    }

#pragma endregion
}
