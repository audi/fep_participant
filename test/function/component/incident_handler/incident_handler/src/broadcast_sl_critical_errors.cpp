/** 
* Implementation of the tester for the broadcast of SL_Critical
* incidents in the FEP Incident Handler
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
 * @req_id "FEPSDK-1444"
 */
TEST_F(TestFixture, TestBroadcastSLCriticalIncidents)
{
    cTestBaseModule* pTestModule = new cTestBaseModule();
    ASSERT_OR_THROW(NULL == pTestModule->GetIncidentHandler());
    ASSERT_RESULT_OR_THROW(pTestModule->Create(cModuleOptions("TempTestModule")));
    ASSERT_OR_THROW(NULL != pTestModule->GetIncidentHandler());

    cTestBaseModule* pTestModule2 = new cTestBaseModule();
    ASSERT_OR_THROW(NULL == pTestModule2->GetIncidentHandler());
    ASSERT_RESULT_OR_THROW(pTestModule2->Create(cModuleOptions("TempTestModule2")));
    ASSERT_OR_THROW(NULL != pTestModule2->GetIncidentHandler());
    
    cIncidentHandler* const pIH = dynamic_cast<cIncidentHandler* const>(pTestModule->GetIncidentHandler());
    
    cIncidentHandler* const pIH2 = dynamic_cast<cIncidentHandler* const>(pTestModule2->GetIncidentHandler());

    int16_t nCustomIncident = 4;
    std::string strCustomErrorMsg = "SL Critical Error! It should be broadcasted";

    // Associate custom strategy to check call to HandleGlobalIncident()
    cMyTestStrategy oCustStrat;
    pIH->AssociateStrategy(nCustomIncident, &oCustStrat, "");
    
    cMyTestStrategy oCustStrat2;
    pIH2->AssociateStrategy(nCustomIncident, &oCustStrat2, "");


    // Activate Incident Handler
    ASSERT_EQ(a_util::result::SUCCESS, ConfigureIncidentHandler(pIH,
        component_config::g_strIncidentHandlerPath_bEnable, true));

    ASSERT_EQ(a_util::result::SUCCESS, ConfigureIncidentHandler(pIH2,
        component_config::g_strIncidentHandlerPath_bEnable, true));

    // Activate global scope of the Incident Handler to receive remote incidents.
    ASSERT_EQ(a_util::result::SUCCESS, ConfigureIncidentHandler(pIH,
        component_config::g_strIncidentHandlerPath_bEnableGlobalScope, true));

    ASSERT_EQ(a_util::result::SUCCESS, ConfigureIncidentHandler(pIH2,
        component_config::g_strIncidentHandlerPath_bEnableGlobalScope, true));

    // invoke a SL_Critical Incident in TempTestModule.
    ASSERT_EQ(a_util::result::SUCCESS, pIH2->InvokeIncident(4, fep::SL_Critical, strCustomErrorMsg.c_str(), "", 1, ""));

    // Incident should have been broadcasted and received in pTestModule as Remote Incident 
    ASSERT_TRUE(oCustStrat.m_oIncidentEvent.wait_for(a_util::chrono::microseconds(INCIDENCE_FEEDBACK_TIMEOUT)));
    ASSERT_EQ(oCustStrat.m_nRemoteIncidentCount, 1);
    ASSERT_EQ(oCustStrat.m_sLastEvent.eSeverity, fep::SL_Critical);
    ASSERT_NE(oCustStrat.m_sLastEvent.strLastDescription.find(strCustomErrorMsg), std::string::npos);
    ASSERT_NE(oCustStrat.m_sLastEvent.strOriginName.find("TempTestModule2"), std::string::npos);

    // Disassociate custom strategy
    pIH->DisassociateStrategy(nCustomIncident, &oCustStrat);
    pIH2->DisassociateStrategy(nCustomIncident, &oCustStrat2);

    pTestModule->Destroy();
    delete pTestModule;
    pTestModule = NULL;

    pTestModule2->Destroy();
    delete pTestModule2;
    pTestModule2 = NULL;
}
