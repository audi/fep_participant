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
* Test Case:   TestHandleLocalIncident
* Test ID:     2.1
* Test Title:  Test HandleLocalIncident()
* Description: Testing the functionality of cIncidentHistoryStrategy::HandleLocalIncident()
* Strategy:   The cIncidentHistoryStrategy gets configured as catch all strategy.
*              Incidents with different severity levels are invoked.
* Passed If:   All invoked incidents are written to the history log.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1442 FEPSDK-1454
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
 * @req_id "FEPSDK-1442 FEPSDK-1454"
 */
TEST_F(TestFixtureIncidentStrat, TestHandleLocalIncident)
{
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_bEnableCatchAll, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_bEnable, true));

    int16_t nCustomIncident = 22;
    std::string strCustomInfoMsg = "My custom info is here!";
    std::string strCustomWarningMsg = "My custom warning is here!";
    std::string strCustomErrorMsg = "My custom error is here!";
    std::string strCustomGlobalErrorMsg = "My custom global error is here!";

    // Testing whether the incident history works for all severity levels
    const tIncidentEntry* pLastIncident = NULL;

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->GetLastIncident(&pLastIncident));
    ASSERT_EQ(nCustomIncident , pLastIncident->nIncident);
    ASSERT_EQ(SL_Info , pLastIncident->eSeverity);
    ASSERT_EQ(strCustomInfoMsg , pLastIncident->strMessage);
    ASSERT_EQ(0 , a_util::strings::compare(strTestModuleName.c_str(), pLastIncident->strSource, 0, strTestModuleName.size()));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeWarning(nCustomIncident, strCustomWarningMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->GetLastIncident(&pLastIncident));
    ASSERT_EQ(nCustomIncident , pLastIncident->nIncident);
    ASSERT_EQ(SL_Warning , pLastIncident->eSeverity);
    ASSERT_EQ(strCustomWarningMsg , pLastIncident->strMessage);
    ASSERT_EQ(0 , a_util::strings::compare(strTestModuleName.c_str(), pLastIncident->strSource, 0, strTestModuleName.size()));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeError(nCustomIncident, strCustomErrorMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->GetLastIncident(&pLastIncident));
    ASSERT_EQ(nCustomIncident , pLastIncident->nIncident);
    ASSERT_EQ(SL_Critical_Local , pLastIncident->eSeverity);
    ASSERT_EQ(strCustomErrorMsg , pLastIncident->strMessage);
    ASSERT_EQ(0 , a_util::strings::compare(strTestModuleName.c_str(), pLastIncident->strSource, 0, strTestModuleName.size()));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeGlobalError(nCustomIncident, strCustomGlobalErrorMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->GetLastIncident(&pLastIncident));
    ASSERT_EQ(nCustomIncident , pLastIncident->nIncident);
    ASSERT_EQ(SL_Critical_Global , pLastIncident->eSeverity);
    ASSERT_EQ(strCustomGlobalErrorMsg , pLastIncident->strMessage);
    ASSERT_EQ(0 , a_util::strings::compare(strTestModuleName.c_str(), pLastIncident->strSource, 0, strTestModuleName.size()));
}