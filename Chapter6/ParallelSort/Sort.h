//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <vector>
#include <ppl.h>
#include <concrtrm.h>

#include "SampleUtilities.h"

namespace ParallelSort
{
    using namespace ::std;
    using namespace ::Concurrency;
    using namespace ::SampleUtilities;

    typedef vector<int>::iterator VectorIter;

    // To generate logarithms for other bases, use the mathematical relation: log base b of a == natural log (a) / natural log (b).
    inline float LogN(float value, float base)
    {
        return logf(value) / logf(base);
    }

    class Sort
    {
    public: 
        static void SequentialQuickSort(vector<int>& a, long threshold)
        {
            SequentialQuickSort(a.begin(), a.end(), threshold);
        }

        static void ParallelQuickSort(vector<int>& a, long threshold)
        {
            int maxTasks = 
                CurrentScheduler::Get()->GetNumberOfVirtualProcessors();
            ParallelQuickSort(a.begin(), a.end(), 
                (int)LogN(float(maxTasks), 2.0f) + 4, threshold);
        }

        static void ParallelQuickSortWithSTL(vector<int>& a, long threshold)
        {
            const int maxTasks = 
                CurrentScheduler::Get()->GetNumberOfVirtualProcessors();
            ParallelQuickSortWithSTL(a.begin(), a.end(), 
                (int)LogN((float)maxTasks, 2.0f) + 4, threshold);
        }

    private:  
        static void InsertionSort(VectorIter begin, VectorIter end)   
        {   
            VectorIter lowest = begin;
            for(VectorIter i = begin + 1; i < end; ++i )
                if (*i < *lowest)
                    lowest = i;

            iter_swap(begin, lowest);
            while(++begin < end)
                for(VectorIter j = begin; *j < *(j - 1); --j)
                    iter_swap((j - 1), j);
        }

        static void SequentialQuickSort(VectorIter begin, 
                                        VectorIter end, 
                                        long threshold)
        {
            if (distance(begin, end) <= threshold) 
            {
                InsertionSort(begin, end);
            }
            else
            {
                VectorIter pivot = partition(begin + 1, 
                                             end, 
                                             bind2nd(less<int>(), *begin));
                iter_swap(begin, pivot-1);
                SequentialQuickSort(begin, pivot - 1, threshold);
                SequentialQuickSort(pivot, end, threshold);
            }
        }

        static void ParallelQuickSort(VectorIter begin, VectorIter end, 
                                      long threshold, int depthRemaining)
        {
            if (distance(begin, end) <= threshold)
            {
                InsertionSort(begin, end);
            }
            else
            {
                VectorIter pivot = partition(begin + 1, 
                                             end, 
                                             bind2nd(less<int>(), *begin));
                iter_swap(begin, pivot-1);
                if (depthRemaining > 0)
                {
                    parallel_invoke(
                        [begin, end, pivot, depthRemaining, threshold] { 
                            Sort::ParallelQuickSort(begin, pivot - 1, 
                                                    depthRemaining - 1, threshold);
                    },
                        [&pivot, begin, end, depthRemaining, threshold] { 
                            Sort::ParallelQuickSort(pivot, end, 
                                                    depthRemaining - 1, threshold);
                    }
                    );
                }
                else
                {
                    SequentialQuickSort(begin, pivot - 1, threshold);
                    SequentialQuickSort(pivot, end, threshold);
                }
            }
        }

        static void ParallelQuickSortWithSTL(VectorIter begin, VectorIter end, int depthRemaining, long threshold)
        {
            if (distance(begin, end) <= threshold)
            {
                sort(begin, end);
            }
            else
            {
                VectorIter pivot = partition(begin, end, bind2nd(less<int>(), *begin));
                if (depthRemaining > 0)
                {
                    parallel_invoke(
                        [begin, end, pivot, depthRemaining, threshold] { 
                            if (pivot != end) 
                                Sort::ParallelQuickSortWithSTL(begin, pivot, depthRemaining - 1, threshold);
                    },
                        [&pivot, begin, end, depthRemaining, threshold] { 
                            if (pivot == begin) ++pivot;
                                Sort::ParallelQuickSortWithSTL(pivot, end, depthRemaining - 1, threshold);
                    }
                    );
                }
                else
                {
                    VectorIter pivot = partition(begin, end, bind2nd(less<int>(), *begin));
                    if (pivot != end) 
                        sort(begin, pivot);
                    if (pivot == begin) ++pivot;
                        sort(pivot, end);
                }
            }
        }

    };
}
