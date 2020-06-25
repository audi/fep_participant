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
* Test Case:   TestHandleGlobalIncident
* Test ID:     5.2
* Test Title:  Test HandleGlobalIncident()
* Description: Testing the functionality of cNotificationStrategy::HandleGlobalIncident()
* Strategy:   The cNotificationStrategy gets configured as catch all strategy. "
*              A remote Incident is simulated.
* Passed If:   No incident is send out as notification. 
*              Should not be broadcasted again to avoid loops
*              
* Ticket:      #33515
* Requirement: FEPSDK-1442 FEPSDK-1443
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
 * @req_id "FEPSDK-1442 FEPSDK-1443"
 */
TEST_F(TestFixtureNotifStrat, TestHandleGlobalIncident)
{
    cIncidentHandler* const pIH = dynamic_cast<cIncidentHandler* const>(m_pTestModule->GetIncidentHandler());

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnableGlobalScope, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentNotificationLogPath_bEnableCatchAll, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentNotificationLogPath_bEnable, true));

    int16_t nCustomIncident = 4;
    std::string strCustomErrorMsg = "My custom error is here!";

    // Simulate remote Incident
    m_pTestTx->m_dequeNotifications.clear();
    fep::cIncidentNotification oLogNotification(nCustomIncident,
                        strCustomErrorMsg.c_str(), fep::SL_Critical_Global,
                        strTestModuleName.c_str(), "*", 0,
                        0);
    ASSERT_EQ(a_util::result::SUCCESS, pIH->Update(&oLogNotification));

    // No incident notification should be send
    ASSERT_EQ(0 , m_pTestTx->m_dequeNotifications.size());
}