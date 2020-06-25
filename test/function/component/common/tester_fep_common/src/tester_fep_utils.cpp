/**
 * Implementation of the tester for the FEP Utils (Adtf Utils in FEP Context)
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
/*
* Test Case:   TestBlockingQueue
* Test ID:     1.3
* Test Title:  Waitable Queue Tests
* Description: Test the (internal) fep::ext::mutex class.
* Strategy:    Call the API of waitable queue in correct and incorrect way to see, if it behaves as
*              expected/documented
*              
* Passed If:   no errors occur
* Ticket:      #38657
* Requirement: FEPSDK-1406
*/
 
// no min/max macros on windows
#define NOMINMAX

#include <iostream>
#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include "fep_test_common.h"
#include "tester_fep_common_helper.h"
using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

/**
 * @req_id "FEPSDK-1406"
 */
TEST(cTesterFepUtils, TestBlockingQueue)
{
    //__adtf_test_result(run_test_queue<cBlockingQueue<tSlimItem>,tSlimItem,0>(1,1,5));
    //__adtf_test_result(run_test_queue<cBlockingQueue<tSlimItem>,tSlimItem,0>(2,1,5));

    // cBlockingQueue does not work if using more consumers (nConsumers > 1)
    //__adtf_test_result(run_test_blockingQueue<cBlockingQueue<tSlimItem>,tSlimItem,0>(1,2,5));
    fep::Result nRes = run_test_queue<cBlockingQueue<tSlimItem>,tSlimItem,10000>(1,1,5);
    ASSERT_EQ(a_util::result::SUCCESS, nRes);
}

