/**
 * Implementation of the tester for the integration of FEP Incident Handler with the FEP Property Tree.
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
* Test Case:   TestSetPropertiesToConfigureIH
* Test ID:     1.1
* Test Title:  Test Set Properties to configure IH
* Description: Configure the Incident Handler by setting its properties in the Property Tree. 
*              Test if the configuration was successful by testing the reception of remote incidents. 
*              Test sequence is the same as in TestReceivingRemoteIncidents, but IH is configured 
*              over PT here.
* Strategy:  Associate custom test strategy to register received remote incidents.
*             1) Simulate remote incident with enabled IH and enabled global scope. 
*             2) Simulate remote incident with enabled IH and disabled global scope. 
*             3) Simulate remote incident with disabled IH. 
*             4) Simulate remote incident with enabled IH, enabled global scope 
*                and source filter property set to TestModuleName. 
*             5) Simulate remote incident with enabled IH, enabled global scope and source
*                filter property set to *.
*              
* Passed If:   1) Custom strategy HandleGlobalIncident() has been called. 
*              2) No Custom strategy HandleGlobalIncident() has been called. 
*              3) cIncidentHandler::Update() returns an error. No Custom strategy HandleGlobalIncident() has been called. 
*              4) Custom strategy HandleGlobalIncident() has been called for incident from TestModuleName, 
*               but not from other source.
*              5) Custom strategy HandleGlobalIncident() has been called for two different sources.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1437 FEPSDK-1441
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
#include "test_fixture.h"
#include <fep_ih_test_common.h>

/**
 * @req_id "FEPSDK-1437 FEPSDK-1441"
 */
TEST_F(TestFixture, TestSetPropertiesToConfigureIH)
{
    cIncidentHandler* const pIH = dynamic_cast<cIncidentHandler* const>(m_pTestModule->GetIncidentHandler());

    // Set IH Properties to enable IH, enable global scope and no source filtering for remote incidents
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnableGlobalScope, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_strSourceFilter, "*"));

    int16_t nCustomIncident = 4;
    std::string strCustomErrorMsg = "My custom error is here!";

    // Associate custom strategy to check call to HandleGlobalIncident()
    cMyTestStrategy oCustStrat;
    ASSERT_EQ(a_util::result::SUCCESS, pIH->AssociateStrategy(nCustomIncident, &oCustStrat, ""));

    // Simulate remote Incident
    fep::cIncidentNotification oLogNotification(nCustomIncident,
                        strCustomErrorMsg.c_str(), fep::SL_Critical_Global,
                        strTestModuleName.c_str(), "*", 0,
                        0);
    ASSERT_EQ(a_util::result::SUCCESS, pIH->Update(&oLogNotification));

    // Incident should have been received
    ASSERT_TRUE(oCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_EQ(oCustStrat.m_nRemoteIncidentCount, 1);
    ASSERT_NE(oCustStrat.m_sLastEvent.strLastDescription.find(strCustomErrorMsg), std::string::npos);
    ASSERT_NE(oCustStrat.m_sLastEvent.strOriginName.find(strTestModuleName), std::string::npos);

    // Deactivate global scope of the Incident Handler to stop receiving remote incidents.
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnableGlobalScope, false));

    // Simulate remote Incident
    ASSERT_EQ(a_util::result::SUCCESS, pIH->Update(&oLogNotification));

    // No Incident should have been received
    ASSERT_TRUE(!oCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_EQ(oCustStrat.m_nRemoteIncidentCount, 1);

    // Reactivate global scope of the Incident Handler
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnableGlobalScope, true));

    // Simulate remote Incident
    ASSERT_EQ(a_util::result::SUCCESS, pIH->Update(&oLogNotification));

    // Incident should have been received
    ASSERT_TRUE(oCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_EQ(oCustStrat.m_nRemoteIncidentCount, 2);
    ASSERT_NE(oCustStrat.m_sLastEvent.strLastDescription.find(strCustomErrorMsg), std::string::npos);
    ASSERT_NE(oCustStrat.m_sLastEvent.strOriginName.find(strTestModuleName), std::string::npos);

    // Deactivate the Incident Handler
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnable, false));

    // Simulate remote Incident while IH is disabled
    ASSERT_EQ(ERR_NOT_READY, pIH->Update(&oLogNotification));

    // No Incident should have been received
    ASSERT_FALSE(oCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_EQ(oCustStrat.m_nRemoteIncidentCount, 2);

    // Reactivate the Incident Handler to receive remote incidents.
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnable, true));

    // Set source filter property of the Incident Handler to filter received remote incidents.
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_strSourceFilter, strTestModuleName.c_str()));

    // Simulate remote Incident from filtered source
    ASSERT_EQ(a_util::result::SUCCESS, pIH->Update(&oLogNotification));

    // Incident should have been received
    ASSERT_TRUE(oCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_EQ(oCustStrat.m_nRemoteIncidentCount, 3);
    ASSERT_NE(oCustStrat.m_sLastEvent.strLastDescription.find(strCustomErrorMsg), std::string::npos);
    ASSERT_NE(oCustStrat.m_sLastEvent.strOriginName.find(strTestModuleName), std::string::npos);

    // Simulate remote Incident from another source
    fep::cIncidentNotification oLogNotificationNotWanted(nCustomIncident,
                        strCustomErrorMsg.c_str(), fep::SL_Critical_Global,
                        "NotTheExpectedModule", "*", 0,
                        0);
    ASSERT_EQ(a_util::result::SUCCESS, pIH->Update(&oLogNotificationNotWanted));

    // No Incident should have been received
    ASSERT_TRUE(!oCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_EQ(oCustStrat.m_nRemoteIncidentCount, 3);

    // Set source filter to receive remote properties from everybody
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_strSourceFilter, "*"));

    // Simulate remote Incident from another source
    ASSERT_EQ(a_util::result::SUCCESS, pIH->Update(&oLogNotificationNotWanted));

    // Incident should have been received
    ASSERT_TRUE(oCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_EQ(oCustStrat.m_nRemoteIncidentCount, 4);
    ASSERT_NE(oCustStrat.m_sLastEvent.strLastDescription.find(strCustomErrorMsg), std::string::npos);
    ASSERT_NE(oCustStrat.m_sLastEvent.strOriginName.find("NotTheExpectedModule"), std::string::npos);

    // Disassociate custom strategy
    ASSERT_EQ(a_util::result::SUCCESS, pIH->DisassociateStrategy(nCustomIncident, &oCustStrat));

    // crash quickfix until bug in sdk is fixed (property listener still registered)
    TearDown();
}
