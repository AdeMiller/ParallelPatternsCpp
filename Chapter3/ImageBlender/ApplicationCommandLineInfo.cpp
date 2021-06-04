//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include "stdafx.h"
#include <string>

#include "ApplicationCommandLineInfo.h"

ApplicationCommandLineInfo::ApplicationCommandLineInfo()
    : m_destDir(L""), m_sourceDir(L""), m_file1(L""), m_file2(L""), m_paramIndex(0)
{
}

void ApplicationCommandLineInfo::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
    switch(m_paramIndex)
    {
    case 3: 
        m_destDir = wstring(pszParam);
        break;
    case 2: 
        m_file2 = wstring(pszParam);
        break;
    case 1: 
        m_file1 = wstring(pszParam);
        break;
    case 0: 
        m_sourceDir = wstring(pszParam);
        break;
    default: 
        break;
    }

    ++m_paramIndex;
}
