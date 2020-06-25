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
* Test Case:   TestHandleLocalIncident
* Test ID:     2.1
* Test Title:  Test HandleLocalIncident()
* Description: Testing the functionality of cLogConsoleStrategy::HandleLocalIncident()
* Strategy:   The cLogConsoleStrategy gets configured as catch all strategy.
*              Incidents with different severity levels are invoked.
* Passed If:   All invoked incidents are written to the console.
*              
* Ticket:      #33515
* Requirement: FEPSDK-1442 FEPSDK-1445
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
 * @req_id "FEPSDK-1442 FEPSDK-1445"
 */
TEST_F(TestFixtureConsoleStrat, TestHandleLocalIncident)
{
    a_util::filesystem::Path strTestLogFile = "LocalIncident.log";

    cLogFileTester oLogFileTester;

    // deleting leftover files...
    a_util::filesystem::remove(strTestLogFile);

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentHandlerPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentConsoleLogPath_bEnableCatchAll, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentConsoleLogPath_bEnable, true));

    int16_t nCustomIncident = 15;
    std::string strCustomInfoMsg = "My custom info is here!";
    std::string strCustomWarningMsg = "My custom warning is here!";
    std::string strCustomErrorMsg = "My custom error is here!";
    std::string strCustomGlobalErrorMsg = "My custom global error is here!";
    std::string strOrigin = "Tester_strat_console_log";
    std::string strFile = "/foo/bar.cpp";

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

    // Check if incident was logged
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester.ParseLogFile());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(m_pTestModule->GetName()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine("Info"));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(a_util::system::getHostname().c_str()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strCustomInfoMsg.c_str()));

    // Invoke warning
    RedirectStdoutToFile(strTestLogFile.toString().c_str());
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeWarning(nCustomIncident, strCustomWarningMsg.c_str()));
    RedirectToStdout();
    // Check if incident was logged
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester.ParseLogFile());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(m_pTestModule->GetName()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine("Warning"));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(a_util::system::getHostname().c_str()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strCustomWarningMsg.c_str()));

    // Invoke error
    RedirectStderrToFile(strTestLogFile.toString().c_str());
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeError(nCustomIncident, strCustomErrorMsg.c_str()));
    RedirectToStderr();
    // Check if incident was logged
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester.ParseLogFile());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(m_pTestModule->GetName()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine("Critical"));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(a_util::system::getHostname().c_str()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strCustomErrorMsg.c_str()));

    // invoke global error
    RedirectStderrToFile(strTestLogFile.toString().c_str());
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeGlobalError(nCustomIncident, strCustomGlobalErrorMsg.c_str()));
    RedirectToStderr();
    // Check if incident was logged
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester.ParseLogFile());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(m_pTestModule->GetName()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine("Critical"));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(a_util::system::getHostname().c_str()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strCustomGlobalErrorMsg.c_str()));

    // Sleeping a second to allow the async console log (RT compliant) to dequeue all
    // invoked incidents.
    a_util::system::sleepMilliseconds(1000);

    // deleting leftover files...
    oLogFileTester.Close();
    a_util::filesystem::remove(strTestLogFile);
}