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
* Test ID:     3.3
* Test Title:  Test Configuration Options
* Description: Test the configuration options of cLogFileStrategy: 
*              bEnable, strPath, bEnableCatchAll, bOverwriteExisting and bEnableCSV
* Strategy:   1) Test creation of log file for disabled strategy (default settings). 
*            2) Associate built in log file strategy. 
*            3) Enable strategy without setting valid path. 
*            4) Configure a nonexisting path.
*            5) Configure valid path and enable strategy. 
*            6) Invoke local incident. 
*            7) Disable strategy and invoke another incident. 
*            8) Reactivate strategy with option bOverwriteExisting set to false. 
*            9) Set bEnableCatchAll to false and invoke a non associated incident. 
*            10) Set bEnableCatchAll to true and invoke a non associated incident. 
*            11) Set bOverwriteExisting to true and invoke further incidents. 
*            12) Set bEnableCSV to true and invoke incident
* Passed If:  1) No log file is created. 
*             2) No error occurs.
*             3) ERR_INVALID_ARG. Strategy cannot be enabled. 
*             4) ERR_OPEN_FAILED. Path cannot be configured. 
*             5) No error occurs. Log file is created
*             6) Incident is logged to configured log file. 
*             7) No incident is logged to configured log file. Log file still exists. 
*             8) Content of existing log file is not erased. 
*             9) No incident is logged. 
*             10) Incident is logged to file. 
*             11) Content of existing log file is erased. 
*             12) Incident is logged to file in csv format.
*            
* Ticket:      #33515
* Requirement: FEPSDK-1446 FEPSDK-1447 FEPSDK-1448 FEPSDK-1450 FEPSDK-1451 FEPSDK-1452 FEPSDK-1453 FEPSDK-1635
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
 * @req_id "FEPSDK-1446 FEPSDK-1447 FEPSDK-1448 FEPSDK-1450 FEPSDK-1451 FEPSDK-1452 FEPSDK-1453 FEPSDK-1635"
 */
