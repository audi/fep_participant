/**
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
* Test Case:   TestRefreshConfiguration
* Test ID:     1.6
* Test Title:  Test RefreshConfiguration
* Description: Test call to RefreshConfiguration() for all associated custom strategies.
* Strategy:   Associate some custom strategies to the IH and call cIncidentHandler::RefreshConfiguration()
* Passed If:   RefreshConfiguration() is forwarded to all associated strategies
*              
* Ticket:      #33515
* Requirement: FEPSDK-1711
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
 * @req_id "FEPSDK-1711"
 */
TEST_F(TestFixture, TestRefreshConfiguration)
{
    cIncidentHandler* const pIH = dynamic_cast<cIncidentHandler* const>
        (m_pTestModule->GetIncidentHandler());

    // Activate Incident Handler
    ASSERT_EQ(a_util::result::SUCCESS, ConfigureIncidentHandler(pIH,
        component_config::g_strIncidentHandlerPath_bEnable, true));

    // Custom strategies to be associated
    cMyTestStrategy oFirstCustStrat;
    cMyTestStrategy oSecondCustStrat;
    cMyTestStrategy oThirdCustStrat;

    // Associate strategies
    pIH->AssociateStrategy(1, &oFirstCustStrat, "");
    pIH->AssociateStrategy(27, &oSecondCustStrat, "");
    pIH->AssociateCatchAllStrategy(&oThirdCustStrat, "");

    uint32_t nCallsToStrat1 = oFirstCustStrat.m_nRefreshPropCount;
    uint32_t nCallsToStrat2 = oSecondCustStrat.m_nRefreshPropCount;
    uint32_t nCallsToStrat3 = oThirdCustStrat.m_nRefreshPropCount;

    // Make call to RefreshConfiguration()
    pIH->RefreshConfiguration();

    // Check if RefreshConfiguration() has been called for all associated strategies
    ASSERT_EQ(oFirstCustStrat.m_nRefreshPropCount , nCallsToStrat1 + 1);
    ASSERT_EQ(oSecondCustStrat.m_nRefreshPropCount , nCallsToStrat2 + 1);
    ASSERT_EQ(oThirdCustStrat.m_nRefreshPropCount , nCallsToStrat3 + 1);

    // Disassociate all strategies
    pIH->DisassociateStrategy(1, &oFirstCustStrat);
    pIH->DisassociateStrategy(27, &oSecondCustStrat);
    pIH->DisassociateCatchAllStrategy(&oThirdCustStrat);
}