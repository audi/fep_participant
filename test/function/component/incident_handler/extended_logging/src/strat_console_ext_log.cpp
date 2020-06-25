/**
* Implementation of the tester for the extended logging
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
* Test Case:   TestStratConsoleExtLog
* Test ID:     5.2
* Test Title:  Test ConsoleLogStrat ext log
* Description: Test log console strat with extended logging
* Strategy:   Console log startegy is used and an incident is invoked with strOrigin, nLine and strFile arguments
*              
* Passed If:   no errors occur
*              
* Ticket:      
* Requirement: FEPSDK-1706 FEPSDK-1707 FEPSDK-1708 FEPSDK-1710
*/


#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include <fep_test_common.h>
#include <fep_ih_test_common.h>

using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

#define FEP_TEST_MODULE_NAME "MyBadlyCodedModule"
#define FEP_LOG_WAIT_TIMEOUT static_cast<timestamp_t>(3e6)

/**
 * @req_id "FEPSDK-1706 FEPSDK-1707 FEPSDK-1708 FEPSDK-1710"
 */
TEST(cTesterExtendedLogging, TestStratConsoleExtLog)
{
    std::string strBadlyCodedModuleName = "MyBadlyCodedModule";
    cTestBaseModule oModule;
    ASSERT_TRUE(NULL == oModule.GetIncidentHandler());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions( strBadlyCodedModuleName.c_str())));
    ASSERT_TRUE(NULL != oModule.GetIncidentHandler());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetIncidentHandler()->AssociateStrategy(
                           33, ES_LogConsole, SA_REPLACE));
    int16_t nCustomIncident = 15;
    const char *strDescription = "InfoDesc";
    const char *strOrigin = "TesterExtendedLogging";
    int nLine = 42;
    const char *strFile = "/foo/bar.cpp";

    //There is no propper way to test console output, so just the invocation of
    //an incident is tested.
    ASSERT_EQ(a_util::result::SUCCESS, oModule.InvokeInfo(nCustomIncident, strDescription,strOrigin,
                                          nLine, strFile));
}