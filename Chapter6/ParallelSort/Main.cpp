//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include <random>
#include <vector>
#include <algorithm>
#include <ppl.h>

#include "Sort.h"
#include "SampleUtilities.h"

using namespace ::std;
using namespace ::Concurrency;
using namespace ::SampleUtilities;
using namespace ::ParallelSort;

#pragma region Helper methods

vector<int> MakeArray(int length, unsigned long seed)
{
    vector<int> a;
    a.resize(length);
    int i = 1;
    generate(a.begin(), a.end(), [&i] { return i++; } );
    random_shuffle(a.begin(), a.end());
    return a;
}

void PrintElements(const wstring& name, const vector<int>& a, size_t count)
{
    wprintf(L"%s\n", name.c_str());
    if (a.empty())
    {
        printf("[]\n");
        return;
    }
    if (a.size() < count)
        count = max(a.size() / 2, 1);
    printf("[ ");
    vector<int>::const_iterator itr;
    for (itr = a.cbegin(); itr != a.cbegin() + count / 2 ; ++itr )
        printf("%d ", *itr);
    printf("... ");
    for (itr = a.cend() - count / 2; itr != a.cend();  ++itr)
        printf("%d ", *itr);
    printf("]\n");
}

#pragma endregion

/// Command line arguments are:
///   length - of array to sort
///   threshold -  array length to use InsertionSort instead of SequentialQuickSort

int main(int argc, char* argv[])
{
    printf("Sort Sample\n\n");
#if _DEBUG
    printf("For most accurate timing results, use Release build.\n\n");
#endif

    const int seed = 4;     // defaults
#if _DEBUG
    long length = 400000; 
#else
    long length = 40000000;
#endif
    long threshold = 256;

    if (argc > 1)
        length = atoi(argv[1]);
    if (argc > 2)
        threshold = atoi(argv[2]);

    printf("\nCreating data: %d elements...\n", length);
    vector<int> a;
    a = MakeArray(length, seed);
    PrintElements(wstring(L"Initial: "), a, 8);
    TimedRun([&a, threshold]() { Sort::SequentialQuickSort(a, threshold); }, "Sequential");
    PrintElements(L"Result: ", a, 8);
    if (!is_sorted(a.cbegin(), a.cend()))
        printf("Error! Array not sorted.\n");

    printf("\n");
    a = MakeArray(length, seed);
    PrintElements(L"Initial: ", a, 8);
    TimedRun([&a, threshold]() { Sort::ParallelQuickSort(a, threshold); }, "  Parallel");
    PrintElements(L"Result: ", a, 8);
    if (!is_sorted(a.cbegin(), a.cend()))
        printf("Error! Array not sorted.\n");

    printf("\n");
    a = MakeArray(length, seed);
    PrintElements(L"Initial: ", a, 8);
    TimedRun([&a, threshold]() { Sort::ParallelQuickSortWithSTL(a, threshold); }, "  Parallel with std::sort");
    PrintElements(L"Result: ", a, 8);
    if (!is_sorted(a.cbegin(), a.cend()))
        printf("Error! Array not sorted.\n");

    printf("\nRun complete... press enter to finish.");
    getchar();
}
