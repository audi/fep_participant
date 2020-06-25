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
* Test Case:   TestAssociateDisassociateStrategy
* Test ID:     1.3
* Test Title:  Test AssociateStrategy() and DisassociateStrategy()
* Description: Test functionality of the IIncidentHandler methods AssociateStrategy()
*              and DisassociateStrategy()
* Strategy:   Associate and disassociate all built in strategies. 
*              Associate and disassociate several custom strategies with SA_REPLACE and SA_APPEND 
*              for different incident codes. Invoke incidents for the different associated codes. 
*              Test cases are: 
*              Several strategies for the same incident code 
*              everal strategies for different incident codes 
*              One strategy for several incident codes
* Passed If:   Custom strategies are called only for their associated incidents.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1434 FEPSDK-1435 FEPSDK-1436 FEPSDK-1635 FEPSDK-1636 FEPSDK-1637
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
 * @req_id "FEPSDK-1434 FEPSDK-1435 FEPSDK-1436 FEPSDK-1635 FEPSDK-1636 FEPSDK-1637"
 */
TEST_F(TestFixture, TestAssociateDisassociateStrategy)
{

    cIncidentHandler* const pIH = dynamic_cast<cIncidentHandler* const>
        (m_pTestModule->GetIncidentHandler());

    // Activate Incident Handler
    ASSERT_EQ(a_util::result::SUCCESS, ConfigureIncidentHandler(pIH,
        component_config::g_strIncidentHandlerPath_bEnable, true));

    // Custom strategies to be associated and disassociated
    cMyTestStrategy oFirstCustStratForIncident_1;
    cMyTestStrategy oSecondCustStratForIncident_1;
    cMyTestStrategy oThirdCustStratForIncident_1;
    cMyTestStrategy oCustStratForIncident_2;
    cMyTestStrategy oCustStratForIncident_3;
    cMyTestStrategy oCustStratForIncident_1_2_3;

    // Associate one strategy
    pIH->AssociateStrategy(1, &oFirstCustStratForIncident_1, "", SA_APPEND);

    // First Incident (Code 1)
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(1, "First incident"));

    // Check who received it
    ASSERT_TRUE(oFirstCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oSecondCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oThirdCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oCustStratForIncident_2.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oCustStratForIncident_3.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oCustStratForIncident_1_2_3.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));

    ASSERT_EQ(oFirstCustStratForIncident_1.m_nIncidentCount, 1); // +1
    ASSERT_EQ(oSecondCustStratForIncident_1.m_nIncidentCount, 0);
    ASSERT_EQ(oThirdCustStratForIncident_1.m_nIncidentCount, 0);
    ASSERT_EQ(oCustStratForIncident_2.m_nIncidentCount, 0);
    ASSERT_EQ(oCustStratForIncident_3.m_nIncidentCount, 0);
    ASSERT_EQ(oCustStratForIncident_1_2_3.m_nIncidentCount, 0);

    ASSERT_EQ(oFirstCustStratForIncident_1.m_sLastEvent.nIncident, 1);
    ASSERT_EQ(oFirstCustStratForIncident_1.m_sLastEvent.strLastDescription, "First incident");
    ASSERT_NE(oFirstCustStratForIncident_1.m_sLastEvent.strOriginName.find(strTestModuleName), std::string::npos);

    // Associate more strategies
    pIH->AssociateStrategy(1, &oSecondCustStratForIncident_1, "", SA_APPEND);
    pIH->AssociateStrategy(2, &oCustStratForIncident_2, "", SA_APPEND);
    pIH->AssociateStrategy(3, &oCustStratForIncident_3, "", SA_APPEND);
    pIH->AssociateStrategy(1, &oCustStratForIncident_1_2_3, "", SA_APPEND);
    pIH->AssociateStrategy(2, &oCustStratForIncident_1_2_3, "", SA_APPEND);
    pIH->AssociateStrategy(3, &oCustStratForIncident_1_2_3, "", SA_APPEND);

    // Second Incident (Code 1)
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(1, "Second incident"));

    // Check who received it
    ASSERT_TRUE(oFirstCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_TRUE(oSecondCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oThirdCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oCustStratForIncident_2.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oCustStratForIncident_3.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_TRUE(oCustStratForIncident_1_2_3.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));

    ASSERT_EQ(oFirstCustStratForIncident_1.m_nIncidentCount , 2); // +1
    ASSERT_EQ(oSecondCustStratForIncident_1.m_nIncidentCount , 1); // +1
    ASSERT_EQ(oThirdCustStratForIncident_1.m_nIncidentCount , 0);
    ASSERT_EQ(oCustStratForIncident_2.m_nIncidentCount , 0);
    ASSERT_EQ(oCustStratForIncident_3.m_nIncidentCount , 0);
    ASSERT_EQ(oCustStratForIncident_1_2_3.m_nIncidentCount , 1); // +1

    ASSERT_EQ(oFirstCustStratForIncident_1.m_sLastEvent.nIncident , 1);
    ASSERT_EQ(oFirstCustStratForIncident_1.m_sLastEvent.strLastDescription , "Second incident");
    ASSERT_TRUE(oFirstCustStratForIncident_1.m_sLastEvent.strOriginName.find(strTestModuleName) != std::string::npos);
    ASSERT_EQ(oSecondCustStratForIncident_1.m_sLastEvent.nIncident , 1);
    ASSERT_EQ(oSecondCustStratForIncident_1.m_sLastEvent.strLastDescription , "Second incident");
    ASSERT_TRUE(oSecondCustStratForIncident_1.m_sLastEvent.strOriginName.find(strTestModuleName) != std::string::npos);
    ASSERT_EQ(oCustStratForIncident_1_2_3.m_sLastEvent.nIncident , 1);
    ASSERT_EQ(oCustStratForIncident_1_2_3.m_sLastEvent.strLastDescription , "Second incident");
    ASSERT_TRUE(oCustStratForIncident_1_2_3.m_sLastEvent.strOriginName.find(strTestModuleName) != std::string::npos);

    // Third Incident (Code 2)
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(2, "Third incident"));

    // Check who received it
    ASSERT_FALSE(oFirstCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oSecondCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oThirdCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_TRUE(oCustStratForIncident_2.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oCustStratForIncident_3.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_TRUE(oCustStratForIncident_1_2_3.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));

    ASSERT_EQ(oFirstCustStratForIncident_1.m_nIncidentCount , 2);
    ASSERT_EQ(oSecondCustStratForIncident_1.m_nIncidentCount , 1);
    ASSERT_EQ(oThirdCustStratForIncident_1.m_nIncidentCount , 0);
    ASSERT_EQ(oCustStratForIncident_2.m_nIncidentCount , 1); // +1
    ASSERT_EQ(oCustStratForIncident_3.m_nIncidentCount , 0);
    ASSERT_EQ(oCustStratForIncident_1_2_3.m_nIncidentCount , 2); // +1

    ASSERT_EQ(oCustStratForIncident_2.m_sLastEvent.nIncident , 2);
    ASSERT_EQ(oCustStratForIncident_2.m_sLastEvent.strLastDescription , "Third incident");
    ASSERT_NE(oCustStratForIncident_2.m_sLastEvent.strOriginName.find(strTestModuleName), std::string::npos);
    ASSERT_EQ(oCustStratForIncident_1_2_3.m_sLastEvent.nIncident , 2);
    ASSERT_EQ(oCustStratForIncident_1_2_3.m_sLastEvent.strLastDescription , "Third incident");
    ASSERT_NE(oCustStratForIncident_1_2_3.m_sLastEvent.strOriginName.find(strTestModuleName), std::string::npos);

    // Fourth Incident (Code 3)
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(3, "Fourth incident"));

    // Check who received it
    ASSERT_FALSE(oFirstCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oSecondCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oThirdCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oCustStratForIncident_2.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_TRUE(oCustStratForIncident_3.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_TRUE(oCustStratForIncident_1_2_3.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));

    ASSERT_EQ(oFirstCustStratForIncident_1.m_nIncidentCount , 2);
    ASSERT_EQ(oSecondCustStratForIncident_1.m_nIncidentCount , 1);
    ASSERT_EQ(oThirdCustStratForIncident_1.m_nIncidentCount , 0);
    ASSERT_EQ(oCustStratForIncident_2.m_nIncidentCount , 1);
    ASSERT_EQ(oCustStratForIncident_3.m_nIncidentCount , 1); // +1
    ASSERT_EQ(oCustStratForIncident_1_2_3.m_nIncidentCount , 3); // +1

    ASSERT_EQ(oCustStratForIncident_3.m_sLastEvent.nIncident , 3);
    ASSERT_EQ(oCustStratForIncident_3.m_sLastEvent.strLastDescription , "Fourth incident");
    ASSERT_NE(oCustStratForIncident_3.m_sLastEvent.strOriginName.find(strTestModuleName), std::string::npos);
    ASSERT_EQ(oCustStratForIncident_1_2_3.m_sLastEvent.nIncident , 3);
    ASSERT_EQ(oCustStratForIncident_1_2_3.m_sLastEvent.strLastDescription , "Fourth incident");
    ASSERT_NE(oCustStratForIncident_1_2_3.m_sLastEvent.strOriginName.find(strTestModuleName), std::string::npos);

    // Replace and disassociate strategies
    pIH->AssociateStrategy(1, &oThirdCustStratForIncident_1, "", SA_REPLACE);
    pIH->DisassociateStrategy(2, &oCustStratForIncident_2);
    pIH->DisassociateStrategy(3, &oCustStratForIncident_1_2_3);

    // Fifth Incident (Code 1)
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(1, "Fifth incident"));

    // Check who received it
    ASSERT_FALSE(oFirstCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oSecondCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_TRUE(oThirdCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oCustStratForIncident_2.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oCustStratForIncident_3.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oCustStratForIncident_1_2_3.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));

    ASSERT_EQ(oFirstCustStratForIncident_1.m_nIncidentCount , 2);
    ASSERT_EQ(oSecondCustStratForIncident_1.m_nIncidentCount , 1);
    ASSERT_EQ(oThirdCustStratForIncident_1.m_nIncidentCount , 1); // +1
    ASSERT_EQ(oCustStratForIncident_2.m_nIncidentCount , 1);
    ASSERT_EQ(oCustStratForIncident_3.m_nIncidentCount , 1);
    ASSERT_EQ(oCustStratForIncident_1_2_3.m_nIncidentCount , 3);

    ASSERT_EQ(oThirdCustStratForIncident_1.m_sLastEvent.nIncident , 1);
    ASSERT_EQ(oThirdCustStratForIncident_1.m_sLastEvent.strLastDescription , "Fifth incident");
    ASSERT_NE(oThirdCustStratForIncident_1.m_sLastEvent.strOriginName.find(strTestModuleName) , std::string::npos);

    // Sixth Incident (Code 2)
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(2, "Sixth incident"));

    // Check who received it
    ASSERT_FALSE(oFirstCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oSecondCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oThirdCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oCustStratForIncident_2.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oCustStratForIncident_3.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_TRUE(oCustStratForIncident_1_2_3.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));

    ASSERT_EQ(oFirstCustStratForIncident_1.m_nIncidentCount , 2);
    ASSERT_EQ(oSecondCustStratForIncident_1.m_nIncidentCount , 1);
    ASSERT_EQ(oThirdCustStratForIncident_1.m_nIncidentCount , 1);
    ASSERT_EQ(oCustStratForIncident_2.m_nIncidentCount , 1);
    ASSERT_EQ(oCustStratForIncident_3.m_nIncidentCount , 1);
    ASSERT_EQ(oCustStratForIncident_1_2_3.m_nIncidentCount , 4); // +1

    ASSERT_EQ(oCustStratForIncident_1_2_3.m_sLastEvent.nIncident , 2);
    ASSERT_EQ(oCustStratForIncident_1_2_3.m_sLastEvent.strLastDescription , "Sixth incident");
    ASSERT_NE(oCustStratForIncident_1_2_3.m_sLastEvent.strOriginName.find(strTestModuleName) , std::string::npos);

    // Seventh Incident (Code 3)
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(3, "Seventh incident"));

    // Check who received it
    ASSERT_FALSE(oFirstCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oSecondCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oThirdCustStratForIncident_1.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oCustStratForIncident_2.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_TRUE(oCustStratForIncident_3.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_FALSE(oCustStratForIncident_1_2_3.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));

    ASSERT_EQ(oFirstCustStratForIncident_1.m_nIncidentCount , 2);
    ASSERT_EQ(oSecondCustStratForIncident_1.m_nIncidentCount , 1);
    ASSERT_EQ(oThirdCustStratForIncident_1.m_nIncidentCount , 1);
    ASSERT_EQ(oCustStratForIncident_2.m_nIncidentCount , 1);
    ASSERT_EQ(oCustStratForIncident_3.m_nIncidentCount , 2); // +1
    ASSERT_EQ(oCustStratForIncident_1_2_3.m_nIncidentCount , 4);

    ASSERT_EQ(oCustStratForIncident_3.m_sLastEvent.nIncident , 3);
    ASSERT_EQ(oCustStratForIncident_3.m_sLastEvent.strLastDescription , "Seventh incident");
    ASSERT_NE(oCustStratForIncident_3.m_sLastEvent.strOriginName.find(strTestModuleName) , std::string::npos);

    // Disassociate remaining strategies
    pIH->DisassociateStrategy(1, &oFirstCustStratForIncident_1);
    pIH->DisassociateStrategy(1, &oSecondCustStratForIncident_1);
    pIH->DisassociateStrategy(1, &oThirdCustStratForIncident_1);
    pIH->DisassociateStrategy(3, &oCustStratForIncident_3);
    pIH->DisassociateStrategy(1, &oCustStratForIncident_1_2_3);
    pIH->DisassociateStrategy(2, &oCustStratForIncident_1_2_3);
}