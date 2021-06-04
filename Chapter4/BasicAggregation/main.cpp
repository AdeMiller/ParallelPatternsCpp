//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include <algorithm>
#include <vector>
#include <numeric>
#include <array>
#include <ppl.h>
#include <concurrent_vector.h>
#include "ppl_extras.h"

#include "SampleUtilities.h"

using namespace ::std;
using namespace ::SampleUtilities;
using namespace ::Concurrency::samples;
using namespace ::Concurrency::samples::details;

// Work function; calculate if a number is a prime

bool IsPrime(int n)
{
   if (n < 2)
      return false;
   for (int i = 2; i < n; ++i)
   {
      if ((n % i) == 0)
         return false;
   }
   return true;
}

// Functor (function operator) to wrap IsPrime()
// Lambdas can be used in place of functors in many cases.

struct IncrementIfPrime
{
    int operator()(int total, int element) const
    {
        return total + (IsPrime(element) ? 1 : 0);
    }
};

// The Basics

// A sequential aggregation using a for loop

int Example01(vector<int> sequence)
{
    int count = 0;
    for (size_t i = 0; i < sequence.size(); i++)
        count += IsPrime(sequence[i]) ? 1 : 0;

    // Note: Using STL the loop can be simplified to:
    // count = count_if(sequence.cbegin(), sequence.cend(), IsPrime);

    return count;
}

// A sequential aggregation using STL count_if and a lambda

int Example02(vector<int> sequence)
{
    return count_if(sequence.cbegin(), sequence.cend(), [](int i){ return IsPrime(i); });
}

// A sequential aggregation using STL accumulate and a functor

int Example02a(vector<int> sequence)
{
    return accumulate(sequence.cbegin(), sequence.cend(), 0, IncrementIfPrime());
}

// A simple parallel aggregation with parallel_for_each and a combinable object

int Example03(vector<int> sequence)
{
    // Create count and explicitly initialize its value to zero.
    combinable<int> count([]() { return 0; });     

    parallel_for_each(sequence.cbegin(), sequence.cend(), [&count](int i)
    {
        count.local() += IsPrime(i) ? 1 : 0;
    });
    return count.combine(plus<int>());
}

// Use the ConcRT Extras parallel_reduce and a functor

struct CountPrimes
{
    int operator()(vector<int>::const_iterator begin, vector<int>::const_iterator end, int right) const
    {
        return right + accumulate(begin, end, 0, IncrementIfPrime());
    }
};

int Example04(vector<int> sequence)
{
    // 3rd parameter is the initial value of the sum
    // 4th parameter is a (symmetric) function for accumulating the final total
    // 5th parameter is an (asymmetric) function for accumulating subtotals
    return parallel_reduce(sequence.cbegin(), sequence.cend(), 0, CountPrimes(), plus<int>());
}

// Use the ConcRT Extras parallel_count_if and a lambda

int Example04a(vector<int> sequence)
{
    // 3rd parameter is the initial value of the sum
    // 4th parameter is a (symmetric) function for accumulating the final total
    // 5th parameter is an (asymmetric) function for accumulating subtotals
    return parallel_count_if(sequence.cbegin(), sequence.cend(), [](int i){ return IsPrime(i); });
}

// Variations

// Sequential map-reduce

int Example05(vector<int> sequence)
{
    vector<bool> mapResult(sequence.size());

    // Create an intermediate result vector containing boolean flags for each prime number
    transform(sequence.cbegin(), sequence.cend(), mapResult.begin(), IsPrime);

    // Count all the true values in the result
    return accumulate(mapResult.cbegin(), mapResult.cend(), 0, plus<int>());
}

// Parallel map-reduce

struct TruesInRange
{
    int operator()(vector<bool>::const_iterator begin, vector<bool>::const_iterator end, const int& right) const
    {
        return right + accumulate(begin, end, 0, [](int lhs, bool rhs){ return lhs + (rhs ? 1 : 0); });
    }
};

int Example06(vector<int> sequence)
{
    vector<bool> mapResult(sequence.size());
    parallel_transform(sequence.begin(), sequence.end(), mapResult.begin(), [](int n){ return IsPrime(n); });

    return parallel_reduce(mapResult.cbegin(), mapResult.cend(), 0, TruesInRange(), plus<int>());
}

// Small loop bodies

// Sequential summation

int Example07(vector<int> sequence)
{
    int sum = 0;
    for (size_t i = 0; i < sequence.size(); i++)
        sum += sequence[i];
    return sum;
}

// Parallel summation with parallel_for_each - poor performance vs. sequential implementation

