/**
* Implementation of the tester for the FEP History Log Strategy
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
* Test Case:   TestConfigurationOptions
* Test ID:     5.3
* Test Title:  Test Configuration Options
* Description: Test the configuration options of cNotificationStrategy: 
*              bEnable, bEnableCatchAll and strTarget
* Strategy:   1) Test incident notifications for disabled strategy (default settings). 
*              2) Test incident notifications for enabled strategy and associated incident code. 
*              3) Test incident notifications for non associated incident code and catch all disabled. 
*              4) Test incident notifications for non associated incident code and catch all enabled.
*              5) Test configuring strTarget.
*              6) Test incident notification for error code FSI_TRANSM_MSG_TX_FAILED.
*              7) Test incident notifications for disabled strategy and specific strTarget.
* Passed If:  1) No notification for local incident is send. Global error is still send.
*              2) Notification for local incident is send.
*              3) No notification for local non associated incident is send. 
*              Global error for non associated incident is still send. 
*              4) Notification for local not associated incident is send. 
*              5) Notification for local incident is send to strTarget. 
*              Notification for global error incident is broadcasted to *. 
*              6) No notification for incident code FSI_TRANSM_MSG_TX_FAILED is send. 
*                Would result in endless loop. 
*              7) Notification for global error incident is still broadcasted to *
*              
* Ticket:      #24718, #26657, #33515
* Requirement: FEPSDK-1443 FEPSDK-1462 FEPSDK-1634 FEPSDK-1635
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include <fep_test_common.h>
#include <fep_ih_test_common.h>

#include <fstream>
#include <iostream>

#include "test_fixture.h"

using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

/**
 * @req_id "FEPSDK-1443 FEPSDK-1462 FEPSDK-1634 FEPSDK-1635"
 */