TEST_F(TestFixtureFileStrat, TestConfigurationOptions)
{
    a_util::filesystem::Path strTestLogFile = "mylogfile.log";

    a_util::filesystem::Path strTestLogFile2 = "mylogfile2.log";

    cLogFileTester oLogFileTester;
    cLogFileTester oLogFileTester2;

    // deleting leftover files...
    a_util::filesystem::remove(strTestLogFile);
    a_util::filesystem::remove(strTestLogFile2);

    int16_t nCustomIncident = 15;
    std::string strCustomInfoMsg = "My custom info is here!";
    std::string strCustomWarningMsg = "My custom warning is here!";
    std::string strCustomErrorMsg = "My custom error is here!";
    std::string strCustomGlobalErrorMsg = "My custom global error is here!";

    // disable the console logger to prevent it from logging into our stdout
    ASSERT_TRUE(ERR_NOERROR == m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentConsoleLogPath_bEnable, false));

    // since everything is deactivated by default, no file should have been
    // created...
    ASSERT_FALSE(a_util::filesystem::exists(strTestLogFile));

    // standard association succeeds since the strategy is disabled (no matter any misconfiguration)
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetIncidentHandler()->AssociateStrategy(nCustomIncident, ES_LogFile, SA_REPLACE));

    // Try enabling the strategy (whilst no valid path for log file is configured)
    ASSERT_TRUE(ERR_INVALID_ARG == m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_bEnable, true));
    ASSERT_FALSE(a_util::filesystem::exists(strTestLogFile));

    // using non-existing path
    a_util::filesystem::Path strNoExtPath("./a/path/down/to/nothing");
    ASSERT_TRUE(ERR_OPEN_FAILED == m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_strPath,
        strNoExtPath.toString().c_str()));

    // configure valid path
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_strPath, strTestLogFile.toString().c_str()));

    // enabling the strategy (whilst it is associated)
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_bEnable, true));

    // Association should now succeed and configure the strategy
    ASSERT_TRUE(a_util::filesystem::exists(strTestLogFile));
    ASSERT_TRUE(oLogFileTester.Open(strTestLogFile, false));

    // File log on invoked incident
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester.ParseLogFile());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(m_pTestModule->GetName()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine("Info"));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(a_util::system::getHostname().c_str()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strCustomInfoMsg.c_str()));

    // "online switching" of target files
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_strPath,
        strTestLogFile2.toString().c_str()));
    ASSERT_TRUE(a_util::filesystem::exists(strTestLogFile2));
    ASSERT_TRUE(oLogFileTester2.Open(strTestLogFile2, false));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester2.ParseLogFile());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(m_pTestModule->GetName()));
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(a_util::system::getHostname().c_str()));
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strCustomInfoMsg.c_str()));

    // "online disabling" of file logging
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_bEnable, false));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeWarning(nCustomIncident, strCustomWarningMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester2.ParseLogFile());
    ASSERT_FALSE(oLogFileTester2.WasLoggedToFilePartialLastLine(strCustomWarningMsg.c_str()));

    // deactivating the strategy must retain the log files
    ASSERT_TRUE(a_util::filesystem::exists(strTestLogFile2));
    ASSERT_TRUE(a_util::filesystem::exists(strTestLogFile));

    // testing the append at restart
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_bEnable, true));
    ASSERT_TRUE(0 < oLogFileTester2.GetSize());

    // Testing catch-all functionality
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_bEnableCatchAll, false));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeError(nCustomIncident + 5, strCustomErrorMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester2.ParseLogFile());
    ASSERT_FALSE(oLogFileTester2.WasLoggedToFilePartialLastLine(strCustomErrorMsg.c_str()));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_bEnableCatchAll, true));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeError(nCustomIncident + 5, strCustomErrorMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester2.ParseLogFile());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strCustomErrorMsg.c_str()));

    // testing the overwrite function.
    ASSERT_TRUE(0 < oLogFileTester2.GetSize());
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_bEnableCatchAll, false));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_bOverwriteExisting, true));
    ASSERT_EQ(0 , oLogFileTester2.GetSize());

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester2.ParseLogFile());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(m_pTestModule->GetName()));
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(a_util::system::getHostname().c_str()));
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strCustomInfoMsg.c_str()));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeWarning(nCustomIncident, strCustomWarningMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester2.ParseLogFile());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(m_pTestModule->GetName()));
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(a_util::system::getHostname().c_str()));
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strCustomWarningMsg.c_str()));

    // testing the optional CSV file format.
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_bEnableCSV, true));
    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->GetPropertyTree()->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_bEnableCatchAll, true));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester2.ParseLogFile());
    std::string strStringToFind = a_util::strings::format("%s,", m_pTestModule->GetName());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine("Info,"));
    strStringToFind = a_util::strings::format("%s,", a_util::system::getHostname().c_str());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
    strStringToFind = a_util::strings::format(",%s", strCustomInfoMsg.c_str());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeWarning(nCustomIncident + 15, strCustomWarningMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester2.ParseLogFile());
    strStringToFind = a_util::strings::format("%s,", m_pTestModule->GetName());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine("Warning,"));
    strStringToFind = a_util::strings::format("%s,", a_util::system::getHostname().c_str());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
    strStringToFind = a_util::strings::format(",%s", strCustomWarningMsg.c_str());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeError(nCustomIncident + 25, strCustomErrorMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester2.ParseLogFile());
    strStringToFind = a_util::strings::format("%s,", m_pTestModule->GetName());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine("Critical,"));
    strStringToFind = a_util::strings::format("%s,", a_util::system::getHostname().c_str());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
    strStringToFind = a_util::strings::format(",%s", strCustomErrorMsg.c_str());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));

    ASSERT_EQ(a_util::result::SUCCESS, m_pTestModule->InvokeGlobalError(nCustomIncident + 35, strCustomErrorMsg.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester2.ParseLogFile());
    strStringToFind = a_util::strings::format("%s,", m_pTestModule->GetName());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine("Critical,"));
    strStringToFind = a_util::strings::format("%s,", a_util::system::getHostname().c_str());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
    strStringToFind = a_util::strings::format(",%s", strCustomErrorMsg.c_str());
    ASSERT_TRUE(oLogFileTester2.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));

    // deleting leftover files...
    oLogFileTester.Close();
    oLogFileTester2.Close();
    a_util::filesystem::remove(strTestLogFile);
    a_util::filesystem::remove(strTestLogFile2);
}
