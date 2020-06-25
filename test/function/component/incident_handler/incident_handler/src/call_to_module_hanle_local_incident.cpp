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
* Test Case:   TestCallToModuleHandleLocalIncident
* Test ID:     1.2
* Test Title:  Test HandleLocalIncident of cModule
* Description: Test if cModule::HandleLocalIncident() is called for every occurring local incident
* Strategy:   1) Try to invoke incident locally, when the IH is deactivated. "
*              2) Activate IH and invoke incidents of all severity levels. "
*              3) Simulate a received remote incident, when the IH and its global scope are enabled.
* Passed If:   1) No incident can be invoked. No call to HandleLocalIncident() is made. 
*              2) Incidents are invoked and calls to HandleLocalIncident() are received.
*              3) No call to HandleLocalIncident() is made.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1438 FEPSDK-1439 FEPSDK-1667
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
 * @req_id "FEPSDK-1438 FEPSDK-1439 FEPSDK-1667"
 */
TEST_F(TestFixture, TestCallToModuleHandleLocalIncident)
{

    cIncidentHandler* const pIH = dynamic_cast<cIncidentHandler* const>(m_pTestModule->GetIncidentHandler());

    int16_t nCustomIncident = 15;
    std::string strCustomInfoMsg = "My custom info is here!";
    std::string strCustomWarningMsg = "My custom warning is here!";
    std::string strCustomErrorMsg = "My custom error is here!";
    std::string strCustomGlobalErrorMsg = "My custom global error is here!";

    // Deactivate Incident Handler
    ASSERT_EQ(a_util::result::SUCCESS, ConfigureIncidentHandler(pIH,
        component_config::g_strIncidentHandlerPath_bEnable, false));

    // Testing for the internal callback of cModule - handler is disabled completely.
    // cModule::HandleLocalIncident() is not called
    ASSERT_NE(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_FALSE(m_pTestModule->m_oIncidentEvent.wait_for(a_util::chrono::milliseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_EQ(m_pTestModule->m_sLastEvent.eSeverity, fep::SL_Info);
    ASSERT_EQ(m_pTestModule->m_sLastEvent.nIncident, 0);
    ASSERT_TRUE(m_pTestModule->m_sLastEvent.strLastDescription.empty());

    // Reactivating the Incident Handler.
    ASSERT_EQ(a_util::result::SUCCESS, ConfigureIncidentHandler(pIH,
        component_config::g_strIncidentHandlerPath_bEnable, true));

    // cModule::HandleLocalIncident() is called for all incident severity levels
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_TRUE(m_pTestModule->m_oIncidentEvent.wait_for(a_util::chrono::milliseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_EQ(m_pTestModule->m_sLastEvent.eSeverity, fep::SL_Info);
    ASSERT_EQ(m_pTestModule->m_sLastEvent.nIncident, nCustomIncident);
    ASSERT_NE(m_pTestModule->m_sLastEvent.strLastDescription.find(strTestModuleName), std::string::npos);
    ASSERT_NE(m_pTestModule->m_sLastEvent.strLastDescription.find(strCustomInfoMsg), std::string::npos);

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeWarning(nCustomIncident + 1, strCustomWarningMsg.c_str()));
    ASSERT_TRUE(m_pTestModule->m_oIncidentEvent.wait_for(a_util::chrono::milliseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_EQ(m_pTestModule->m_sLastEvent.eSeverity, fep::SL_Warning);
    ASSERT_EQ(m_pTestModule->m_sLastEvent.nIncident, nCustomIncident + 1);
    ASSERT_NE(m_pTestModule->m_sLastEvent.strLastDescription.find(strTestModuleName), std::string::npos);
    ASSERT_NE(m_pTestModule->m_sLastEvent.strLastDescription.find(strCustomWarningMsg), std::string::npos);

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeError(nCustomIncident + 2, strCustomErrorMsg.c_str()));
    ASSERT_TRUE(m_pTestModule->m_oIncidentEvent.wait_for(a_util::chrono::milliseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_EQ(m_pTestModule->m_sLastEvent.eSeverity, fep::SL_Critical_Local);
    ASSERT_EQ(m_pTestModule->m_sLastEvent.nIncident, nCustomIncident + 2);
    ASSERT_NE(m_pTestModule->m_sLastEvent.strLastDescription.find(strTestModuleName), std::string::npos);
    ASSERT_NE(m_pTestModule->m_sLastEvent.strLastDescription.find(strCustomErrorMsg), std::string::npos);

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeGlobalError(nCustomIncident + 3, strCustomGlobalErrorMsg.c_str()));
    ASSERT_TRUE(m_pTestModule->m_oIncidentEvent.wait_for(a_util::chrono::milliseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_EQ(m_pTestModule->m_sLastEvent.eSeverity, fep::SL_Critical_Global);
    ASSERT_EQ(m_pTestModule->m_sLastEvent.nIncident, nCustomIncident + 3);
    ASSERT_NE(m_pTestModule->m_sLastEvent.strLastDescription.find(strTestModuleName), std::string::npos);
    ASSERT_NE(m_pTestModule->m_sLastEvent.strLastDescription.find(strCustomGlobalErrorMsg), std::string::npos);

    // cModule::HandleLocalIncident() is not called for remote incidents
    // Activate global scope of the Incident Handler to receive remote incidents.
    ASSERT_EQ(a_util::result::SUCCESS, ConfigureIncidentHandler(pIH,
        component_config::g_strIncidentHandlerPath_bEnableGlobalScope, true));
    // Simulate remote Incident
    fep::cIncidentNotification oLogNotification(nCustomIncident + 4,
        strCustomGlobalErrorMsg.c_str(), fep::SL_Critical_Global,
        strTestModuleName.c_str(), "*", 0,
        0);
    ASSERT_EQ(a_util::result::SUCCESS, pIH->Update(&oLogNotification));
    // No LOCAL incident was received
    ASSERT_FALSE(m_pTestModule->m_oIncidentEvent.wait_for(a_util::chrono::milliseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_EQ(m_pTestModule->m_sLastEvent.eSeverity, fep::SL_Critical_Global);
    ASSERT_EQ(m_pTestModule->m_sLastEvent.nIncident, nCustomIncident + 3);
    ASSERT_NE(m_pTestModule->m_sLastEvent.strLastDescription.find(strTestModuleName), std::string::npos);
    ASSERT_NE(m_pTestModule->m_sLastEvent.strLastDescription.find(strCustomGlobalErrorMsg), std::string::npos);

}