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
 * Test Case:   TestModuleWaitForShutdown
 * Test ID:     1.10
 * Test Title:  Tests all return codes of WaitForShutdown method
 * Description: This test verifies, that the WaitForShutdown returns the desired values
 * Strategy:    Call the WaitForShutdown method with invalid arguments, with valid but too
 *              short timeout and finally with proper timeout
 * Passed If:   Test responds respectivly with ERR_INVALID_ARG, ERR_TIMEOUT and finally with ERR_NOERROR
 * Ticket:      #34512
 * Requirement: FEPSDK-1673
 */

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;

/**
 * @req_id "FEPSDK-1673"
 */
TEST(TesterFepModule, TestModuleWaitForShutdown)
{
    //Create Module and bring it up to running
    cTestBaseModule oModule;

    cModuleOptions oModOptions("TestModule");
    ASSERT_TRUE(ERR_NOERROR == oModule.Create(oModOptions));
    ASSERT_TRUE(ERR_NOERROR == oModule.StartUpModule(true));

    //Call WaitForShutdown with invalid timeout
    ASSERT_TRUE(ERR_INVALID_ARG == oModule.WaitForShutdown(-32));

    //Call WaitForShutdown with extreme short timeout (should fail with ERR_TIMEOUT)
    ASSERT_TRUE(isFailed(oModule.WaitForShutdown(1)));

    //Actually shutdown module and wait for shutdown (infinite time out)
    ASSERT_TRUE(ERR_NOERROR == oModule.GetStateMachine()->StopEvent());
    ASSERT_TRUE(ERR_NOERROR == oModule.GetStateMachine()->ShutdownEvent());
    ASSERT_TRUE(ERR_NOERROR == oModule.WaitForShutdown(-1));

    //Check that shutdown is reached
    ASSERT_TRUE(fep::FS_SHUTDOWN == oModule.GetStateMachine()->GetState());
}
