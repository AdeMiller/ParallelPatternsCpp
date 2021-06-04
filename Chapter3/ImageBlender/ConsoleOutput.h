//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once;

// RAII container for a console window.

class ConsoleOutput
{
private:
    FILE* m_pStdout;

    static BOOL CtrlHandler(DWORD fdwCtrlType) { return true; }

public:
    ConsoleOutput(void)
    {
        AllocConsole();
        SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, true);
        freopen_s(&m_pStdout, "CONOUT$", "w+", stdout);
    }

    ~ConsoleOutput(void)
    {
        fclose(m_pStdout);
        SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, false);
    }

private:
    // Hide assignment operator and copy constructor.
    ConsoleOutput const &operator =(ConsoleOutput const&);
    ConsoleOutput(ConsoleOutput const &);
};
