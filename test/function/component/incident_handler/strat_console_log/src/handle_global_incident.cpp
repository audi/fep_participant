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
* Test ID:     2.1
* Test Title:  Test HandleGlobalIncident()
* Description: Testing the functionality of cLogConsoleStrategy::HandleGlobalIncident()
* Strategy:   The cLogConsoleStrategy gets configured as catch all strategy. "
*             A remote Incident is simulated.
* Passed If:  The invoked incident is written to the console.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1442
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
 * @req_id "FEPSDK-1442"
 */
TEST_F(TestFixtureConsoleStrat, TestHandleGlobalIncident)
{
    a_util::filesystem::Path strTestLogFile = "GlobalIncident.log";

    cLogFileTester oLogFileTester;

    // deleting leftover files...
    a_util::filesystem::remove(strTestLogFile);

    cIncidentHandler* const pIH = dynamic_cast<cIncidentHandler* const>(m_pTestModule->GetIncidentHandler());

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHandlerPath_bEnableGlobalScope, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentConsoleLogPath_bEnableCatchAll, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentConsoleLogPath_bEnable, true));

    int16_t nCustomIncident = 3;
    std::string strCustomErrorMsg = "My custom error is here!";

    // Console output cannot be checked directly.
    // We redirect stdout and stderr to a file and check the file content

    // Redirect console output to file
    RedirectStderrToFile(strTestLogFile.toString().c_str());

    // Simulate remote Incident
    fep::cIncidentNotification oLogNotification(nCustomIncident,
                        strCustomErrorMsg.c_str(), fep::SL_Critical_Global,
                        strTestModuleName.c_str(), "*", 0,
                        0);
    ASSERT_EQ(a_util::result::SUCCESS, pIH->Update(&oLogNotification));

    RedirectToStderr();

    // File for console log should have been created
    ASSERT_TRUE(a_util::filesystem::exists(strTestLogFile));
    ASSERT_TRUE(oLogFileTester.Open(strTestLogFile, false));

    // Check if incident was logged
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester.ParseLogFile());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strTestModuleName.c_str()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine("Critical"));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(a_util::system::getHostname().c_str()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strCustomErrorMsg.c_str()));

    // Sleeping a second to allow the async console log (RT compliant) to dequeue all
    // invoked incidents.
    a_util::system::sleepMilliseconds(1000);

    // deleting leftover files...
    oLogFileTester.Close();
    a_util::filesystem::remove(strTestLogFile);
}