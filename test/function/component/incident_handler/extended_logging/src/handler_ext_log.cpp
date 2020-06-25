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
* Test Case:   TestIncidentHandlerExtLog
* Test ID:     5.1
* Test Title:  Test incident invokation
* Description: Test incident Invokation with extended logging
* Strategy:   An incident is invoked with strOrigin, nLine and strFile arguments
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

//############################### Custom Strategy ########################################

#define SOME_STRATEGY_PROPERTY "sMyStrategy.bWhatever"


//############################### Test Setup goes here ###################################

void TestIntegratedAssocDisassoc(cTestBaseModule& oModule, const tIncidentStrategy eStrategy)
{
    ASSERT_RESULT_OR_THROW(oModule.GetIncidentHandler()->AssociateStrategy(10, eStrategy, SA_REPLACE));
    ASSERT_RESULT_OR_THROW(oModule.GetIncidentHandler()->AssociateStrategy(10, eStrategy, SA_REPLACE));
    ASSERT_OR_THROW(ERR_RESOURCE_IN_USE == oModule.GetIncidentHandler()->AssociateStrategy(10, eStrategy, SA_APPEND));
    ASSERT_RESULT_OR_THROW(oModule.GetIncidentHandler()->DisassociateStrategy(10, eStrategy));
    ASSERT_RESULT_OR_THROW(oModule.GetIncidentHandler()->AssociateStrategy(10, eStrategy, SA_REPLACE));
    ASSERT_RESULT_OR_THROW(oModule.GetIncidentHandler()->AssociateStrategy(11, eStrategy, SA_REPLACE));
    ASSERT_RESULT_OR_THROW(oModule.GetIncidentHandler()->DisassociateStrategy(10, eStrategy));
    ASSERT_RESULT_OR_THROW(oModule.GetIncidentHandler()->DisassociateStrategy(11, eStrategy));
    ASSERT_OR_THROW(ERR_NOT_FOUND == oModule.GetIncidentHandler()->DisassociateStrategy(11, eStrategy));
}

/**
 * @req_id "FEPSDK-1706 FEPSDK-1707 FEPSDK-1708 FEPSDK-1710"
 */
TEST(cTesterExtendedLogging, TestIncidentHandlerExtLog)
{
    std::string strBadlyCodedModuleName = "MyBadlyCodedModule";
    cTestBaseModule oModule;
    ASSERT_TRUE(NULL == oModule.GetIncidentHandler());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions( strBadlyCodedModuleName.c_str())));
    ASSERT_TRUE(NULL != oModule.GetIncidentHandler());
    int16_t nCustomIncident = 15;
    const char *strDescription = "InfoDesc";
    const char *strOrigin = "TesterExtendedLogging";
    int nLine = 42;
    const char *strFile = "/foo/bar.cpp";
    ASSERT_EQ(a_util::result::SUCCESS, oModule.InvokeInfo(nCustomIncident, strDescription,strOrigin,
                                          nLine, strFile));
}