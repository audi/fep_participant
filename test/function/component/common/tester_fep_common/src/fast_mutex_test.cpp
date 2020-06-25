/**
 * Mutex test implementation
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
#include "_common/fast/mutex.h"
#include "a_util/concurrency/thread.h"
#include "a_util/system/system.h"   //system::sleep

using namespace fep::fast;
using namespace a_util;

struct MutexTestStruct
{
    recursive_mutex mtx;
    void Work()
    {
        mtx.lock();
        mtx.unlock();
    }
};

/**
 * @req_id ""
 */
TEST(fast_mutex_test, TestMutexlock)
{
    MutexTestStruct t;
    ASSERT_TRUE(t.mtx.try_lock());

    {
        concurrency::thread th(&MutexTestStruct::Work, &t);
        system::sleepMilliseconds(100);
        ASSERT_TRUE(th.joinable());

        t.mtx.unlock();
        th.join();
    }
}

TEST(fast_spinlock_test, TestRecursiveMutex)
{
    recursive_mutex m;
    m.lock();
    m.lock();
    m.unlock();
    m.unlock();
}
