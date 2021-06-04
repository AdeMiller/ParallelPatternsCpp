//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include <concrt.h>
#include <concrtrm.h>
#include <stdio.h>
#include <windows.h>
#include <iostream>

using namespace ::Concurrency;
using namespace ::std;

int main()
{
    printf("Scheduler Samples\n\n");
#if _DEBUG
    printf("For most accurate timing results, use Release build.\n\n");
#endif
    SchedulerPolicy myPolicy(2, MinConcurrency, 2, 
                                MaxConcurrency, 2);
    Scheduler* myScheduler = Scheduler::Create(myPolicy);
    cout << "My scheduler ID: " << myScheduler->Id() << endl;
    cout << "Default scheduler ID: " 
        << CurrentScheduler::Get()->Id() << endl;

    myScheduler->Attach();
    cout << "Current scheduler ID: " 
        <<  CurrentScheduler::Get()->Id() << endl;

    CurrentScheduler::Detach();
    cout << "Current scheduler ID: " 
        << Concurrency::CurrentScheduler::Get()->Id() << endl;

    HANDLE schedulerShutdownEvent = 
        CreateEvent(NULL, TRUE, FALSE, L"Shutdown Scheduler");
    myScheduler->RegisterShutdownEvent(schedulerShutdownEvent);
    myScheduler->Release();
    WaitForSingleObject(schedulerShutdownEvent, INFINITE);

    printf("\nRun complete... press enter to finish.");
    getchar();
}
