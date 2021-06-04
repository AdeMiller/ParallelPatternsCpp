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
#include <array>
#include <random>

#include "SampleUtilities.h"

namespace Pipeline
{
    using namespace ::std;
    using namespace ::SampleUtilities;

    void Stage1AdditionalWork() { DoCpuIntensiveOperation(1); }
    void Stage2AdditionalWork() { DoCpuIntensiveOperation(1); }
    void Stage3AdditionalWork() { DoCpuIntensiveOperation(1); }
    void Stage4AdditionalWork() { DoCpuIntensiveOperation(8); }

    void OutputProgress(int sentenceCount)
    {
        if ((sentenceCount % 10) == 0)
            wcout << L".";
    }

    const wstring g_finishedToken = L"FINISHED!";

    class PhraseSource
    {
    private:
        array<wstring, 8> m_phrases;
        array<wstring, 8>::const_iterator m_phraseIt;
        array<wstring, 2> m_nouns;
        array<wstring, 3> m_adjectives;

        int m_numberOfSentences;
        int m_sentence;
        default_random_engine m_randomEngine;
        uniform_int_distribution<int> m_distributionNouns;
        uniform_int_distribution<int> m_distributionAdjectives;

    public:
        PhraseSource(int seed, int numberOfSentences) : 
            m_numberOfSentences(numberOfSentences),
            m_sentence(0)
        {
            array<wstring, 8> phrases = { L"the", L"<Adjective>", L"<Adjective>", L"<Noun>", L"jumped over the", L"<Adjective>", L"<Noun>", L"." };
            m_phrases.swap(phrases);
            m_phraseIt = m_phrases.cbegin();

            array<wstring, 2> nouns = { L"fox", L"dog" };
            m_nouns.swap(nouns);

            array<wstring, 3> adjectives = { L"quick", L"brown", L"lazy" };
            m_adjectives.swap(adjectives);

            m_randomEngine = default_random_engine(42);

            m_distributionNouns = uniform_int_distribution<int>(0, m_nouns.size() - 1);
            m_distributionAdjectives = uniform_int_distribution<int>(0, m_adjectives.size() - 1);
        }

        const wstring& Next()
        {
            if (m_sentence >= m_numberOfSentences)
                return FinishedSentinel();

            const wstring* pPhrase = &m_phraseIt[0];
            if (*pPhrase == L"<Adjective>")
                pPhrase = &m_adjectives[m_distributionAdjectives(m_randomEngine)];
            if (*pPhrase == L"<Noun>")
                pPhrase = &m_nouns[m_distributionNouns(m_randomEngine)];
            ++m_phraseIt;
            if (m_phraseIt == m_phrases.cend())
                m_phraseIt = m_phrases.cbegin();
            if (*pPhrase == L".")
                ++m_sentence;
            return *pPhrase;
        }
        
        static const wstring& FinishedSentinel() { return g_finishedToken; }
    };
};