int Example08(vector<int> sequence)
{
    combinable<int> sum([](){ return 0; }); 

    // WARNING: Using parallel_for_each with very a small loop body is unlikely to give good performance
    parallel_for_each(sequence.cbegin(), sequence.cend(), [&sum](int i) 
    {
        sum.local() += i;
    });
    return sum.combine(plus<int>());
}

// Preserving result set ordering with parallel aggregation

int Example10(vector<int> sequence)
{
    combinable<vector<int>> primes([](){ return vector<int>(); });

    // For each subsequence of numbers create an ordered list of primes
    parallel_for_each(sequence.cbegin(), sequence.cend(), [&primes](int i)
    {
        if (IsPrime(i))
            primes.local().push_back(i);
    });

    // Combine ordered sub lists to form single ordered list
    vector<int> result = primes.combine([](vector<int> left, vector<int> right)->vector<int> 
    { 
        vector<int> result(left.size() + right.size());
        sort(left.begin(), left.end());
        sort(right.begin(), right.end());
        merge(left.begin(), left.end(), right.begin(), right.end(), result.begin()); 
        return result;
    });

    // Print some of the result set
    printf("  ");
    for (size_t i = 0; i < 5; ++i) { printf("%d,", result[i]); }
    printf("...");
    for (size_t i = result.size() - 5; i < result.size(); ++i) { printf("%d,", result[i]); }
    printf("\n");

    return result.size();
}

// Design notes

// WARNING: BUGGY CODE. Do not copy this method.
// This version will run *much slower* than the sequential version
int Example11(vector<int>& sequence)
{
    CRITICAL_SECTION cs;
    InitializeCriticalSectionAndSpinCount(&cs, 0x80000400);
    int count = 0;

    // BUG -- Do not use parallel_for_each
    parallel_for_each(sequence.cbegin(), sequence.cend(), [&count, &cs](int i)
    {
        // BUG -- Do not use locking inside of a parallel loop for aggregation
        EnterCriticalSection(&cs);
        // BUG -- Do not use shared variable for parallel aggregation
        count += IsPrime(i) ? 1 : 0;
        LeaveCriticalSection(&cs);
    });

    return count;
}

// WARNING: BUGGY CODE. Do not copy this method.
// This version will run *much slower* than the sequential version
int Example12(vector<int>& sequence)
{
    critical_section cs;
    int count = 0;

    // BUG -- Do not use parallel_for_each_fixed
    parallel_for_each_fixed(sequence.cbegin(), sequence.cend(), [&cs, &count](int i)
    {
        // BUG -- Do not use locking inside of a parallel loop for aggregation
        cs.lock();
        // BUG -- Do not use shared variable for parallel aggregation
        count += IsPrime(i) ? 1 : 0;
        cs.unlock();
    });

    return count;
}

int main()
{
    printf("Basic Aggregation Samples\n\n");
#if _DEBUG
    printf("For most accurate timing results, use Release build.\n\n");
#endif

    vector<int> sequence;
    sequence.resize(100000);
    int n = 1;
    generate(sequence.begin(), sequence.end(), [&n] { return n += 2; }); // Even numbers are never prime.

    cout << "Count Primes:" << endl << endl;

    TimedResult([&sequence](){ return Example01(sequence); }, "Sequential for loop   ");
    TimedResult([&sequence](){ return Example02(sequence); }, "Sequential count_if   ");
    TimedResult([&sequence](){ return Example02a(sequence); }, "Sequential accumulate ");
    TimedResult([&sequence](){ return Example03(sequence); }, "parallel_for_each     ");
    TimedResult([&sequence](){ return Example04(sequence); }, "parallel_reduce       ");
    TimedResult([&sequence](){ return Example04a(sequence); }, "parallel_count_if     ");
    cout << endl;
    TimedResult([&sequence](){ return Example05(sequence); }, "Sequential map-reduce ");
    TimedResult([&sequence](){ return Example06(sequence); }, "Parallel map-reduce   ");
    cout << endl;
    TimedResult([&sequence](){ return Example10(sequence); }, "Ordered result set    ");

    cout << endl;

    TimedResult([&sequence](){ return Example11(sequence); }, "parallel_for_each & CRITICAL_SECTION      ");
    TimedResult([&sequence](){ return Example12(sequence); }, "parallel_for_each_fixed & critical_section");

    cout << endl << "Calculate Sum:" << endl << endl;

    sequence.resize(100000000);
    fill(sequence.begin(), sequence.end(), 1); 

    TimedResult([&sequence](){ return Example07(sequence); }, "Sequential             ");
    TimedResult([&sequence](){ return Example08(sequence); }, "parallel_for_each      ");

    printf("\nRun complete... press enter to finish.");
    getchar();
}
