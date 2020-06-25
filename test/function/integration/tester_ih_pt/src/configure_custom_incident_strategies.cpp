/**
 * Implementation of the tester for the integration of FEP Incident Handler with the FEP Property Tree.
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
* Test Case:   TestConfigureCustomIncidentStrategies
* Test ID:     1.2
* Test Title:  Test Configure Custom Incident Strategies
* Description: Testing reconfiguration of associated custom strategies
* Strategy:    Two strategies are associated with AssociateStrategy() and AssociateCatchAllStrategy()
*              and a specified strConfigurationPath. A property on strConfigurationPath is changed.
*              
* Passed If:   RefreshConfiguration() is called for both strategies.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1435
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
#include "test_fixture.h"
#include <fep_ih_test_common.h>

/**
 * @req_id "FEPSDK-1435"
 */
TEST_F(TestFixture, TestConfigureCustomIncidentStrategies)
{
    cIncidentHandler* const pIH = dynamic_cast<cIncidentHandler* const>(m_pTestModule->GetIncidentHandler());

    // Enable Incident Handler
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnable, true));
    
    int16_t nCustomIncident = 4;
    double fFloatPropertyValue = 11.1;

    // Associate custom strategies to get reconfigured (both have the same strConfigurationPath)
    cMyTestStrategy oCustStrat;
    cMyTestStrategy oCatchAllCustStrat;

    ASSERT_EQ(a_util::result::SUCCESS, pIH->AssociateStrategy(nCustomIncident, &oCustStrat, "sMyStrategy"));
    ASSERT_EQ(a_util::result::SUCCESS, pIH->AssociateCatchAllStrategy(&oCatchAllCustStrat, "sMyStrategy"));

    // RefreshConfiguration() is also called at association. Check counter before changing property.
    uint32_t nRefreshCountCustStrat = oCustStrat.m_nRefreshPropCount;
    oCatchAllCustStrat.m_nRefreshPropCount;

    // Change a property on the associated strConfigurationPath
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           SOME_STRATEGY_PROPERTY, fFloatPropertyValue));

    // Check if RefreshConfiguration() was called for both strategies
    ASSERT_EQ(oCustStrat.m_nRefreshPropCount, nRefreshCountCustStrat + 1);
    ASSERT_EQ(oCatchAllCustStrat.m_fStrategyProperty, fFloatPropertyValue);
    ASSERT_EQ(oCustStrat.m_nRefreshPropCount, nRefreshCountCustStrat + 1);
    ASSERT_EQ(oCatchAllCustStrat.m_fStrategyProperty, fFloatPropertyValue);

    // Disassociate strategies
    ASSERT_EQ(a_util::result::SUCCESS, pIH->DisassociateStrategy(nCustomIncident, &oCustStrat));
    ASSERT_EQ(a_util::result::SUCCESS, pIH->DisassociateCatchAllStrategy(&oCatchAllCustStrat));

    // crash quickfix until bug in sdk is fixed (property listener still registered)
    TearDown();
}