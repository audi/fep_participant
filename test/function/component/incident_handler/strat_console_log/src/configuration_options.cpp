/**
* Implementation of the test fixture for the tester for the FEP Console Log Strategy
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
* Test ID:     2.3
* Test Title:  Test Configuration Options
* Description: Test the configuration options of cLogConsoleStrategy: 
*              bEnable and bEnableCatchAll
* Strategy:   1) Associate built in console log strategy. 
*           Invoke local incident with disabled strategy. 
*           2) Enable strategy and invoke local incident.
*           3) Set bEnableCatchAll to false and invoke a non associated incident. 
*           4) Set bEnableCatchAll to true and invoke a non associated incident.
* Passed If:  1) No incident is logged to console. 
*             2) Incident is logged to console. 
*             3) No non associated incident is logged to console. 
*             4) Non associated incident is logged to console.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1451  FEPSDK-1635
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
 * @req_id "FEPSDK-1451  FEPSDK-1635"
 */
TEST_F(TestFixtureConsoleStrat, TestConfigurationOptions)
{
    a_util::filesystem::Path strTestLogFile = "ConfigIncident.log";

    cLogFileTester oLogFileTester;

    // deleting leftover files...
    a_util::filesystem::remove(strTestLogFile);

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentConsoleLogPath_bEnableCatchAll, false));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentConsoleLogPath_bEnable, false));

    int16_t nCustomIncident = 13;
    std::string strCustomInfoMsg = "My custom info is here!";

    // standard association succeeds since the strategy is disabled (no matter any misconfiguration)
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->AssociateStrategy(nCustomIncident, ES_LogConsole, SA_REPLACE));

    // Test with disabled strategy

    // Console output cannot be checked directly.
    // We redirect stdout and stderr to a file and check the file content

    // Redirect console output to file
    RedirectStdoutToFile(strTestLogFile.toString().c_str());

    // Invoke info
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    RedirectToStdout();

    // File for console log should have been created
    ASSERT_TRUE(a_util::filesystem::exists(strTestLogFile));
    ASSERT_TRUE(oLogFileTester.Open(strTestLogFile, false));

    // Check if incident was NOT logged.
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester.ParseLogFile());
    ASSERT_FALSE(oLogFileTester.WasLoggedToFilePartialLastLine(m_pTestModule->GetName()));

    // Enable strategy
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentConsoleLogPath_bEnable, true));

    // Redirect console output to file
    RedirectStdoutToFile(strTestLogFile.toString().c_str());
    // Invoke info
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    RedirectToStdout();

    // File for console log should have been created
    ASSERT_TRUE(a_util::filesystem::exists(strTestLogFile));
    ASSERT_TRUE(oLogFileTester.Open(strTestLogFile, false));

    // Check if incident was logged
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester.ParseLogFile());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(m_pTestModule->GetName()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine("Info"));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(a_util::system::getHostname().c_str()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strCustomInfoMsg.c_str()));

    // Test with catch all disabled

    // Redirect console output to file
    RedirectStdoutToFile(strTestLogFile.toString().c_str());
    // Invoke info for non associated incident code
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident + 1, strCustomInfoMsg.c_str()));
    RedirectToStdout();

    // File for console log should have been created
    ASSERT_TRUE(a_util::filesystem::exists(strTestLogFile));
    ASSERT_TRUE(oLogFileTester.Open(strTestLogFile, false));

    // Check if incident was NOT logged.
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester.ParseLogFile());
    ASSERT_FALSE(oLogFileTester.WasLoggedToFilePartialLastLine(m_pTestModule->GetName()));

    // Enable catch all
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentConsoleLogPath_bEnableCatchAll, true));

    // Redirect console output to file
    RedirectStdoutToFile(strTestLogFile.toString().c_str());
    // Invoke info for non associated incident code
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident + 1, strCustomInfoMsg.c_str()));
    RedirectToStdout();

    // File for console log should have been created
    ASSERT_TRUE(a_util::filesystem::exists(strTestLogFile));
    ASSERT_TRUE(oLogFileTester.Open(strTestLogFile, false));

    // Check if incident was logged
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester.ParseLogFile());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(m_pTestModule->GetName()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine("Info"));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(a_util::system::getHostname().c_str()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strCustomInfoMsg.c_str()));

    // Sleeping a second to allow the async console log (RT compliant) to dequeue all
    // invoked incidents.
    a_util::system::sleepMilliseconds(500);

    // deleting leftover files...
    oLogFileTester.Close();
    a_util::filesystem::remove(strTestLogFile);
}