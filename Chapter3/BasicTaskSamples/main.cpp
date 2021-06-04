//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include <iosfwd>
#include <concrt.h>
#include <ppl.h>

#include "SampleUtilities.h"

using namespace ::std;
using namespace ::Concurrency;
using namespace ::SampleUtilities;

const DWORD g_TaskMilliseconds = 1000;

#pragma region Worker methods - DoLeft, DoRight

void DoLeft()
{
    DoCpuIntensiveOperation(g_TaskMilliseconds/5);
    wcout << L"  Left finished" << endl;
}

void DoRight()
{
    DoCpuIntensiveOperation(g_TaskMilliseconds/3);
    wcout << L"  Right finished" << endl;
}

#pragma endregion

#pragma region Worker methods - SearchLeft, SearchCenter, SearchRight

void SearchCenter(task_group& tg)
{
    bool result = DoCpuIntensiveOperation(g_TaskMilliseconds/2, &tg);
    wcout << L"  Center search finished, completed = " << result << endl;
    tg.cancel();
}

void SearchLeft(task_group& tg)
{
    bool result = DoCpuIntensiveOperation(g_TaskMilliseconds/5, &tg);
    wcout << L"  Left search finished, completed = " << result << endl;
    tg.cancel();
}

void SearchRight(task_group& tg)
{
    bool result = DoCpuIntensiveOperation(g_TaskMilliseconds/3, &tg);
    wcout << L"  Right search finished, completed = " << result << endl;
    tg.cancel();
}

#pragma endregion

// Sequential tasks

void Example01()
{
    wcout << endl << "Started..." << endl;

    DoLeft();
    DoRight();
}

// Parallel tasks with parallel_invoke

void Example02()
{
    wcout << endl << "Started..." << endl;

    parallel_invoke(
        []() { DoLeft(); },
        []() { DoRight(); }
    );
}

// Parallel tasks with task_group

void Example03()
{
    wcout << endl << "Started..." << endl;

    task_group tg;

    tg.run([](){ DoLeft(); });
    tg.run([](){ DoRight(); });
    tg.wait();
}

void Example04()
{
    wcout << endl << "Started..." << endl;

    task_group tg;

    tg.run([](){ DoLeft(); });
    tg.run_and_wait([](){ DoRight(); });
}

// Canceling a task_group directly

void Example05()
{
    wcout << endl << "Started..." << endl;

    task_group tg;
    tg.run([](){ DoLeft(); });
    tg.cancel();                   // could be called from any thread
    wcout << L"  Canceling: " << tg.is_canceling() << endl;

    task_group_status status = tg.wait();
    wcout << L"  Status:    " << status << endl;
}

// Canceling a task from within parallel_invoke

void Example06()
{
    wcout << endl << "Started..." << endl;

    task_group tg;
    parallel_invoke(
        []() { DoLeft(); },
        []() { DoRight(); },
        [&tg]() { tg.cancel(); }
    );
    wcout << "  Cancelled: " << tg.is_canceling() << endl;
}

// Handling exceptions

void Example07()
{
    wcout << endl << "Started..." << endl;

    try
    {
        parallel_invoke(
            []() { DoLeft(); },
            []() { DoRight(); },
            []() { throw exception("Error!"); }
        );
    }
    catch (const exception& e)
    {
    	wcout << "  Caught exception: " << e.what() << endl;
    }

    wcout << L"  Finished" << endl;
}

void Example08()
{
    wcout << endl << "Started..." << endl;

    task_group tg;
    tg.run([&](){ DoLeft(); });
    tg.run([&](){ DoRight(); });
    tg.run([&](){ throw exception("Error!"); });

    try
    {
        tg.wait();
    }
    catch (const exception& e)
    {
        wcout << "  Caught exception: " << e.what() << endl;
    }
    wcout << L"  Finished" << endl;
}

void Example09()
{
    wcout << endl << "Started..." << endl;

    task_group tg;
    tg.run([&tg](){ DoLeft(); });
    tg.run([&tg](){ DoRight(); });
    tg.run([&tg](){ throw exception("Error 1!"); });
    tg.run([&tg](){ throw exception("Error 2!"); });

    try
    {
        tg.wait();
    }
    catch (const exception& e)
    {
        wcout << "  Caught an exception: " << e.what() << endl;
    }
    wcout << L"  Finished" << endl;
}

// Speculative execution

void Example10()
{
    wcout << endl << "Started..." << endl;

    task_group tg;

    tg.run([&tg](){
        SearchLeft(tg);
    });
    tg.run([&tg](){
        SearchRight(tg);
    });
    tg.run_and_wait([&tg](){ 
        SearchCenter(tg); 
    });
}

// Variables captured by closures

void Example11()
{
    wcout << endl << "Started..." << endl << "Capture by reference:" << endl;
    {
        task_group tg;
        for (int i = 0; i < 4; i++)
        {
            // WARNING: BUGGY CODE, i has unexpected value
            tg.run([&i]() { wcout << i << endl; } );
        }
        tg.wait();
    }

    wcout << endl << "Capture by value:" << endl;
    {
        task_group tg;
        for (int i = 0; i < 4; i++)
        {
            tg.run([i]() { wcout << i << endl; } );
        }
        tg.wait();
    }
}

void main() 
{
    printf("Basic Parallel Tasks Samples\n");
#if _DEBUG
    printf("For most accurate timing results, use Release build.\n\n");
#endif

    TimedRun(Example01, "2 steps, sequential");
    TimedRun(Example02, "2 steps, parallel_invoke");
    TimedRun(Example03, "2 steps, task_group");
    TimedRun(Example04, "2 steps, task_group and wait");
    TimedRun(Example05, "2 steps, parallel_invoke cancel");
    TimedRun(Example06, "2 steps, task_group cancel");
    TimedRun(Example07, "2 steps, parallel_invoke exception");
    TimedRun(Example08, "2 steps, task_group exception");
    TimedRun(Example09, "2 steps, task_group multiple exceptions");
    TimedRun(Example10, "Speculative Execution");
    TimedRun(Example11, "Variable capture");

    printf("\nRun complete... press enter to finish.");
    getchar();
}
