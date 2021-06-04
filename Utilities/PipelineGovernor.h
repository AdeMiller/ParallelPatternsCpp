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

namespace PipelineUtilities
{
    using namespace ::Concurrency;

    // A pipeline has a fixed capacity of in-flight elements. The PipelineGovernor class
    // implements a messaged-based throttling mechanism that is similar in functionality
    // to a semaphore but uses messages instead of shared memory.
    //
    // The last stage of the pipeline should call FreePipelineSlot() each time it finishes processing 
    // an element.
    //
    // The first stage of the pipeline should call WaitForAvailablePipelineSlot() before forwarding
    // each new element to the next stage of the pipeline.
    //
    // The first stage of the pipeline should call WaitForEmptyPipeline() on shutdown before it
    // reclaims memory for pipeline stages.

    class PipelineGovernor
    {
    private:
        struct signal {};

        int m_capacity;
        int m_phase;
        unbounded_buffer<signal> m_completedItems;

    public:
          PipelineGovernor(int capacity) :
              m_phase(0), m_capacity(capacity) {}

          // only called by last stage of pipeline
          void FreePipelineSlot() 
          {
              send(m_completedItems, signal());
          }
          
          // only called by first pipeline stage
          void WaitForAvailablePipelineSlot() 
          {
              if (m_phase < m_capacity)
                  ++m_phase;
              else
                  receive(m_completedItems);
          }

          // only called by first pipeline stage
          void WaitForEmptyPipeline() 
          {
              while(m_phase > 0)
              {
                  --m_phase;
                  receive(m_completedItems);
              }
          }   

    private:
        // Disable copy constructor and assignment.
        PipelineGovernor(const PipelineGovernor&);
        PipelineGovernor const & operator=(PipelineGovernor const&);
    };
};
