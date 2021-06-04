//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <agents.h>
#include <concrt.h>
#include <string>
#include <iostream>
#include <fstream>
#include <ppl.h>

#include "SampleUtilities.h"
#include "PipelineGovernor.h"
#include "PhraseSource.h"

namespace Pipeline
{
    using namespace ::std;
    using namespace ::Concurrency;
    using namespace ::PipelineUtilities;

    class ReadStringsAgent : public agent
    {
    private:
        int m_seed;
        int m_numberOfSentences;
        int m_sentencePipelineLimit;
        PipelineGovernor& m_governor;
        ITarget<wstring>& m_phraseOutput;

    public:
        ReadStringsAgent(int seed, int numberOfSentences, PipelineGovernor& governor, ITarget<wstring>& phraseOutput) :
            m_seed(seed),
            m_numberOfSentences(numberOfSentences),
            m_governor(governor), 
            m_phraseOutput(phraseOutput)
        {
        }

        void run()
        {
            PhraseSource source(m_seed, m_numberOfSentences);
            wstring inputPhrase;
            do
            {
                Stage1AdditionalWork();
                inputPhrase = source.Next();
                // Limit whole sentences in the pipeline not phrases.
                if (inputPhrase == L".")
                    m_governor.WaitForAvailablePipelineSlot();
                asend(m_phraseOutput, inputPhrase);
            }
            while (inputPhrase != PhraseSource::FinishedSentinel());
            done();
        }
    };

    class CorrectCaseAgent : public agent
    {
    private:
        ISource<wstring>& m_phraseInput;
        ITarget<wstring>& m_phraseOutput;
        bool m_isFirstPhrase;

    public:
        CorrectCaseAgent(ISource<wstring>& phraseInput, ITarget<wstring>& phraseOutput) :
            m_phraseInput(phraseInput),
            m_phraseOutput(phraseOutput),
            m_isFirstPhrase(true)
        {
        }

        void run()
        {
            wstring inputPhrase;
            while(true)
            {
                Stage2AdditionalWork();
                inputPhrase = receive(m_phraseInput);
                if (inputPhrase == PhraseSource::FinishedSentinel())
                {
                    asend(m_phraseOutput, inputPhrase);
                    break;
                }

                // Transform phrase by possibly capitalizing it
                wstring outputPhrase = inputPhrase;
                if (m_isFirstPhrase)
                {
                    outputPhrase[0] = towupper(outputPhrase[0]);
                    m_isFirstPhrase = false;
                }
                if (inputPhrase == L".")
                    m_isFirstPhrase = true;

                asend(m_phraseOutput, outputPhrase);
            }
            done();
        }
    };

    class CreateSentencesAgent : public agent
    {
    private:
        ISource<wstring>& m_phraseInput;
        ITarget<wstring>& m_sentenceOutput;

    public:
        CreateSentencesAgent(ISource<wstring>& phraseInput, ITarget<wstring>& sentenceOutput) :
            m_phraseInput(phraseInput),
            m_sentenceOutput(sentenceOutput)
        {
        }

        void run()
        {
            wstring inputPhrase;
            wstring sentence;
            bool isFirstPhrase = true;
            while(true)
            {
                inputPhrase = receive(m_phraseInput);
                if (inputPhrase == PhraseSource::FinishedSentinel())
                {
                    asend(m_sentenceOutput, inputPhrase);
                    break;
                }

                // Create sentences from input phrases
                if (!isFirstPhrase && inputPhrase != L".")
                    sentence.append(L" ");
                sentence.append(inputPhrase);
                isFirstPhrase = false;
                if (inputPhrase == L".")
                {
                    Stage3AdditionalWork();
                    asend(m_sentenceOutput, sentence);
                    sentence.clear();
                    isFirstPhrase = true;
                }
            }
            done();
        }
    };

    class WriteSentencesAgent : public agent
    {
    private:
        wstring m_targetSentence;
        PipelineGovernor& m_governor;
        ISource<wstring>& m_sentenceInput;
        wstring m_outputPath;
        int m_currentSentenceCount;

    public:
        WriteSentencesAgent(wstring targetSentence, wstring outputPath, PipelineGovernor& governor, ISource<wstring>& sentenceInput) :
            m_targetSentence(targetSentence),
            m_governor(governor),
            m_sentenceInput(sentenceInput),
            m_outputPath(outputPath),
            m_currentSentenceCount(1)
        {
        }

        void run()
        {
            wofstream fout;
            fout.open(m_outputPath);
            wstring sentence;
            while(true)
            {
                Stage4AdditionalWork();
                sentence = receive(m_sentenceInput);
                if (sentence == PhraseSource::FinishedSentinel())
                    break;
                if (sentence == m_targetSentence)
                    sentence.append(L"       Success!");
                fout << m_currentSentenceCount++ << L" " << sentence.c_str() << endl;
                sentence.clear();
                m_governor.FreePipelineSlot();

                OutputProgress(m_currentSentenceCount);
            }
            fout.close();
            done();
        }
    };
};
