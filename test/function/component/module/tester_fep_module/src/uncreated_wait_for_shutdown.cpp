/**
 * Implementation of the tester for the FEP Module
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
 * Test Case:   TestUncreatedWaitForShutdown
 * Test ID:     1.5
 * Test Title:  Test WaitForShutdown method on uncreated module
 * Description: This test verifies, that WaitForShutdown method simply returns when the module was
 *              not created yet
 * Strategy:    Call WaitForShutdown method on uncreated module
 * Passed If:   Test does not segfault
 * Ticket:      #35391
 * Requirement: FEPSDK-1597
 */

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;

/**
 * @req_id "FEPSDK-1597"
 */
TEST(TesterFepModule, TestUncreatedWaitForShutdown)
{
    fep::cModule oModule;
    ASSERT_TRUE(ERR_NOERROR == oModule.WaitForShutdown());
}
