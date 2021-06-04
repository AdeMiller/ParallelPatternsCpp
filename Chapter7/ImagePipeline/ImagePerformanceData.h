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

namespace ImagePipeline
{
    using namespace ::std;

    enum Phases
    {
        kLoad,
        kScale,    
        kFilter,
        kDisplay,
    };

    enum Queues
    {
        kLoaderToScaler,
        kScalerToFilterer,
        kFiltererToDisplayer
    };

    enum Sequence
    {
        kFirstImage = 1,
        kLastImageSentinel = -1,
    };

    class ImagePerformanceData
    {
    private:
        int m_sequenceNumber;
        LARGE_INTEGER m_clockOffset;
        vector<LONGLONG> m_phaseStartTick;
        vector<LONGLONG> m_phaseEndTick;

    public:
        ImagePerformanceData(int sequenceNumber) :  m_sequenceNumber(sequenceNumber)
        {
            m_phaseStartTick.resize(4, 0);
            m_phaseEndTick.resize(4, 0);
        }

        // This assignment operator duplicates the behavior of the default compiler generated operator.
        // It is implemented here so that you can track assignments. See the discussion in Appendix B.

        ImagePerformanceData& operator=(const ImagePerformanceData& rhs)
        {
            // For the Appendix B: The Parallel Tasks and Parallel Stacks Windows walkthrough set the breakpoint on the next line.
            // The breakpoint hit count should track rhs.m_sequenceNumber as ImagePerformanceData is only copied once for each image. 
            ImagePerformanceData tmp(rhs);
            std::swap(m_sequenceNumber, (int)rhs.m_sequenceNumber);
            std::swap(m_clockOffset, (LARGE_INTEGER)rhs.m_clockOffset);
            std::swap(m_phaseStartTick, (vector<LONGLONG>)rhs.m_phaseStartTick);
            std::swap(m_phaseEndTick, (vector<LONGLONG>)rhs.m_phaseEndTick);
            return *this;
        }

        void SetStartTick(int phase)
        {
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            m_phaseStartTick[phase] = now.QuadPart - m_clockOffset.QuadPart;
        }

        void SetStartTick(int phase, const LARGE_INTEGER& start)
        {
            m_phaseStartTick[phase] = start.QuadPart - m_clockOffset.QuadPart;
        }

        void SetEndTick(int phase)
        {
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            m_phaseEndTick[phase] = now.QuadPart - m_clockOffset.QuadPart;
        }

        void SetClockOffset(const LARGE_INTEGER& offset) { m_clockOffset = offset; }

        int GetSequence() const { return m_sequenceNumber; }

        LONGLONG GetPhaseDuration(int phase) const { return m_phaseEndTick[phase] - m_phaseStartTick[phase]; }

        // infer queue wait times by comparing phase(n+1) start with phase(n) finish timestamp
        LONGLONG GetQueueDuration(int queue) const { return m_phaseStartTick[queue+1] - m_phaseEndTick[queue]; }
    };
}
