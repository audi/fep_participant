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
#include "_common/fast/semaphore.h"
#include "a_util/system.h"

using namespace fep::fast;
using namespace a_util;

struct SemaphoreTestStruct
{
    semaphore sema;
    bool done;

    SemaphoreTestStruct() : done(false) {}

    void Work()
    {
        if (sema.timed_wait(1 * 1000000))
        {
            done = true;
        }
    }
};

/**
 * @req_id ""
 */
TEST(fast_semaphore_test, TestSemaphore)
{
    SemaphoreTestStruct test;
    concurrency::thread th(&SemaphoreTestStruct::Work, &test);
    system::sleepMilliseconds(500);

    ASSERT_FALSE(test.done);
    test.sema.post();
    th.join();
    ASSERT_TRUE(test.done);
    ASSERT_EQ(test.sema.value(), 0);
    ASSERT_FALSE(test.sema.timed_wait(50 * 1000)); // 50 ms
    ASSERT_EQ(test.sema.value(), 0);
    test.sema.post();
    ASSERT_EQ(test.sema.value(), 1);
    test.sema.wait();
    ASSERT_EQ(test.sema.value(), 0);
}
