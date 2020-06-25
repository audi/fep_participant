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
#include "_common/fast/condition_variable.h"
#include "a_util/concurrency/thread.h"
#include "a_util/system/system.h"   //system::sleep

using namespace fep::fast;
using namespace a_util;

struct CondVarTestStruct
{
    condition_variable& cond;
    spinlock& mtx;
    volatile bool& signal;
    CondVarTestStruct(condition_variable& cond_, spinlock& mtx_, bool& signal_) :
    cond(cond_), mtx(mtx_), signal(signal_) {}

    void Work()
    {
        lock_guard<spinlock> guard(mtx);
        ASSERT_FALSE(signal);
        while (!signal)
        {
            cond.wait_for(mtx, 60 * 1000000); // 60 s
            //concurrency::detail::memory_barrier();
        }
    }
};

/**
 * @req_id ""
 */
TEST(fast_condvar_test, TestCondVar)
{
    condition_variable cond;
    spinlock mtx;
    bool signal = false;

    CondVarTestStruct test(cond, mtx, signal);
    concurrency::thread th(&CondVarTestStruct::Work, &test);
    system::sleepMilliseconds(500);

    {
        lock_guard<spinlock> guard(mtx);
        signal = true;
        cond.notify_one();
    }

    th.join();
}
