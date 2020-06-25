/**
 * Condition variable test implementation
 *
 * @file

   @copyright
   @verbatim
   Copyright @ 2019 Audi AG. All rights reserved.
   
       This Source Code Form is subject to the terms of the Mozilla
       Public License, v. 2.0. If a copy of the MPL was not distributed
       with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
   
   If it is not possible or desirable to put the notice in a particular file, then
   You may include the notice in a location (such as a LICENSE file in a
   relevant directory) where a recipient would be likely to look for such a notice.
   
   You may add additional accurate notices of copyright ownership.
   @endverbatim
 *
 */

#include <gtest/gtest.h>
#include "_common/fast/latch.h"
#include "_common/fast/semaphore.h"
#include "a_util/system.h"

#include <cassert>

using namespace fep::fast;
using namespace a_util;

class Control
{
public:
	Control(const std::size_t count_workers)
		: m_count_workers(count_workers)
        , m_oSemaphoreA(0)
		, m_oEnterLatchB(0)
		, m_oLeaveLatchC(0)
	{
	}

public:
 	void ManagerPost()
	{
        assert(m_oEnterLatchB.value() == 0);
		assert(m_oLeaveLatchC.value() == 0);
		assert(m_oSemaphoreA.value() == 0);

		m_oEnterLatchB.reset(m_count_workers);
		m_oLeaveLatchC.reset(m_count_workers);

        assert(m_oEnterLatchB.value() == m_count_workers);
		assert(m_oLeaveLatchC.value() == m_count_workers);

		m_oSemaphoreA.post_n(m_count_workers);
    }

	void ManagerWait()
	{
		m_oLeaveLatchC.wait();
    }

public:
	bool TryWorkerEnter(const timestamp_t& timestamp)
	{
        if (!m_oSemaphoreA.timed_wait(timestamp))
        {
            return false;
        }

		std::size_t v1 = m_oEnterLatchB.value();
		m_oEnterLatchB.count_down();
		std::size_t v2 = m_oEnterLatchB.value();
		assert(v2 < v1);

        return true;
	}

	void WorkerLeave()
	{
        m_oEnterLatchB.wait();
	    m_oLeaveLatchC.count_down();
	}

        
public:
    std::size_t m_count_workers;
    semaphore m_oSemaphoreA;
    latch m_oEnterLatchB;
	latch m_oLeaveLatchC;

};

class Worker 
{
public:
	Worker(Control& control)
		: m_control(control)
        , m_is_running(true)
        , m_count(0)
	{
	}

public:
    void RequestShutdown()
    {
        m_is_running= false;
    }

    void Work() 
	{
		while (m_is_running)
		{
			if (m_control.TryWorkerEnter(10 * 1000))
            {

                // Do work
			    ++m_count;

                m_control.WorkerLeave();
            }
        }
	}

private:
	Control& m_control;
    bool m_is_running;

public:
    // Just for debugging
    std::size_t m_count;
};

class Manager 
{
public:
	Manager(Control& control)
		: m_control(control)
        , m_is_running(true)
	{
	}

public:
    void RequestShutdown()
    {
        m_is_running= false;
    }

	void Work() 
	{
		while (m_is_running)
		{
            // Wait for hearbeat ...
            // ... update clocks ...

            m_control.ManagerPost();
            
            // Nothing to do here ... just wait for finish

            m_control.ManagerWait();
		}
	}

public:
	Control& m_control;
    bool m_is_running;
};



/**
 * @req_id ""
 */
TEST(fast_workerthreads_test, TestWorkerThread)
{
    Control ctrl(4);
    Manager mgr(ctrl);

    Worker taskA(ctrl);
    Worker taskB(ctrl);
    Worker taskC(ctrl);
    Worker taskD(ctrl);

	ASSERT_EQ(taskA.m_count, 0);
	ASSERT_EQ(taskA.m_count, taskB.m_count);
	ASSERT_EQ(taskA.m_count, taskC.m_count);
	ASSERT_EQ(taskA.m_count, taskD.m_count);

    // Start things
    concurrency::thread taskA_th(&Worker::Work, &taskA);
    concurrency::thread taskB_th(&Worker::Work, &taskB);
    concurrency::thread taskC_th(&Worker::Work, &taskC);
    concurrency::thread taskD_th(&Worker::Work, &taskD);
    concurrency::thread mgr_th(&Manager::Work, &mgr);

    // Run for 1 seconds
    system::sleepMilliseconds(1 * 1000);

    // RequestShutdown
    taskA.RequestShutdown();
    taskB.RequestShutdown();
    taskC.RequestShutdown();
    taskD.RequestShutdown();
    mgr.RequestShutdown();

    // Join
    taskA_th.join();
    taskB_th.join();
    taskC_th.join();
    taskD_th.join();
    mgr_th.join();

    ASSERT_GT(taskA.m_count, 0);
	ASSERT_EQ(taskA.m_count, taskB.m_count);
	ASSERT_EQ(taskA.m_count, taskC.m_count);
	ASSERT_EQ(taskA.m_count, taskD.m_count);

    std::cerr << "RUN: " << taskA.m_count << " counts" << std::endl;
}
