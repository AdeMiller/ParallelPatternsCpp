//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <string>

using namespace ::std;

class ApplicationCommandLineInfo : public CCommandLineInfo
{
private:
    wstring m_sourceDir;
    wstring m_file1;
    wstring m_file2;
    wstring m_destDir;
    int m_paramIndex;

public:
    ApplicationCommandLineInfo();
    virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);

    wstring SourceDir() const { return m_sourceDir; }
    wstring File1() const     { return m_file1; }
    wstring File2() const     { return m_file2; }
    wstring DestDir()const    { return m_destDir; }
};
