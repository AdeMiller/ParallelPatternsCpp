//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include <windows.h>
#include <ppl.h>
#include <concurrent_queue.h>
#include <memory>
#include <exception>

#include "SampleUtilities.h"
#include "FuturesExample.h"

using namespace ::Concurrency;
using namespace ::FuturesExample;

#pragma region Worker functions

void DoWork(DWORD milliseconds)
{
    SampleUtilities::DoCpuIntensiveOperation(milliseconds);
}

int F1(int value)
{
    DoWork(2000);
    return value * value;
}

int F2(int value)
{
    DoWork(1000);
    return value - 2;
}

int F3(int value)
{
    DoWork(1000);
    return value + 1;
}

int F4(int value1, int value2)
{
    DoWork(100);
    return value1 + value2;
}

#pragma endregion

// The Basics

// Sequential example

int Example01()
{
    int a = 22;

    int b = F1(a); 
    int c = F2(a); 
    int d = F3(c); 
    int f = F4(b, d); 
    return f;
}

// A parallel example that uses the futures pattern for F1
 
int Example02()
{
    int a = 22;

    Future<int> futureB([a](){ return F1(a); });
    int c = F2(a);
    int d = F3(c);
    int f = F4(futureB.Result(), d);
    return f;
}

// A parallel example that uses the futures pattern for F2/F3
 
int Example03()
{
    int a = 22;

    Future<int> futureD([a](){ return F3(F2(a)); });
    int b = F1(a);
    int f = F4(b, futureD.Result());
    return f;
}

// Exception handling version of Example03

concurrent_queue<std::exception_ptr> g_exceptions;

// Special version of F2 used to show error handling in Example04.

int F2error(int value)
{
    throw exception("F2 failed!"); 
    return value - 2;
}

void Example04()
{
    printf("\nParallel with exception handling\n");
    int a = 22;

    Future<int> futureD([a](){ return F3(F2error(a)); });
    int b = F1(a);
    try
    {
        // futureD.Result() will always throw so the user never sees the result.
        int f = F4(b, futureD.Result());
        printf("  Result = %d\n", f);
    }
    catch (exception& e)
    {
        printf("  Exception '%s' is caught as expected.\n", e.what());
    }
}
 
// A parallel example that uses the futures pattern applied to two values.
// This is for comparison only; there is no performance benefit in this case over Example 2 or 3 above.
// You should pattern your own code after either Example 2 or 3, not this method.
 
int Example05()
{
    int a = 22;
    Future<int> futureB([a](){ return F1(a);} );
    Future<int> futureD([a](){ return F3(F2(a));} );
    int f = F4(futureB.Result(), futureD.Result());
    return f;
}

int main()
{
    printf("Basic Futures Samples\n");
#ifdef _DEBUG
    printf("For most accurate timing results, use Release build.\n");
#endif

    SampleUtilities::TimedResult(Example01, "Sequential                                   ");
    SampleUtilities::TimedResult(Example02, "Parallel using F1 future                     ");
    SampleUtilities::TimedResult(Example03, "Parallel using F2/F3 future                  ");
    SampleUtilities::TimedResult(Example05, "Parallel, using F1 and F2/F3 future          ");
    Example04();
    
    printf("\nRun complete... press enter to finish.");
    getchar();
}
