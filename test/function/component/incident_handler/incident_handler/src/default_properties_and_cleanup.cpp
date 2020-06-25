/**
 * Implementation of the tester for the FEP Incident Handler
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

/**
* Test Case:   TestDefaultPropertiesAndCleanup
* Test ID:     1.0
* Test Title:  Test default properties and Cleanup
* Description: Test setting of default properties at initialization and on Cleanup()
* Strategy:   The cIncidentHandler is getting created and initialized. Values for default properties are tested. 
*             Properties are changed and then reset to default values by cIncidentHandler::Cleanup()
* Passed If:   The incident handler properties are set to expected values after initialization and Cleanup() "
*              resets all changed properties to their default values.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1432
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include <fep_test_common.h>
#include <fep_ih_test_common.h>
#include "test_fixture.h"

using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

/**
 * @req_id "FEPSDK-1432"
 */
TEST_F(TestFixture, TestDefaultPropertiesAndCleanup)
{
    // Testing for the default properties.
    IPropertyTree * pPropTree = m_pTestModule->GetPropertyTree();
    cPropertyTester oPropTest(pPropTree, "");
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHandlerPath_bEnableGlobalScope, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHandlerPath_strSourceFilter, "*"));

    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHistoryLogPath_bEnableCatchAll, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHistoryLogPath_bEnable, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHistoryLogPath_nQueueSize,
                                          static_cast<int32_t>(500)));

    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentFileLogPath_bEnableCatchAll, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentFileLogPath_bEnable, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentFileLogPath_bEnableCSV, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentFileLogPath_strPath, ""));

    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentNotificationLogPath_bEnableCatchAll, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentNotificationLogPath_bEnable, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentNotificationLogPath_strTarget, "*")); 

    // Change properties
    ASSERT_EQ(a_util::result::SUCCESS, pPropTree->SetPropertyValue(component_config::g_strIncidentHandlerPath_bEnable, false));
    ASSERT_EQ(a_util::result::SUCCESS, pPropTree->SetPropertyValue(component_config::g_strIncidentHandlerPath_bEnableGlobalScope, true));
    ASSERT_EQ(a_util::result::SUCCESS, pPropTree->SetPropertyValue(component_config::g_strIncidentHandlerPath_strSourceFilter, "blub"));

    ASSERT_EQ(a_util::result::SUCCESS, pPropTree->SetPropertyValue(component_config::g_strIncidentHistoryLogPath_bEnableCatchAll, true));
    ASSERT_EQ(a_util::result::SUCCESS, pPropTree->SetPropertyValue(component_config::g_strIncidentHistoryLogPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, pPropTree->SetPropertyValue(component_config::g_strIncidentHistoryLogPath_nQueueSize,
                                          static_cast<int32_t>(5)));

    ASSERT_EQ(a_util::result::SUCCESS, pPropTree->SetPropertyValue(
                           component_config::g_strIncidentFileLogPath_bEnableCatchAll, true));
    ASSERT_EQ(a_util::result::SUCCESS, pPropTree->SetPropertyValue(
                           component_config::g_strIncidentFileLogPath_bEnableCSV, true));
    ASSERT_EQ(a_util::result::SUCCESS, pPropTree->SetPropertyValue(
                           component_config::g_strIncidentFileLogPath_strPath, "some path"));
    ASSERT_EQ(a_util::result::SUCCESS, pPropTree->SetPropertyValue(
                           component_config::g_strIncidentFileLogPath_bEnable, true));

    ASSERT_EQ(a_util::result::SUCCESS, pPropTree->SetPropertyValue(
                           component_config::g_strIncidentNotificationLogPath_bEnableCatchAll, true));
    ASSERT_EQ(a_util::result::SUCCESS, pPropTree->SetPropertyValue(
                           component_config::g_strIncidentNotificationLogPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, pPropTree->SetPropertyValue(
                           component_config::g_strIncidentNotificationLogPath_strTarget, "bla")); 

    // Test for changed properties
    ASSERT_NE(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_NE(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHandlerPath_bEnableGlobalScope, false));
    ASSERT_NE(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHandlerPath_strSourceFilter, "*"));

    ASSERT_NE(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHistoryLogPath_bEnableCatchAll, false));
    ASSERT_NE(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHistoryLogPath_bEnable, false));
    ASSERT_NE(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHistoryLogPath_nQueueSize,
                                          static_cast<int32_t>(500)));

    ASSERT_NE(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentFileLogPath_bEnableCatchAll, false));
    ASSERT_NE(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentFileLogPath_bEnable, false));
    ASSERT_NE(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentFileLogPath_bEnableCSV, false));
    ASSERT_NE(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentFileLogPath_strPath, ""));

    ASSERT_NE(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentNotificationLogPath_bEnableCatchAll, false));
    ASSERT_NE(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentNotificationLogPath_bEnable, false));
    ASSERT_NE(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentNotificationLogPath_strTarget, "*")); 

    // Call Cleanup()
    cIncidentHandler* const pIH = dynamic_cast<cIncidentHandler* const>(m_pTestModule->GetIncidentHandler());
    pIH->SetDefaultProperties();

    // Test if default properties have been restored
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHandlerPath_bEnableGlobalScope, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHandlerPath_strSourceFilter, "*"));

    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHistoryLogPath_bEnableCatchAll, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHistoryLogPath_bEnable, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(component_config::g_strIncidentHistoryLogPath_nQueueSize,
                                          static_cast<int32_t>(500)));

    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentFileLogPath_bEnableCatchAll, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentFileLogPath_bEnable, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentFileLogPath_bEnableCSV, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentFileLogPath_strPath, ""));

    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentNotificationLogPath_bEnableCatchAll, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentNotificationLogPath_bEnable, false));
    ASSERT_EQ(a_util::result::SUCCESS, oPropTest.Validate(
                           component_config::g_strIncidentNotificationLogPath_strTarget, "*")); 
}