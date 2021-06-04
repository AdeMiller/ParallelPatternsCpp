//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include <windows.h>
#include <string>
#include <algorithm>
#include <ppl.h>
#include <concrt.h>
#include <concrtrm.h>
#include "ppl_extras.h"

using namespace ::std;
using namespace ::Concurrency;
using namespace ::Concurrency::samples;

void Help()
{
    printf("Usage: ProfilerExamples [deadlock|lockcontention|oversubscription|loadimbalance]\n");

    printf("To set command line parameters from the debugger set the Command Arguments in\nthe Debugging tab of the project's Configuration Properties.\n\n");
}

void Deadlock()
{
    printf("Deadlock\n");

    reader_writer_lock lock1;
    reader_writer_lock lock2;

    parallel_invoke(
        [&lock1, &lock2]() 
        { 
            for (int i = 0; ; i++)
            {
                lock1.lock();
                printf("Got lock 1 at %d\n", i);
                lock2.lock();
                printf("Got lock 2 at %d\n", i);
            }
        },
        [&lock1, &lock2]() 
        { 
            for (int i = 0; ; i++)
            {
                lock2.lock();
                printf("Got lock 2 at %d\n", i);
                lock1.lock();
                printf("Got lock 1 at %d\n", i);
            }
        }
    );
}

// Turn off optimization so delay() does work.
#pragma optimize ("", off)

void delay(int load)
{
    for (int j = 0; j < load; j++);
}

void delay(int i, int loadFactor)
{
    delay(i * loadFactor);
}

#pragma optimize ("", on)

void LockContention()
{
    printf("LockContention\n");

    task_group tasks;
    reader_writer_lock lock;

    for (unsigned int p = 0; p < GetProcessorCount(); p++)
    {
        tasks.run([&lock]() 
        {
            for (int i = 0; i < 10; i++)
            {
                // Do work
                delay(100000);

                // Do protected work
                lock.lock();
                delay(100000000);
                lock.unlock();
            }
        });
    }
    tasks.wait();
}

void Oversubscription()
{
    printf("Oversubscription\n");

    task_group tasks;

    for (unsigned int p = 0; p < (GetProcessorCount() * 4); p++)
    {
        tasks.run([]()
        {
            // Oversubscribe in an exception safe manner
            scoped_oversubcription_token oversubscribe;
            // Do work 
            delay(1000000000);
        });
    }
    tasks.wait();
}

void LoadImbalance()
{
    printf("LoadImbalance\n");

    const int loadFactor = 20;

    parallel_for_fixed(0, 100000, [loadFactor](int i)
    {
        // Do work
        delay(i, loadFactor);
    });
}

int main(int argc, char* argv[])
{
    printf("Profiler Samples\n\n");
    
    if (argc != 2)
    {
        Help();
        printf("Press enter to finish.\n");
        getchar();
        return 0;
    }

    printf("Starting...\n\n");
    // Enable tracing for native code
    Concurrency::EnableTracing();
    // Wait for the profiler to initialize as it also uses resources.
    Sleep(8000);

    string command = argv[1];
    transform(command.begin(), command.end(), command.begin(), tolower);

    if (command == "deadlock")
    {
        Deadlock();
    }
    else if (command == "lockcontention")
    {
        LockContention();
    }
    else if (command == "oversubscription")
    {
        Oversubscription();
    }
    else if (command == "loadimbalance")
    {
        LoadImbalance();
    }
    else
    {
        Help();
    }

    printf("\nRun complete... press enter to finish.\n");
    getchar();
    return 0;
}
