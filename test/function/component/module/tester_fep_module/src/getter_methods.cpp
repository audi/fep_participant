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
 * Test Case:   TestModuleGetterMethods
 * Test ID:     1.9
 * Test Title:  Test all Getter-Methods of fep module
 * Description: This test verifies, that the getter methods correctly return the desired values
 * Strategy:    Call the getter methods and check their return values
 * Passed If:   Getter methods return the previously provided values (via setter or creation methods)
 *              or are not null (GetStatemachine, GetAutomationAccess, ...)
 * Ticket:      #34512
 * Requirement: XXXXX
 */

#include "gtest/gtest.h"
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;

TEST(TesterFepModule, TestModuleGetterMethods)
{

    //The variables that will be set. They are also the expected results for
    //their respective getter methods
    const char *strModuleName = "TestModule";
    const int16_t nDomainId = 42;

    //create the module on which the tests are performed
    cModule oModule;
    //Set DomainId to test DomainId-Getter-Method
    // must be called before Create!
    cModuleOptions oModOptions(strModuleName);
    oModOptions.SetDomainId(nDomainId);
    ASSERT_TRUE(ERR_NOERROR == oModule.Create(oModOptions));

    //SetDomainPrefix and GetDomainPrefix is not tested, since they are not
    //implemented placeholders
    //Get Values and compare them with the expected results
    ASSERT_TRUE(nDomainId == oModule.GetDomainId());
    ASSERT_TRUE(a_util::strings::isEqual(strModuleName, oModule.GetName()));
    ASSERT_TRUE(NULL != oModule.GetCommandAccess());
    ASSERT_TRUE(NULL != oModule.GetIncidentHandler());
    ASSERT_TRUE(NULL != oModule.GetNotificationAccess());
    ASSERT_TRUE(NULL != oModule.GetPropertyTree());
    ASSERT_TRUE(NULL != oModule.GetSignalMapping());
    ASSERT_TRUE(NULL != oModule.GetSignalRegistry());
    ASSERT_TRUE(NULL != oModule.GetStateMachine());
    ASSERT_TRUE(NULL != oModule.GetTimingInterface());
    ASSERT_TRUE(NULL != oModule.GetUserDataAccess());
}
