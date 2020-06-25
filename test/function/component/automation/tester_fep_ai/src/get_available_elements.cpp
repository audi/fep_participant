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
* Test Case:   TestGetAvailableElements
* Test ID:     1.1
* Test Title:  Test the availability functions GetAvailableElements
* Description: Some tests for availability utility methods
* Strategy:    GetAvailableElements: Up to 3 elements are started and the number
*              of available elements is checked. The method is called
*              by all three elements and a check will be performed if respectivly 
*              the other two are present in the list of available elements.
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
TEST(cTesterFepAutomation, TestGetAvailableElements)
{
    AutomationInterface oAI;
    std::vector<std::string> strElementsList;
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetAvailableParticipants(strElementsList, 
        REM_PROP_TIMEOUT));
    ASSERT_EQ(strElementsList.size(), 0);
    strElementsList.clear();

    cTestBaseModule oMod1, oMod2;

    // GetAvailableElements
    ASSERT_EQ(a_util::result::SUCCESS, oMod1.Create(cModuleOptions("Mod1")));
    ASSERT_EQ(a_util::result::SUCCESS, oMod1.StartUpModule(true));
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);

    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetAvailableParticipants(strElementsList, REM_PROP_TIMEOUT*3));
    ASSERT_EQ(strElementsList.size(), 1);
    ASSERT_EQ(strElementsList[0], oMod1.GetName());
    strElementsList.clear();

    ASSERT_EQ(a_util::result::SUCCESS, oMod2.Create(cModuleOptions("Mod2")));
    ASSERT_EQ(a_util::result::SUCCESS, oMod2.StartUpModule(true));
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);

    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetAvailableParticipants(strElementsList, REM_PROP_TIMEOUT*3));
    ASSERT_EQ(strElementsList.size(), 2);
    std::string strList = a_util::strings::join(strElementsList, ":");
    ASSERT_TRUE(strList.find(oMod1.GetName()) != std::string::npos);
    ASSERT_TRUE(strList.find(oMod2.GetName()) != std::string::npos);
    strElementsList.clear();

    /// Get available elements with state
    std::map<std::string, fep::tState> strElementsMap;
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetAvailableParticipants(strElementsMap, REM_PROP_TIMEOUT*3));
    ASSERT_EQ(strElementsMap.size(), 2);
    ASSERT_NE(strElementsMap.find(oMod1.GetName()), strElementsMap.end());
    ASSERT_EQ(strElementsMap[oMod1.GetName()], FS_RUNNING);
    strElementsList.clear();

    //Check Error Codes
    ASSERT_EQ(ERR_INVALID_ARG , oAI.GetAvailableParticipants(strElementsList, -10));
}

/**
 * @req_id "FEPSDK-1480 FEPSDK-1481 FEPSDK-1482 FEPSDK-1483 FEPSDK-1484 FEPSDK-1485"
 */
TEST(cTesterFepAutomation, TestGetAvailableElementsFast)
{
    // Test for FEPSDK-945
    cTestBaseModule oMod1, oMod2, oMod4, oMod3;

    // GetAvailableElements
    ASSERT_EQ(a_util::result::SUCCESS, oMod1.Create(cModuleOptions("Mod1")));
    ASSERT_EQ(a_util::result::SUCCESS, oMod1.StartUpModule(true));
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);

    ASSERT_EQ(a_util::result::SUCCESS, oMod2.Create(cModuleOptions("Mod2")));
    ASSERT_EQ(a_util::result::SUCCESS, oMod2.StartUpModule(true));
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);

    ASSERT_EQ(a_util::result::SUCCESS, oMod3.Create(cModuleOptions("Mod3")));
    ASSERT_EQ(a_util::result::SUCCESS, oMod3.StartUpModule(true));
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);

    ASSERT_EQ(a_util::result::SUCCESS, oMod4.Create(cModuleOptions("Mod4")));
    ASSERT_EQ(a_util::result::SUCCESS, oMod4.StartUpModule(true));
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);

    for (int i = 0; i < 10; i++)
    {
        std::vector<std::string> strElementsList;
        AutomationInterface oAI;
        // Sleep was needed before the bugfix, now it is build in the AI constructor
        //a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT*20);
        ASSERT_EQ(a_util::result::SUCCESS, oAI.GetAvailableParticipants(strElementsList,
            REM_PROP_TIMEOUT));
        ASSERT_EQ(strElementsList.size(), 4);
        strElementsList.clear();
    }
}