//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include <exception>
#include <string>
#include <iosfwd>
#include <ppl.h>
#include <concrt.h>
#include <concrtrm.h>
#include <math.h>
#include <vector>
#include <algorithm>

#include "SampleUtilities.h"

volatile bool g_SimulateInternalError;

using namespace ::std;
using namespace ::Concurrency;
using namespace ::SampleUtilities;

class ParallelForExampleException : exception {};
class InvalidValueFoundException : exception {};

#pragma region Worker methods

double round(double value, int decimals)
{
    double exp = pow(double(10.0), decimals);
    return floor(value * exp) / exp;
}

double DoWork(int i, int workLoad)
{
    double result = 0;
    for (int j = 1; j < workLoad + 1; ++j)
    {
        double j2 = (double)j;
        double i2 = (double)i;
        result += sqrt(((double)9.0 * i2 * i2 + (double)16.0 * i * i) * j2 * j2);
    }

    // Simulate unexpected condition in loop body
    if ((i % 402030 == 2029) && g_SimulateInternalError)
        throw ParallelForExampleException();

    return round(result, 1);
} 

double ExpectedResult(int i, int workLoad) 
{
    return (double)2.5 * (workLoad + 1) * workLoad * i;
}

void VerifyResult(const vector<double>& values, int workLoad) 
{
    for (unsigned int i = 0; i < values.size(); ++i) 
    {
        if (values[i] != ExpectedResult(i, workLoad))
            throw InvalidValueFoundException();
    }
}

#pragma endregion

// Sequential for loop

void Example01(vector<double>& results, int workLoad)
{
    size_t n = results.size(); 
    for (size_t i = 0; i < n; ++i)
    {
        results[i] = DoWork(i, workLoad);
    }
}

// Parallel for loop

void Example02(vector<double>& results, int workLoad)
{
    size_t n = results.size();
    parallel_for(0u, n, [&results, workLoad](size_t i) 
    {
        results[i] = DoWork(i, workLoad); 
    });
}

// Sequential for each loop

void Example03(size_t size, int workLoad)
{
    // Create input values
    vector<size_t> inputs(size);
    for (size_t i = 0; i < size; ++i) 
        inputs[i] = i;

    for_each(inputs.cbegin(), inputs.cend(), [workLoad](size_t i){
        DoWork(i, workLoad);
    });
}

// Parallel for each loop

void Example04(size_t size, int workLoad)
{
    // Create input values
    vector<size_t> inputs(size);
    for (size_t i = 0; i < size; ++i) 
        inputs[i] = i;

    parallel_for_each(inputs.cbegin(), inputs.cend(), [workLoad](size_t i){
        DoWork(i, workLoad);
    });
}

// Breaking out of loops early (with task group cancellation)

// Use default capture mode for nested lambdas. 
// See: http://connect.microsoft.com/VisualStudio/feedback/details/560907/capturing-variables-in-nested-lambdas

void Example05(vector<double>& results, int workLoad)
{
    task_group tg;
    size_t fillTo = results.size() - 5 ;
    fill(results.begin(), results.end(), -1.0);

    task_group_status status = tg.run_and_wait([&]{
        parallel_for(0u, results.size(), [&](size_t i){
            if (i > fillTo)
                tg.cancel();
            else
                results[i] = DoWork(i, workLoad);
        });
    });

    if (status != canceled)
        throw new InvalidValueFoundException();

    // No results in the last five elements of the array will be set. Some values in the
    // remaining array will be set but not all. 

    for (size_t i = 0; i < results.size(); ++i)
    {
        if ((i > fillTo) && (results[i] != -1.0))
            throw new InvalidValueFoundException();
    }
}

// Handling exceptions

void Example06()
{
    bool foundException = false;
    g_SimulateInternalError = true;

    vector<double> results(100000);
    try
    {
        size_t n = results.size(); 
        parallel_for(0u, n, [&results](size_t i) 
        {
            results[i] = DoWork(i, 10); // throws exception
        });
    }
    catch (ParallelForExampleException e)
    {
        printf( "Exception caught as expected.\n");
        foundException = true;
    }
    if (!foundException)
        throw InvalidValueFoundException();

    g_SimulateInternalError = false;
}

// Special Handling of Small Loop Bodies

// Note: The PPL supports specification of a range size but not custom range partitioners

void Example07(vector<double>& results, int workLoad)
{
    size_t size = results.size();
    size_t rangeSize = size / (GetProcessorCount() * 10);
    rangeSize = max(1, rangeSize);

    parallel_for(0u, size, rangeSize, [&results, size, rangeSize, workLoad](size_t i) 
    {
        for (size_t j = 0; (j < rangeSize) && (i + j < size); ++j)
            results[i + j] = DoWork(i + j, workLoad);
    });
} 

void ParallelForExample(int workLoad, int numberOfSteps, bool verifyResult) 
{
    vector<double> results(numberOfSteps);

    printf("Parallel For Examples (workLoad=%d, NumberOfSteps=%d)\n",
        workLoad, numberOfSteps);
    try 
    {
        TimedRun([&results, workLoad](){ Example01(results, workLoad); },
            "Sequential for          ");
        VerifyResult(results, workLoad);

        TimedRun([&results, workLoad](){ Example02(results, workLoad); },
            "Simple parallel_for     ");
        VerifyResult(results, workLoad);

        TimedRun([numberOfSteps, workLoad](){ Example03(numberOfSteps, workLoad); },
            "Sequential for each     ");

        TimedRun([numberOfSteps, workLoad](){ Example04(numberOfSteps, workLoad); },
            "Simple parallel_for_each");

        TimedRun([&results, workLoad]() { Example05(results, workLoad); },
            "Canceling parallel_for  ");

        TimedRun([&results, workLoad]() { Example07(results, workLoad); },
            "Ranged parallel_for_each");
        VerifyResult(results, workLoad);
    }
    catch (InvalidValueFoundException e) 
    {
        printf( "Error: Verification Failed\n");
    }
    printf("\n");
}

int main()
{
    printf("Basic Parallel Loops Samples\n\n");
#if _DEBUG
    printf("For most accurate timing results, use Release build.\n\n");
#endif

    // Parameters: workLoad, NumberOfSteps, VerifyResult
    ParallelForExample( 10000000, 10, true );
    ParallelForExample( 1000000, 100, true );
    ParallelForExample( 10000, 10000, true );
    ParallelForExample( 100, 1000000, true );
    ParallelForExample( 10, 10000000, true );

    printf("parallel_for handling exceptions\n");
    Example06(); 

    printf("\nRun complete... press enter to finish.");
    getchar();
}
