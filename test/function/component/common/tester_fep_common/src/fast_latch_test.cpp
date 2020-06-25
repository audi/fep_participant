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
#include "a_util/system.h"

using namespace fep::fast;
using namespace a_util;

struct LatchTestStruct
{
    latch ltch;
    bool done;

    LatchTestStruct() : ltch(1), done(false) {}

    void Work()
    {
        if (ltch.wait_for(10 * 1000000))
        {
            done = true;
        }
    }
};

/**
 * @req_id ""
 */
TEST(fast_latch_test, TestLatch)
{
    LatchTestStruct test;
    concurrency::thread th(&LatchTestStruct::Work, &test);
    system::sleepMilliseconds(500);

    ASSERT_FALSE(test.done);
    ASSERT_EQ(test.ltch.value(), 1);

    test.ltch.count_down();
    system::sleepMilliseconds(500);
    th.join();
    
    ASSERT_TRUE(test.done);
    ASSERT_EQ(test.ltch.value(), 0);
    ASSERT_TRUE(test.ltch.wait_for(50 * 1000)); // 50 ms
    ASSERT_EQ(test.ltch.value(), 0);
    test.ltch.reset(1);
    ASSERT_EQ(test.ltch.value(), 1);
    ASSERT_FALSE(test.ltch.wait_for(50 * 1000)); // 50 ms

    test.ltch.count_down_and_wait();
    ASSERT_EQ(test.ltch.value(), 0);
}
