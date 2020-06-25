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
* Test Case:   TestStratFileExtLog
* Test ID:     5.3
* Test Title:  Test fileLogStrat ext log
* Description: Test file log strat with extended logging
* Strategy:   File log startegy is used and an incident is invoked with strOrigin, nLine and strFile arguments
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


class cTimeCaputreStrat : public fep::IIncidentStrategy
{
public:
    cTimeCaputreStrat(const int16_t nIncidentFilter) :
        m_nIncidentFilter(nIncidentFilter),
        m_tmTimeStamp(-1),
        m_mtxProtect()
    {
    }

    ~cTimeCaputreStrat()
    {
    }

    fep::Result RefreshConfiguration(
        const fep::IProperty* pStrategyProperty,
        const fep::IProperty* pAffectedProperty)
    {
        return ERR_NOERROR;
    }

    fep::Result HandleGlobalIncident(const char* strSource, const int16_t nIncident,
        const fep::tSeverityLevel eSeverity,
        const timestamp_t tmSimTime,
        const char* strDescription = NULL
    )
    {
        if (nIncident == m_nIncidentFilter)
        {
            a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> mLock(m_mtxProtect);
            m_tmTimeStamp = tmSimTime;
        }
        return ERR_NOERROR;
    }

    fep::Result HandleLocalIncident(
        fep::IModule* pElementContext, const int16_t nIncident,
        const fep::tSeverityLevel eSeverity,
        const char* strOrigin,
        int nLine,
        const char* strFile,
        const timestamp_t tmSimTime,
        const char* strDescription = NULL)
    {
        if (nIncident == m_nIncidentFilter)
        {
            a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> mLock(m_mtxProtect);
            m_tmTimeStamp = tmSimTime;
        }
        return ERR_NOERROR;
    }

    timestamp_t GetTimeStamp()
    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> mLock(m_mtxProtect);
        return m_tmTimeStamp;
    }

private:
    int16_t m_nIncidentFilter;
    timestamp_t m_tmTimeStamp;
    a_util::concurrency::fast_mutex m_mtxProtect;
};

/**
 * @req_id "FEPSDK-1706 FEPSDK-1707 FEPSDK-1708 FEPSDK-1710"
 */
TEST(cTesterExtendedLogging, TestStratFileExtLog)
{
    const char *strDesc = "InfoDesc";
    const char *strOrigin = "TesterExtendedLogging";
    int nLine = 42;
    const char *strFile = "/foo/bar.cpp";
    std::string strCustomInfoMsg = "My custom info is here!";
    int16_t nCustomIncident = 15;

    a_util::filesystem::Path strTestLogFile = "mylogfile.log";
    cLogFileTester oLogFileTester;
    cTestBaseModule oModule;
    cTimeCaputreStrat oTimeCaputre(nCustomIncident);
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions( "MyBadlyCodedModule")));
    ASSERT_TRUE(NULL != oModule.GetIncidentHandler());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetIncidentHandler()->AssociateCatchAllStrategy(&oTimeCaputre,
        "", SA_APPEND));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetIncidentHandler()->AssociateStrategy(
                               nCustomIncident, ES_LogFile, SA_APPEND));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentFileLogPath_strPath,
                           strTestLogFile.toString().c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentFileLogPath_bEnable, true));
    ASSERT_TRUE(a_util::filesystem::exists(strTestLogFile));
    ASSERT_TRUE(oLogFileTester.Open(strTestLogFile, false));


    // Testing whether the file logger works
    ASSERT_EQ(a_util::result::SUCCESS, oModule.InvokeInfo(nCustomIncident, strDesc,strOrigin,nLine,strFile));
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester.ParseLogFile());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(oModule.GetName()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine("Info"));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(a_util::system::getHostname().c_str()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strDesc));
    std::string strDateTime = a_util::strings::format(
                "%s - %s", a_util::datetime::getCurrentLocalDate().format("%d.%m.%Y").c_str(),
                a_util::datetime::getCurrentLocalTime().format("%H:%M").c_str());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strDateTime.c_str()));
#ifndef _DEBUG
    std::string strOriginFormated = a_util::strings::format("[%s]",strOrigin);
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strOriginFormated.c_str()));
#else
    std::string strOriginFormated = a_util::strings::format("[%s (%s:%d)]",
                                                strOrigin, strFile,nLine);
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strOriginFormated.c_str()));
#endif
    std::string strSimTimeFormated = std::string("ST: 0[us]");
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strSimTimeFormated.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, oModule.GetName()));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.StartUpModule(true));
    
    oModule.WaitUntilTimeIsGreaterZero();
    
    ASSERT_EQ(a_util::result::SUCCESS, oModule.InvokeInfo(nCustomIncident, strDesc,strOrigin,nLine+1,strFile));

    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester.ParseLogFile());
    int tmSimTimeRetrieved = -1;
    //ASSERT_TRUE(oLogFileTester.GetLoggedTimeStamp(tmSimTimeRetrieved));
    tmSimTimeRetrieved = static_cast<int>(oTimeCaputre.GetTimeStamp());
    ASSERT_TRUE(0 < tmSimTimeRetrieved);


    //Testing whether the file logger works in csv-mode

    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentFileLogPath_bEnableCSV, true));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->SetPropertyValue(
                           component_config::g_strIncidentFileLogPath_bEnableCatchAll, true));

    ASSERT_EQ(a_util::result::SUCCESS, oModule.InvokeInfo(nCustomIncident, strCustomInfoMsg.c_str(),strOrigin,nLine,strFile));
    a_util::system::sleepMilliseconds(10);
    ASSERT_EQ(a_util::result::SUCCESS, oLogFileTester.ParseLogFile());
    tmSimTimeRetrieved = static_cast<int>(oTimeCaputre.GetTimeStamp());
    ASSERT_TRUE(0 < tmSimTimeRetrieved);
    std::string strStringToFind = a_util::strings::format("%s,", oModule.GetName());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine("Info,"));
    strStringToFind = a_util::strings::format("%s,", a_util::system::getHostname().c_str());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
    strStringToFind = a_util::strings::format(",%s", strCustomInfoMsg.c_str());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
    strStringToFind = a_util::strings::format(
                "%s,%s", a_util::datetime::getCurrentLocalDate().format("%d.%m.%Y").c_str(),
                a_util::datetime::getCurrentLocalTime().format("%H:%M").c_str());
    ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
#ifndef _DEBUG
    strStringToFind = a_util::strings::format("%s",strOrigin);
     ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
#else
    strStringToFind = a_util::strings::format(",%s,%d,%s",strOrigin,nLine,strFile);
     ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
#endif

    strStringToFind = a_util::strings::format("%s,%d,%s",oModule.GetName(),tmSimTimeRetrieved,strOrigin);
     ASSERT_TRUE(oLogFileTester.WasLoggedToFilePartialLastLine(strStringToFind.c_str()));
     ASSERT_EQ(a_util::result::SUCCESS, oModule.GetIncidentHandler()->DisassociateCatchAllStrategy(&oTimeCaputre));
}
