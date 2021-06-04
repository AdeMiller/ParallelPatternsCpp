//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#include <string>
#include <random>
#include <iostream>
#include <fstream>

#include "Pipeline.h"
#include "PhraseSource.h"

using namespace ::std;
using namespace ::Pipeline;

const wstring g_targetSentence = L"The quick brown fox jumped over the lazy dog.";
const wstring g_sequentialResults = L"Chapter7_sequential.txt";
const wstring g_pipelineResults = L"Chapter7_pipeline.txt";
const wstring g_successFlag = L"Success!!!";
const int g_sentenceMax = 200;
const int g_sentencePipelineLimit = 10;

void SequentialExample(const int seed)
{
    bool isFirstPhrase = true;
    wstring sentence;
    int sentenceCount = 1;
    wofstream fout;
    fout.open(g_sequentialResults);
    wstring phrase;
    PhraseSource source(seed, g_sentenceMax);
    do 
    {
        // Read phrase ...

        Stage1AdditionalWork();
        phrase = source.Next();

        // Correct case...

        Stage2AdditionalWork();
        if (isFirstPhrase)
            phrase[0] = towupper(phrase[0]);

        // Build sentence...

        Stage3AdditionalWork();
        if (!isFirstPhrase && (phrase != L"."))
            sentence.append(L" ");
        sentence.append(phrase);
        isFirstPhrase = false;

        // Output result...

        if (phrase == L".")
        {
            Stage4AdditionalWork();
            if (sentence == g_targetSentence)
                sentence.append(L"       Success!");
            isFirstPhrase = true;
            fout << sentenceCount++ << L" " << sentence.c_str() << endl;
            sentence.clear();
            OutputProgress(sentenceCount);
        }
    } 
    while (phrase != PhraseSource::FinishedSentinel());
    fout.close();
}

void ParallelPipelineExample(const int seed)
{
    unbounded_buffer<wstring> buffer1;
    unbounded_buffer<wstring> buffer2;
    unbounded_buffer<wstring> buffer3;
    PipelineGovernor governor(g_sentencePipelineLimit);

    ReadStringsAgent agent1(seed, g_sentenceMax, governor, buffer1);
    CorrectCaseAgent agent2(buffer1, buffer2);
    CreateSentencesAgent agent3(buffer2, buffer3);
    WriteSentencesAgent agent4(g_targetSentence, g_pipelineResults, governor, buffer3);

    agent1.start();
    agent2.start();
    agent3.start();
    agent4.start();

    agent* agents[4] = { &agent1, &agent2, &agent3, &agent4 };
    agent::wait_for_all(4, agents);
}

void CompareFiles(const wstring& file1, const wstring& file2) 
{
    wifstream fin1(file1);
    wifstream fin2(file2);
    size_t line = 1;

    while (!fin2.eof())
    {
        if (fin1.eof())
        {
            cout << "Error! Files are unequal length." << endl;
            break;
        }
        wstring line1;
        wstring line2;
        getline(fin2, line2);
        getline(fin1, line1);
        if (line1 != line2) 
            wcout << L"Error! Line mismatch at line " << line << ":" << endl << line1 << endl << line2 << endl << endl;
        ++line;
    }
    fin1.close();
    fin2.close();
}

int main()
{
    printf("Basic Pipeline Samples\n\n");
#if _DEBUG
    printf("For most accurate timing results, use Release build.\n\n");
#endif

    const int seed = 42;
    TimedRun([seed]() { SequentialExample(seed); }, "Write sentences, sequential");
    TimedRun([seed]() { ParallelPipelineExample(seed); }, "Write sentences, pipeline  ");
    CompareFiles(g_sequentialResults, g_pipelineResults);

    printf("\nRun complete... press enter to finish.");
    getchar();
}
