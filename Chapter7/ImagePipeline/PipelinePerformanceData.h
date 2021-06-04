//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <vector>

#include "ImagePerformanceData.h"

namespace ImagePipeline
{
    using namespace ::std;

    class PipelinePerformanceData
    {
    private:
        int m_imageCount;
        LARGE_INTEGER m_startTime;
        LARGE_INTEGER m_currentTime;
        LARGE_INTEGER m_clockFrequency;
        vector<LONGLONG> m_totalPhaseTime;
        vector<LONGLONG> m_totalQueueTime;

    public:
        PipelinePerformanceData() :
          m_imageCount(kFirstImage),
              m_startTime(),
              m_currentTime(),
              m_clockFrequency()
          {
              QueryPerformanceFrequency(&m_clockFrequency);
              m_totalPhaseTime.resize(4, 0);
              m_totalQueueTime.resize(3, 0);
              Reset();
          }

          void Reset()
          {
              m_imageCount = kFirstImage;
              m_currentTime.QuadPart = m_startTime.QuadPart = 0;
              fill(m_totalPhaseTime.begin(), m_totalPhaseTime.end(), 0);
              fill(m_totalQueueTime.begin(), m_totalQueueTime.end(), 0);
          }

          int GetImageCount() { return m_imageCount; }

          double GetAveragePhaseTime(int phase) { return 1000.0 * (double)m_totalPhaseTime[phase] / (double)(m_imageCount * m_clockFrequency.QuadPart); }

          double GetAverageQueueTime(int queue) { return 1000.0 * (double)m_totalQueueTime[queue] / (double)(m_imageCount * m_clockFrequency.QuadPart); }

          double GetElapsedTime() { return (double)(m_currentTime.QuadPart - m_startTime.QuadPart) / (double)m_clockFrequency.QuadPart; }

          double GetTimePerImage() { return 1000.0 * GetElapsedTime() / (double)m_imageCount; }

          void Start()
          {
              QueryPerformanceCounter(&m_startTime);
          }

          void Update(const ImagePerformanceData& data)
          {
              QueryPerformanceCounter(&m_currentTime);
              for (int i = 0; i < 4; i++)
                  m_totalPhaseTime[i] += data.GetPhaseDuration(i);
              for (int i = 0; i < 3; i++)
                  m_totalQueueTime[i] += data.GetQueueDuration(i);
              m_imageCount = data.GetSequence();
          }
    private:
        // Disable copy constructor and assignment.
        PipelinePerformanceData(const PipelinePerformanceData& rhs);
        PipelinePerformanceData const & operator=(PipelinePerformanceData const&);
    };
}
