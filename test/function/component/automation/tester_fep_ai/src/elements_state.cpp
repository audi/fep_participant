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
* Test Case:   TestElementsState
* Test ID:     1.13
* Test Title:  Tests elements state methods
* Description: Tests GetParticipantsState method
* Strategy:    Three modules are created and steped through different states. The elements state are retrieve with
*              GetParticipantsState.
*              
* Passed If:   The expected elements states are retrieved. 
* Ticket:      -
* Requirement:
*/
#include <gtest/gtest.h>
#include <fep_test_common.h>
#include "helper_functions.h"

/**
 * @req_id ""
 */
TEST(cTesterFepAutomation, TestElementsState)
{
    AutomationInterface oAI;
    cTestBaseModule oMod1, oMod2, oMod3;

    oMod1.Create(cModuleOptions("TestMod1"));
    oMod2.Create(cModuleOptions("TestMod2"));
    oMod3.Create(cModuleOptions("TestMod3"));

    // Set states
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);
    // error in startup -> shutdown, not errorEvent
    oMod1.GetStateMachine()->ShutdownEvent();
    oMod1.WaitForState(FS_SHUTDOWN);
    oMod2.StartUpModule(true);
    oMod2.WaitForState(FS_RUNNING);
    oMod3.StartUpModule(true);
    oMod3.WaitForState(FS_RUNNING);

    std::vector<std::string> strModsList;
    strModsList.push_back(oMod1.GetName());
    strModsList.push_back(oMod2.GetName());
    strModsList.push_back(oMod3.GetName());
    std::map<std::string, tState> oStateMap;

    // try a "very short list" (= one module only)
    std::vector<std::string> strTmpList;
    strTmpList.push_back(oMod2.GetName());
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetParticipantsState(oStateMap, strTmpList,
        REM_PROP_TIMEOUT));
    ASSERT_EQ(oStateMap[oMod2.GetName()], FS_RUNNING);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetParticipantsState(oStateMap, strModsList, REM_PROP_TIMEOUT * 2));
    ASSERT_EQ(oStateMap[oMod1.GetName()], FS_SHUTDOWN);
    ASSERT_EQ(oStateMap[oMod2.GetName()], FS_RUNNING);
    ASSERT_EQ(oStateMap[oMod3.GetName()], FS_RUNNING);

    strModsList.push_back("KeinSchoenWetterTest");
    ASSERT_NE(a_util::result::SUCCESS, oAI.GetParticipantsState(oStateMap, strModsList, REM_PROP_TIMEOUT * 2));
    ASSERT_EQ(oStateMap[oMod1.GetName()], FS_SHUTDOWN);
    ASSERT_EQ(oStateMap[oMod2.GetName()], FS_RUNNING);
    ASSERT_EQ(oStateMap[oMod3.GetName()], FS_RUNNING);
    ASSERT_EQ(oStateMap["KeinSchoenWetterTest"], FS_UNKNOWN);

}