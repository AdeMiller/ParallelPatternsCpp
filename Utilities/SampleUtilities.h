//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <windows.h> 
#include <math.h>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

#include <GdiPlus.h>
#include <io.h>
#include <ppl.h>

/// <summary>
/// The sample utilities namespace contains timing and numeric utilities
/// </summary>
namespace SampleUtilities
{
    using namespace ::std;
    using namespace ::Concurrency;

#pragma region Timing utilities
    /// <summary>
    /// Executes a function and prints result and timing
    /// </summary>
    template<typename Func>
    void TimedResult(Func test, string label)
    {
        double begin = GetTickCount();

        // invoke the function
        auto result = test();

        // print timings
        double end = GetTickCount();

        cout << label.c_str() << "  (result = " << setprecision(2) << fixed << result << "): " << (end - begin) <<  "ms" << endl;
    }

    /// <summary>
    /// Executes a function and prints timing results
    /// </summary>
    template<typename Func>
    void TimedRun(Func test, string label)
    {
        LARGE_INTEGER begin, end, freq;
        QueryPerformanceCounter(&begin);

        // invoke the function
        test();

        // print timings
        QueryPerformanceCounter(&end);

        QueryPerformanceFrequency(&freq);

        double result = (end.QuadPart - begin.QuadPart) / (double) freq.QuadPart;
        printf("%s : %4.2f ms\n", label.c_str(), result * 1000);
    }
    /// <summary>
    /// Executes a function and returns the timing in milliseconds
    /// </summary>
    template<typename Func>
    double TimedRun(Func test)
    {
        LARGE_INTEGER begin, end, freq;
        QueryPerformanceCounter(&begin);

        // invoke the function
        test();

        // print timings
        QueryPerformanceCounter(&end);

        QueryPerformanceFrequency(&freq);

        double result = (end.QuadPart - begin.QuadPart) / (double) freq.QuadPart;

        return result;
    }
#pragma endregion

#pragma region File utilities

    /// <summary>
    /// Check whether directory exists, if not write message and exit immediately.
    /// </summary>
    /// <param name="dirName">Directory name</param>
    inline bool CheckDirectoryExists(const wstring& dirName)
    {
        if (FILE_ATTRIBUTE_DIRECTORY != (GetFileAttributesW(dirName.c_str()) & FILE_ATTRIBUTE_DIRECTORY))
        {
            wprintf(L"Directory does not exist: %s\n", dirName.c_str());
            return false;
        }
        return true;
    }

    /// <summary>
    /// Check whether file exists, if not write message and exit immediately.
    /// (can't use this method to check whether directory exists)
    /// </summary>
    /// <param name="path">Fully qualified file name including directory</param>
    inline bool CheckFileExists(const wstring& path)
    {
        if (INVALID_FILE_ATTRIBUTES == (GetFileAttributesW(path.c_str())))
        {
            wprintf(L"File does not exist: %s\n", path.c_str());
            return false;
        }
        return true;
    }

    wstring GetApplicationDirectory() 
    {
        wstring dir;
        WCHAR dir_raw[MAX_PATH];

        GetModuleFileNameW(nullptr, dir_raw, MAX_PATH); 
        dir.append(dir_raw);
        size_t n = dir.find_last_of(L'\\', dir.size());
        dir = dir.substr(0, n);
        dir.append(L"\\");
        return dir;
    }

    vector<wstring> ListFilesInApplicationDirectory(wstring extn)
    {
        vector<wstring> filenames;
        WIN32_FIND_DATAW ffd;
        HANDLE hFind;
        
        wstring dirRoot = GetApplicationDirectory();
        wstring searchMask = dirRoot;
        searchMask.append(L"*.").append(extn);
        if (searchMask.size() > MAX_PATH)
            return filenames;

        hFind = FindFirstFileW(&searchMask[0], &ffd);
        if (INVALID_HANDLE_VALUE == hFind) 
            return filenames;
        do
        {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                wstring name;
                name.append(dirRoot).append(ffd.cFileName);
                filenames.push_back(name);
            }
        }
        while (FindNextFileW(hFind, &ffd) != 0);
        return filenames;
    }

#pragma endregion

#pragma region Dummywork utilities

    /// <summary>
    /// Simulates a CPU-intensive operation on a single core. The operation will use approximately 100% of a
    /// single CPU for a specified duration.
    /// </summary>
    /// <param name="seconds">The approximate duration of the operation in seconds</param>
    /// <param name="tg">A pointer to a task group provided for cancellation support
    /// <returns>true if operation completed normally; false if the user canceled the operation</returns>
    bool DoCpuIntensiveOperation(DWORD milliseconds, task_group* tg = nullptr)
    {
        if (tg && Context::IsCurrentTaskCollectionCanceling())
            return false;

        DWORD tickCount = GetTickCount();
        DWORD checkInterval = min(20000000, 20000 * milliseconds);

        // loop to simulate a computationally intensive operation
        int i = 0;
        while (true) {
            // periodically check to see if the user has requested cancellation 
            // or if the time limit has passed
            if ((milliseconds == 0) || (++i % checkInterval == 0)) {
                if (tg && tg->is_canceling())
                    return false;
            }
            DWORD currentTick = GetTickCount();
            if (currentTick-tickCount > milliseconds)
                return true;
        }
    }

    static int SleepTimeouts[] = 
    { 
        65, 165, 110, 110, 185, 160, 40, 125, 275, 110, 
        80, 190, 70, 165, 80, 50, 45, 155, 100, 215, 
        85, 115, 180, 195, 135, 265, 120, 60, 130, 115, 
        200, 105, 310, 100, 100, 135, 140, 235, 205, 10, 
        95, 175, 170, 90, 145, 230, 365, 340, 160, 190, 
        95, 125, 240, 145, 75, 105, 155, 125, 70, 325, 
        300, 175, 155, 185, 255, 210, 130, 120, 55, 225, 
        120, 65, 400, 290, 205, 90, 250, 245, 145, 85, 
        140, 195, 215, 220, 130, 60, 140, 150, 90, 35, 
        230, 180, 200, 165, 170, 75, 280, 150, 260, 105
    };

    static int SleepTimeoutLength = 100;

    /// <summary>
    /// Simulates an I/O-intensive operation on a single core. The operation will use only a small percent of a
    /// single CPU's cycles; however, it will block for the specified number of seconds.
    /// </summary>
    /// <param name="milliseconds">The approximate duration of the operation in milliseconds</param>
    /// <param name="tasks">A token that may signal a request to cancel the operation.</param>
    /// <returns>true if operation completed normally; false if the user canceled the operation</returns>
    bool DoIoIntensiveOperation(DWORD milliseconds, task_group& tasks)
    {
        if (tasks.is_canceling())
            return false;

        DWORD tickCount = GetTickCount();
        int i = (abs((long)SleepTimeouts) % SleepTimeoutLength);

        while(true)
        {
            int timeout = SleepTimeouts[i];
            i = ++i % SleepTimeoutLength;

            // simulate i/o latency
            Sleep(timeout);

            // Has the user requested cancellation? 
            if (tasks.is_canceling())
                return false;

            // Is the IO finished?
            if ((GetTickCount() - tickCount) > milliseconds)
                return true;
        }
        return false;
    }

#pragma endregion 
}
