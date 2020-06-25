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
* Test Case:   TestStratHistExtLog
* Test ID:     5.4
* Test Title:  Test TestStratHistExtLog ext log
* Description: Test history log strat with extended logging
* Strategy:   History log startegy is used and an incident is invoked with strOrigin, nLine and strFile arguments
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

//Helper for TestStratHistExtLog
template<fep::tSeverityLevel T>
void TestHistoryEntry(int32_t nSeekEntryNo, int16_t nIncident,
                       std::string& strMessage, std::string& strOriginName,
                       std::string& strOrigin,
                       const timestamp_t tmSimTime,
                       tIncidentListConstIter itHistBegin,
                       tIncidentListConstIter itHistEnd)
{
    ASSERT_OR_THROW(itHistBegin != itHistEnd);

   for (int32_t nEntry = 0;
        nEntry != nSeekEntryNo && itHistBegin != itHistEnd;
        nEntry++, itHistBegin++)
   {
       nEntry++;
   }

    ASSERT_OR_THROW(itHistBegin->eSeverity == T);
    ASSERT_OR_THROW(itHistBegin->nIncident == nIncident);
    ASSERT_OR_THROW(itHistBegin->tmSimTime == tmSimTime);
    ASSERT_OR_THROW(strMessage == itHistBegin->strMessage);
    // unable to test for equality since the names are modified internal (for the tests)
    // See cTestBaseModule for details.
    ASSERT_OR_THROW(0 >= a_util::strings::compare(strOriginName.c_str(), itHistBegin->strSource));
    ASSERT_OR_THROW(strOrigin == itHistBegin->strOrigin);
}

/**
 * @req_id "FEPSDK-1706 FEPSDK-1707 FEPSDK-1708 FEPSDK-1710"
 */
TEST(cTesterExtendedLogging, TestStratHistExtLog)
{
    int16_t nCustomIncident = 15;
    std::string strOrigin = std::string("TesterExtendedLogging");
    int nLine = 42;
    const char *strFile = "/foo/bar.cpp";
    std::string strCustomInfoMsg = "My custom info is here!";
    int32_t nSeek =0;

    tIncidentListConstIter itHistBegin;
    tIncidentListConstIter itHistEnd;

    std::string strSourceName = std::string(FEP_TEST_MODULE_NAME);

    cTestBaseModule oModule;
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions( strSourceName.c_str())));
    ASSERT_TRUE(NULL != oModule.GetIncidentHandler());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetIncidentHandler()->AssociateStrategy(
                               nCustomIncident, ES_LogHistory, SA_REPLACE));
    // enable the strategy
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentHistoryLogPath_bEnable, true));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str(),strOrigin.c_str(),nLine,strFile));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));
    //Tests History entries SimulationTime should be zero (element is not running yet)
    TestHistoryEntry<fep::SL_Info>(nSeek, nCustomIncident, strCustomInfoMsg, strSourceName,strOrigin, 0,
                                         itHistBegin, itHistEnd);
    ASSERT_EQ(a_util::result::SUCCESS, oModule.StartUpModule(true));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str(),strOrigin.c_str(),nLine,strFile));
//    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetIncidentHandler()->RetrieveIncidentHistory(itHistBegin, itHistEnd));
//    ASSERT_TRUE(itHistBegin != itHistEnd);
    //Simulation time should be nonzero positive
    ASSERT_TRUE(0 < itHistBegin->nTimeStamp);
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetIncidentHandler()->FreeIncidentHistory());
}