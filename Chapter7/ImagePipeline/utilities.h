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
#include <GdiPlus.h>

namespace ImagePipeline
{
    using namespace ::std;
    using namespace ::Gdiplus;

    template<class InputIterator, class Func>
    Func for_each_infinite(InputIterator first, InputIterator last, Func f)
    {
        if (first == last)
            return f;

        InputIterator i = first;
        bool cancelled = false;
        do
        {
            if (i == last)
                i = first;
            cancelled = f(*i);
            ++i;
        }
        while (!cancelled);
        return f;
    }

    double NextGaussianValue(double mean, double standardDeviation);
};
