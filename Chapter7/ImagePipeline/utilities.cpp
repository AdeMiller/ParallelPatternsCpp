//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include "StdAfx.h"

#include <math.h>
#include <string>
#include <vector>
#include <random>
#include <assert.h>

#include "utilities.h"

namespace ImagePipeline
{
    using namespace ::std;

    double GaussianInverse(double value)
    {
        // Lower and upper breakpoints
        const double plow = 0.02425;
        const double phigh = 1.0 - plow;

        double p = (phigh < value) ? 1.0 - value : value;
        double sign = (phigh < value) ? -1.0 : 1.0;
        double q;

        if (p < plow)
        {
            // Rational approximation for tail
            double c[] = {-7.784894002430293e-03, -3.223964580411365e-01,
                                        -2.400758277161838e+00, -2.549732539343734e+00,
                                        4.374664141464968e+00, 2.938163982698783e+00};

            double d[] = {7.784695709041462e-03, 3.224671290700398e-01,
                                    2.445134137142996e+00, 3.754408661907416e+00};
            q = sqrt(-2 * log(p));
            return sign * (((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) /
                                            ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1);
        }
        else
        {
            // Rational approximation for central region
            double a[] = {-3.969683028665376e+01, 2.209460984245205e+02,
                                        -2.759285104469687e+02, 1.383577518672690e+02,
                                        -3.066479806614716e+01, 2.506628277459239e+00};

            double b[] = {-5.447609879822406e+01, 1.615858368580409e+02,
                                        -1.556989798598866e+02, 6.680131188771972e+01,
                                        -1.328068155288572e+01};
            q = p - 0.5;
            double  r = q * q;
            return (((((a[0] * r + a[1]) * r + a[2]) * r + a[3]) * r + a[4]) * r + a[5]) * q /
                                        (((((b[0] * r + b[1]) * r + b[2]) * r + b[3]) * r + b[4]) * r + 1);
        }
    }

    double GaussianInverse(double cumulativeDistribution, double mean, double standardDeviation)
    {
        assert(0.0 < cumulativeDistribution && cumulativeDistribution < 1.0);

        double result = GaussianInverse(cumulativeDistribution);
        return mean + result * standardDeviation;
    }

    double NextGaussianValue(double mean, double standardDeviation)
    {
        double x = 0.0;

        // get the next value in the interval (0, 1) from the underlying uniform distribution
        while (x == 0.0 || x == 1.0)
            x = float(rand()) / RAND_MAX;

        return GaussianInverse(x, mean, standardDeviation);
    }
};
