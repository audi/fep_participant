/**
 * Implementation of the tester for the FEP Automation Interface
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
* Test Case:   TestCheckElementAvailability
* Test ID:     1.3
* Test Title:  Test the availability functions CheckElementAvailability
* Description: Some tests for availability utility methods
* Strategy:    CheckElementAvailability: 3 elements are created and availability is checked by
*              each element for the other two. The method is called with different invalid 
*              arguments. Method is called for Module in startup state. The method is called
*              concurrently.
*              
* Passed If:   see strategy
* Ticket:      -
* Requirement: FEPSDK-1480 FEPSDK-1481 FEPSDK-1482 FEPSDK-1483 FEPSDK-1484 FEPSDK-1485
*/
#include <gtest/gtest.h>
#include <fep_test_common.h>

#include "helper_functions.h"

/**
 * @req_id "FEPSDK-1480 FEPSDK-1481 FEPSDK-1482 FEPSDK-1483 FEPSDK-1484 FEPSDK-1485"
 */
TEST(cTesterFepAutomation, TestCheckElementAvailability)
{
    AutomationInterface oAI;

    cTestBaseModule oMod1, oMod2;
    ASSERT_EQ(a_util::result::SUCCESS, oMod1.Create(cModuleOptions("Mod1")));
    ASSERT_EQ(a_util::result::SUCCESS, oMod1.StartUpModule(true));

    // CheckElementAvailability
    ASSERT_EQ(a_util::result::SUCCESS, oAI.CheckParticipantAvailability(oMod1.GetName(),
        REM_PROP_TIMEOUT));
    ASSERT_EQ(oAI.CheckParticipantAvailability("bullshit", REM_PROP_TIMEOUT), 
        ERR_TIMEOUT);
    ASSERT_EQ(oAI.CheckParticipantAvailability("", REM_PROP_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.CheckParticipantAvailability("?", REM_PROP_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.CheckParticipantAvailability("*", REM_PROP_TIMEOUT), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.CheckParticipantAvailability(oMod1.GetName(), -1), ERR_INVALID_ARG);

    // Module should be found right after creation, even if state is still FS_STARTUP
    ASSERT_EQ(a_util::result::SUCCESS, oMod2.Create(cModuleOptions("oMod2")));
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.CheckParticipantAvailability(oMod2.GetName(), 
        REM_PROP_TIMEOUT));
}