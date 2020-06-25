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
* Test Case:   TestConfigurationOptions
* Test ID:     4.2
* Test Title:  Test Configuration Options
* Description: Test the configuration options of cIncidentHistoryStrategy: 
*               bEnable, bEnableCatchAll and nBufferSize
* Strategy:   1) Test incident logging to history for disabled strategy (default settings). 
*              2) Test incident logging to history for enabled strategy. 
*              3) Test incident logging to history for non associated incident code and catch all disabled. 
*              4) Test incident logging to history for non associated incident code and catch all enabled.
*              5) Test configuring buffer size.
* Passed If:   1) No incident is added to history. History entry cannot be retrieved. 
*              2) Incident is added to history. History entry can be retrieved.
*              3) Not associated incident is not added to history. 
*              4) Not associated incident is added to history. 
*              5) Buffer size can only be changed, when history is not currently locked. 
*              If setting the property was successful, the history buffer is set to the configured size.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1454 FEPSDK-1457 FEPSDK-1635
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
 * @req_id "FEPSDK-1454 FEPSDK-1457 FEPSDK-1635"
 */
TEST_F(TestFixtureIncidentStrat, TestConfigurationOptions)
{
    tIncidentListConstIter itHistBegin;
    tIncidentListConstIter itHistEnd;

    int16_t nCustomIncident = 4;
    std::string strCustomInfoMsg = "My custom info is here!";

    // disable the console logger to prevent it from logging into our stdout
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentConsoleLogPath_bEnable, false));

    // standard association
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->AssociateStrategy(nCustomIncident, ES_LogHistory, SA_REPLACE));

    // Test setting property bEnable

    // no incident is added to the history while strategy is disabled.
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_EQ(ERR_NOT_READY , m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));

    // enabling the strategy (whilst it is associated)
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_bEnable, true));
    ASSERT_EQ(ERR_EMPTY , m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));

    // first incident is added to the history
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));
    // history contains one entry
    itHistBegin++;
    ASSERT_TRUE(itHistBegin ==  itHistEnd);

    // unlock internal queue
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->FreeIncidentHistory());

    // Test setting property bEnableCatchAll

    // incident for non associated code is not added to the history (catch all is not enabled)
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident + 1, strCustomInfoMsg.c_str()));
    ASSERT_EQ(ERR_EMPTY , m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));

    // enable catchall "online" must work
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_bEnableCatchAll, true));

    // incident for non associated code is added to the history, because catch all is now enabled
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident + 1, strCustomInfoMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));
    // history contains one entry
    itHistBegin++;
    ASSERT_TRUE(itHistBegin ==  itHistEnd);

    // Test setting property nBufferSize

    // resetting the history when resizing (condition: Only change the internal queue if it's not locked)
    ASSERT_TRUE(ERR_ACCESS_DENIED == m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_nQueueSize,
        static_cast<int32_t>(15)));

    // unlock internal queue
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->FreeIncidentHistory());

    // resizing should work / e.g. not segfault and history is cleared
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_nQueueSize,
        static_cast<int32_t>(2)));
    ASSERT_EQ(ERR_EMPTY , m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));

    // buffer size has been set to two
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident + 2, strCustomInfoMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident + 3, strCustomInfoMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident + 4, strCustomInfoMsg.c_str()));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));
    // history contains two entries (three incidents invoked, but buffer size was set to two)
    itHistBegin += 2;
    ASSERT_TRUE(itHistBegin ==  itHistEnd);
}