TEST_F(TestFixtureNotifStrat, TestConfigurationOptions)
{
    // Disable the notification logger for now
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHandlerPath_bEnableGlobalScope, false));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentNotificationLogPath_bEnable, false));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentNotificationLogPath_strTarget, "*"));

    // disable the console logger to prevent it from logging into our stdout
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentConsoleLogPath_bEnable, false));

    int16_t nCustomIncident = 15;
    std::string strCustomInfoMsg = "My custom info is here!";
    std::string strCustomWarningMsg = "My custom warning is here!";
    std::string strCustomErrorMsg = "My custom error is here!";
    std::string strCustomGlobalErrorMsg = "My custom global error is here!";

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->AssociateStrategy(
        nCustomIncident, ES_LogNotification, SA_REPLACE));

    // invoking an information here! Strategy is disabled!
    m_pTestTx->m_dequeNotifications.clear();
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_EQ(0 ,  m_pTestTx->m_dequeNotifications.size());

    // Critical global notifications, however, are always broadcasted!
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeGlobalError(nCustomIncident, strCustomGlobalErrorMsg.c_str()));
    ASSERT_EQ(1 , m_pTestTx->m_dequeNotifications.size());
    ASSERT_EQ(nCustomIncident , m_pTestTx->m_dequeNotifications.back().nIncident);
    ASSERT_EQ(SL_Critical_Global , m_pTestTx->m_dequeNotifications.back().nSeverity);
    ASSERT_EQ(strCustomGlobalErrorMsg , m_pTestTx->m_dequeNotifications.back().strDescription);
    ASSERT_TRUE(0 == a_util::strings::compare(strTestModuleName.c_str(),
        m_pTestTx->m_dequeNotifications.back().strSender.c_str(), 0, strTestModuleName.size()));

    // Enable strategy
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentNotificationLogPath_bEnable, true));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_EQ(2 , m_pTestTx->m_dequeNotifications.size());
    ASSERT_EQ(nCustomIncident , m_pTestTx->m_dequeNotifications.back().nIncident);
    ASSERT_EQ(SL_Info , m_pTestTx->m_dequeNotifications.back().nSeverity);
    ASSERT_EQ(strCustomInfoMsg , m_pTestTx->m_dequeNotifications.back().strDescription);
    ASSERT_TRUE(0 == a_util::strings::compare(strTestModuleName.c_str(),
        m_pTestTx->m_dequeNotifications.back().strSender.c_str(), 0, strTestModuleName.size()));

    // now for notifications, "global" scoped error level incidents are always supposed
    // to be distributed and published.
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->DisassociateStrategy(
        nCustomIncident, ES_LogNotification));

    // catchall is generally disabled
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentNotificationLogPath_bEnableCatchAll, false));

    // however, global criticals are always getting through!
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeGlobalError(nCustomIncident - 1, strCustomGlobalErrorMsg.c_str()));
    ASSERT_EQ(3 , m_pTestTx->m_dequeNotifications.size());
    ASSERT_EQ(nCustomIncident -1 , m_pTestTx->m_dequeNotifications.back().nIncident);
    ASSERT_EQ(SL_Critical_Global , m_pTestTx->m_dequeNotifications.back().nSeverity);
    ASSERT_EQ(strCustomGlobalErrorMsg , m_pTestTx->m_dequeNotifications.back().strDescription);
    ASSERT_TRUE(0 == a_util::strings::compare(strTestModuleName.c_str(),
        m_pTestTx->m_dequeNotifications.back().strSender.c_str(), 0, strTestModuleName.size()));

    // local unassociated incidents
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeWarning(nCustomIncident + 1, strCustomWarningMsg.c_str()));
    ASSERT_EQ(3 , m_pTestTx->m_dequeNotifications.size());

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeError(nCustomIncident + 2, strCustomErrorMsg.c_str()));
    ASSERT_EQ(4 , m_pTestTx->m_dequeNotifications.size());

    // catch-all enabled
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentNotificationLogPath_bEnableCatchAll, true));
    // local unassociated incident
    nCustomIncident = 25;
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_EQ(5 , m_pTestTx->m_dequeNotifications.size());
    ASSERT_EQ(nCustomIncident , m_pTestTx->m_dequeNotifications.back().nIncident);
    ASSERT_EQ(SL_Info , m_pTestTx->m_dequeNotifications.back().nSeverity);
    ASSERT_EQ(strCustomInfoMsg , m_pTestTx->m_dequeNotifications.back().strDescription);
    ASSERT_TRUE(0 == a_util::strings::compare(strTestModuleName.c_str(),
        m_pTestTx->m_dequeNotifications.back().strSender.c_str(), 0, strTestModuleName.size()));

    // testing specific log targets.
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentNotificationLogPath_strTarget,
        "MyTargetModule"));

    nCustomIncident = 70;
    m_pTestTx->ClearLog();

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_EQ(1 , m_pTestTx->m_dequeNotifications.size());
    ASSERT_EQ(nCustomIncident , m_pTestTx->m_dequeNotifications.back().nIncident);
    ASSERT_EQ(SL_Info , m_pTestTx->m_dequeNotifications.back().nSeverity);
    ASSERT_EQ(strCustomInfoMsg , m_pTestTx->m_dequeNotifications.back().strDescription);
    ASSERT_TRUE(0 == a_util::strings::compare(strTestModuleName.c_str(),
        m_pTestTx->m_dequeNotifications.back().strSender.c_str(), 0, strTestModuleName.size()));
    // Target was strTarget
    ASSERT_EQ(m_pTestTx->m_strNotifTarget , "MyTargetModule");

    // global critical incidents are always being broadcasted to "*"!
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeGlobalError(nCustomIncident, strCustomGlobalErrorMsg.c_str()));
    ASSERT_EQ(2 , m_pTestTx->m_dequeNotifications.size());
    ASSERT_EQ(nCustomIncident , m_pTestTx->m_dequeNotifications.back().nIncident);
    ASSERT_EQ(SL_Critical_Global , m_pTestTx->m_dequeNotifications.back().nSeverity);
    ASSERT_EQ(strCustomGlobalErrorMsg , m_pTestTx->m_dequeNotifications.back().strDescription);
    ASSERT_TRUE(0 == a_util::strings::compare(strTestModuleName.c_str(),
        m_pTestTx->m_dequeNotifications.back().strSender.c_str(), 0, strTestModuleName.size()));
    // Target was not strTarget
    ASSERT_EQ(m_pTestTx->m_strNotifTarget , "*");

    // Vital: The notification log strategy must not attempt to handle
    // the FSI_TRANSM_MSG_TX_FAILED incident code - this would result in endless loops
    // with the transmission adapter.
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(FSI_TRANSM_MSG_TX_FAILED, "This should never ever arrive!"));
    ASSERT_EQ(2 , m_pTestTx->m_dequeNotifications.size());

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeGlobalError(FSI_TRANSM_MSG_TX_FAILED, "This should never ever arrive!"));
    ASSERT_EQ(2 , m_pTestTx->m_dequeNotifications.size());

    // Importantly, even if disabled, this strategy *must* broadcast(!) critical
    // incidents which are labelled to be of global significance - this is what this severity
    // is all about!
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentNotificationLogPath_strTarget,
        "AnonymousModule"));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentNotificationLogPath_bEnable,
        false));

    nCustomIncident += 1;
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeGlobalError(nCustomIncident, strCustomGlobalErrorMsg.c_str()));
    ASSERT_EQ(3 , m_pTestTx->m_dequeNotifications.size());
    ASSERT_EQ(nCustomIncident , m_pTestTx->m_dequeNotifications.back().nIncident);
    ASSERT_EQ(SL_Critical_Global , m_pTestTx->m_dequeNotifications.back().nSeverity);
    ASSERT_EQ(strCustomGlobalErrorMsg , m_pTestTx->m_dequeNotifications.back().strDescription);
    ASSERT_TRUE(0 == a_util::strings::compare(strTestModuleName.c_str(),
        m_pTestTx->m_dequeNotifications.back().strSender.c_str(), 0, strTestModuleName.size()));
    // Target was not strTarget
    ASSERT_EQ(m_pTestTx->m_strNotifTarget , "*");
}