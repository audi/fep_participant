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
* Test Case:   TestHandleGlobalIncident
* Test ID:     3.2
* Test Title:  Test HandleGlobalIncident()
* Description: Testing the functionality of cLogFileStrategy::HandleGlobalIncident()
* Strategy:   The cLogFileStrategy gets configured and associated for a certain incident code. "
*              A remote Incident for the associated error code is simulated.
* Passed If:   All invoked incidents are written to the configured log file.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1442 FEPSDK-1446 FEPSDK-1635
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
 * @req_id "FEPSDK-1442 FEPSDK-1446 FEPSDK-1449 FEPSDK-1635"
 */
TEST_F(TestFixtureFileStrat, TestHandleGlobalIncident)
{
    cIncidentHandler* const pIH = dynamic_cast<cIncidentHandler* const>(m_pTestModule->GetIncidentHandler());

    a_util::filesystem::Path strTestLogFile = "mylogfile.log";

    cLogFileTester oLogFileTester;

    // deleting leftover files...
    a_util::filesystem::remove(strTestLogFile);

    // Configure IH
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnableGlobalScope, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentConsoleLogPath_bEnable, false));

    int16_t nCustomIncident = 4;
    std::string strCustomErrorMsg = "My custom error is here!";

    // since everything is deactivated by default, no file should have been
    // created...
    ASSERT_FALSE(a_util::filesystem::exists(strTestLogFile));    

    // Apply a valid path and enable the strategy
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentFileLogPath_strPath,
                           strTestLogFile.toString().c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentFileLogPath_bEnable, true));

    // Log file should have been created
    ASSERT_TRUE(a_util::filesystem::exists(strTestLogFile));
    ASSERT_TRUE(oLogFileTester.Open(strTestLogFile, false));

    // Associate strategy
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->AssociateStrategy(nCustomIncident, ES_LogFile, SA_REPLACE));

    // Simulate remote Incident
    fep::cIncidentNotification oLogNotification(nCustomIncident,
                        strCustomErrorMsg.c_str(), fep::SL_Critical_Global,
                        strTestModuleName.c_str(), "*", 0,
                        0);
    ASSERT_EQ(a_util::result::SUCCESS, pIH->Update(&oLogNotification));

    // Test if remote incident has been logged
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester.ParseLogFile());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strTestModuleName.c_str()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine("Critical"));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(a_util::system::getHostname().c_str()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strCustomErrorMsg.c_str()));

    // deleting leftover files...
    oLogFileTester.Close();
    a_util::filesystem::remove(strTestLogFile);
}