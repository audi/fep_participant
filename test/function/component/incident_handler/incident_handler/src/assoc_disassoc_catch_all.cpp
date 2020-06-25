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
* Test Case:   TestAssociateDisassociateCatchAllStrategy
* Test ID:     1.4
* Test Title:  Test AssociateCatchAllStrategy() and DisassociateCatchAllStrategy()
* Description: Test functionality of the IIncidentHandler methods AssociateCatchAllStrategy() "
*              and DisassociateCatchAllStrategy()
* Strategy:   Associate and disassociate several custom strategies with SA_REPLACE and SA_APPEND "
*              as catch all strategies.
* Passed If:   Only associated custom strategies are called for invoked incidents.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1434 FEPSDK-1440
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
 * @req_id "FEPSDK-1434 FEPSDK-1440"
 */
TEST_F(TestFixture, TestAssociateDisassociateCatchAllStrategy)
{
    cIncidentHandler* const pIH = dynamic_cast<cIncidentHandler* const>
        (m_pTestModule->GetIncidentHandler());

    // Activate Incident Handler
    ASSERT_EQ(a_util::result::SUCCESS, ConfigureIncidentHandler(pIH,
        component_config::g_strIncidentHandlerPath_bEnable, true));

    // Custom strategies to be associated and disassociated
    cMyTestStrategy oFirstCustStrat;
    cMyTestStrategy oSecondCustStrat;
    cMyTestStrategy oThirdCustStrat;

    // Associate one strategy
    ASSERT_EQ(a_util::result::SUCCESS, pIH->AssociateCatchAllStrategy(&oFirstCustStrat, "", SA_APPEND));

    // First Incident with some code
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(178, "First incident"));

    // Check who received it
    ASSERT_TRUE(oFirstCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oSecondCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oThirdCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));

    ASSERT_EQ(oFirstCustStrat.m_nIncidentCount , 1); // +1
    ASSERT_EQ(oSecondCustStrat.m_nIncidentCount , 0);
    ASSERT_EQ(oThirdCustStrat.m_nIncidentCount , 0);

    ASSERT_EQ(oFirstCustStrat.m_sLastEvent.nIncident , 178);
    ASSERT_EQ(oFirstCustStrat.m_sLastEvent.strLastDescription , "First incident");
    ASSERT_NE(oFirstCustStrat.m_sLastEvent.strOriginName.find(strTestModuleName) , std::string::npos);

    // Associate another strategy
    ASSERT_EQ(a_util::result::SUCCESS, pIH->AssociateCatchAllStrategy(&oSecondCustStrat, "", SA_APPEND));

    // Second Incident with some code
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(5, "Second incident"));

    // Check who received it
    ASSERT_TRUE(oFirstCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_TRUE(oSecondCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oThirdCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));

    ASSERT_EQ(oFirstCustStrat.m_nIncidentCount , 2); // +1
    ASSERT_EQ(oSecondCustStrat.m_nIncidentCount , 1); // +1
    ASSERT_EQ(oThirdCustStrat.m_nIncidentCount , 0);

    ASSERT_EQ(oFirstCustStrat.m_sLastEvent.nIncident , 5);
    ASSERT_EQ(oFirstCustStrat.m_sLastEvent.strLastDescription , "Second incident");
    ASSERT_NE(oFirstCustStrat.m_sLastEvent.strOriginName.find(strTestModuleName) , std::string::npos);
    ASSERT_EQ(oSecondCustStrat.m_sLastEvent.nIncident , 5);
    ASSERT_EQ(oSecondCustStrat.m_sLastEvent.strLastDescription , "Second incident");
    ASSERT_NE(oSecondCustStrat.m_sLastEvent.strOriginName.find(strTestModuleName) , std::string::npos);

    // Disassociate strategy
    ASSERT_EQ(a_util::result::SUCCESS, pIH->DisassociateCatchAllStrategy(&oSecondCustStrat));

    // Third Incident with some code
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(45, "Third incident"));

    // Check who received it
    ASSERT_TRUE(oFirstCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oSecondCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oThirdCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));

    ASSERT_EQ(oFirstCustStrat.m_nIncidentCount , 3); // +1
    ASSERT_EQ(oSecondCustStrat.m_nIncidentCount , 1);
    ASSERT_EQ(oThirdCustStrat.m_nIncidentCount , 0);

    ASSERT_EQ(oFirstCustStrat.m_sLastEvent.nIncident , 45);
    ASSERT_EQ(oFirstCustStrat.m_sLastEvent.strLastDescription , "Third incident");
    ASSERT_NE(oFirstCustStrat.m_sLastEvent.strOriginName.find(strTestModuleName) , std::string::npos);

    // Replace strategies
    ASSERT_EQ(a_util::result::SUCCESS, pIH->AssociateCatchAllStrategy(&oThirdCustStrat, "", SA_REPLACE));
    // shouln't be there anymore
    ASSERT_NE(a_util::result::SUCCESS, pIH->DisassociateCatchAllStrategy(&oFirstCustStrat));

    // Fourth Incident with some code
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(213, "Fourth incident"));

    // Check who received it
    ASSERT_FALSE(oFirstCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oSecondCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_TRUE(oThirdCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));

    ASSERT_EQ(oFirstCustStrat.m_nIncidentCount , 3); 
    ASSERT_EQ(oSecondCustStrat.m_nIncidentCount , 1);
    ASSERT_EQ(oThirdCustStrat.m_nIncidentCount , 1); // +1

    ASSERT_EQ(oThirdCustStrat.m_sLastEvent.nIncident , 213);
    ASSERT_EQ(oThirdCustStrat.m_sLastEvent.strLastDescription , "Fourth incident");
    ASSERT_NE(oThirdCustStrat.m_sLastEvent.strOriginName.find(strTestModuleName) , std::string::npos);

    // Disassociate remaining strategies
    ASSERT_EQ(a_util::result::SUCCESS, pIH->DisassociateCatchAllStrategy(&oThirdCustStrat));
}
