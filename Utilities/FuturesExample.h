//===============================================================================
// Microsoft patterns & practices
// Parallel Programming Guide
//===============================================================================
// Copyright © Microsoft Corporation.  All rights reserved.
// This code released under the terms of the 
// Microsoft patterns & practices license (http://parallelpatterns.codeplex.com/license).
//===============================================================================

#pragma once

#include <ppl.h>
#include <agents.h>

namespace FuturesExample
{
    using namespace ::Concurrency;
    using namespace ::std;

    // This is a very simple example of the Future pattern implemented using a task_group. 
    // It should not be confused with other Future pattern implementations like the 
    // std::future implementation slated for STL.

    // This implementation of a Future class omits features such as the ability to rethrow 
    // exceptions when you call the Result method multiple times. You can use this implementation 
    // in your own applications, but you should be aware that it is not meant to be 
    // completely full-featured.

    template <class T> 
    class Future
    {
    private:
        single_assignment<T> m_val;
        task_group m_tg; 

    public:
        template <class Func>
        Future(Func f) 
        {
            m_tg.run([f, this]() 
            {
                send(m_val, f());
            });
        }

        T Result() 
        {
            m_tg.wait();
            return receive(&m_val);
        }
    };
};
