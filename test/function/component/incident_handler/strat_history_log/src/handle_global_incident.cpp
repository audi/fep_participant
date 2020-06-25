/**
* Implementation of the tester for the FEP Console Log Strategy
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
* Test ID:     4.2
* Test Title:  Test TestHandleGlobalIncident()
* Description: Testing the functionality of cIncidentHistoryStrategy::TestHandleGlobalIncident()
* Strategy:   The cIncidentHistoryStrategy gets configured as catch all strategy.
*              A remote Incident for the associated error code is simulated.
* Passed If:   The invoked incident is written to the history log.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1442 FEPSDK-1454 FEPSDK-1635
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
 * @req_id "FEPSDK-1442 FEPSDK-1454 FEPSDK-1635"
 */
TEST_F(TestFixtureIncidentStrat, TestHandleGlobalIncident)
{
    cIncidentHandler* const pIH = dynamic_cast<cIncidentHandler* const>(m_pTestModule->GetIncidentHandler());

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnableGlobalScope, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHistoryLogPath_bEnableCatchAll, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHistoryLogPath_bEnable, true));

    int16_t nCustomIncident = 4;
    std::string strCustomErrorMsg = "My custom error is here!";

    // Simulate remote Incident
    fep::cIncidentNotification oLogNotification(nCustomIncident,
                        strCustomErrorMsg.c_str(), fep::SL_Critical_Global,
                        strTestModuleName.c_str(), "*", 0,
                        0);
    ASSERT_EQ(a_util::result::SUCCESS, pIH->Update(&oLogNotification));

    // Incident should have been received
    const tIncidentEntry* pLastIncident = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->GetLastIncident(&pLastIncident));

    ASSERT_EQ(nCustomIncident , pLastIncident->nIncident);
    ASSERT_EQ(SL_Critical_Global , pLastIncident->eSeverity);
    ASSERT_EQ(strCustomErrorMsg , pLastIncident->strMessage);
    ASSERT_EQ(strTestModuleName , pLastIncident->strSource);
}