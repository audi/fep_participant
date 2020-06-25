/**
 * Implementation of the tester for the FEP Incident Notification Strategy
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
* Test Case:   TestHandleLocalIncident
* Test ID:     5.1
* Test Title:  Test HandleLocalIncident()
* Description: Testing the functionality of cNotificationStrategy::HandleLocalIncident()
* Strategy:   The cNotificationStrategy gets configured as catch all strategy.
*              Incidents with different severity levels are invoked.
* Passed If:   All invoked incidents are send out as notification.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1442 FEPSDK-1461
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include <fep_test_common.h>
#include <fep_ih_test_common.h>
#include <fep_mock_tx_driver.h>

using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

#include "test_fixture.h"

/**
 * @req_id "FEPSDK-1442 FEPSDK-1461"
 */
TEST_F(TestFixtureNotifStrat, TestHandleLocalIncident)
{
        ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentNotificationLogPath_bEnableCatchAll, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentNotificationLogPath_bEnable, true));

    int16_t nCustomIncident = 8;
    std::string strCustomInfoMsg = "My custom info is here!";
    std::string strCustomWarningMsg = "My custom warning is here!";
    std::string strCustomErrorMsg = "My custom error is here!";
    std::string strCustomGlobalErrorMsg = "My custom global error is here!";

    // Testing whether the incident notification strategy works for all severity levels

    m_pTestTx->m_dequeNotifications.clear();
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_EQ(1 , m_pTestTx->m_dequeNotifications.size());
    ASSERT_EQ(nCustomIncident , m_pTestTx->m_dequeNotifications.back().nIncident);
    ASSERT_EQ(SL_Info , m_pTestTx->m_dequeNotifications.back().nSeverity);
    ASSERT_EQ(strCustomInfoMsg , m_pTestTx->m_dequeNotifications.back().strDescription);
    ASSERT_TRUE(0 == a_util::strings::compare(strTestModuleName.c_str(),
        m_pTestTx->m_dequeNotifications.back().strSender.c_str(), 0, strTestModuleName.size()));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeWarning(nCustomIncident + 1, strCustomWarningMsg.c_str()));
    ASSERT_EQ(2 , m_pTestTx->m_dequeNotifications.size());
    ASSERT_EQ(nCustomIncident + 1 , m_pTestTx->m_dequeNotifications.back().nIncident);
    ASSERT_EQ(SL_Warning , m_pTestTx->m_dequeNotifications.back().nSeverity);
    ASSERT_EQ(strCustomWarningMsg , m_pTestTx->m_dequeNotifications.back().strDescription);
    ASSERT_TRUE(0 == a_util::strings::compare(strTestModuleName.c_str(),
        m_pTestTx->m_dequeNotifications.back().strSender.c_str(), 0, strTestModuleName.size()));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeError(nCustomIncident + 2, strCustomErrorMsg.c_str()));
    ASSERT_EQ(3 , m_pTestTx->m_dequeNotifications.size());
    ASSERT_EQ(nCustomIncident + 2 , m_pTestTx->m_dequeNotifications.back().nIncident);
    ASSERT_EQ(SL_Critical_Local , m_pTestTx->m_dequeNotifications.back().nSeverity);
    ASSERT_EQ(strCustomErrorMsg , m_pTestTx->m_dequeNotifications.back().strDescription);
    ASSERT_TRUE(0 == a_util::strings::compare(strTestModuleName.c_str(),
        m_pTestTx->m_dequeNotifications.back().strSender.c_str(), 0, strTestModuleName.size()));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeGlobalError(nCustomIncident + 3, strCustomGlobalErrorMsg.c_str()));
    ASSERT_EQ(4 , m_pTestTx->m_dequeNotifications.size());
    ASSERT_EQ(nCustomIncident + 3 , m_pTestTx->m_dequeNotifications.back().nIncident);
    ASSERT_EQ(SL_Critical_Global , m_pTestTx->m_dequeNotifications.back().nSeverity);
    ASSERT_EQ(strCustomGlobalErrorMsg , m_pTestTx->m_dequeNotifications.back().strDescription);
    ASSERT_TRUE(0 == a_util::strings::compare(strTestModuleName.c_str(),
        m_pTestTx->m_dequeNotifications.back().strSender.c_str(), 0, strTestModuleName.size()));
}