/**
* Implementation of the tester for the FEP Automation Interface
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

/*
* Test Case:   TestSignalRegistryRemote
* Test ID:     1.15
* Test Title:  Tests aggregates system registry methods (description handling)
* Description: Tests the registration of signal descriptions via the automation interface
* Strategy:    One module is created and a description file is registered. Additionally an incompatible
*              and a compatible description file are attempted to be merged. After each registration
*              the according signals are being resolved. Registration is done remotely. 
*              
* Passed If:   Conflicting merge attempt fails. Other registrations succeed and the registered signal
*              description types can be resolved.
* Ticket:      -
* Requirement: FEPSDK-1503 FEPSDK-1504
*/
#include <gtest/gtest.h>

#include <fep_test_common.h>
#include "helper_functions.h"
#include "a_util/filesystem.h"

/**
 * @req_id "FEPSDK-1503 FEPSDK-1504"
 */
TEST(cTesterFepAutomation, TestSignalRegistryRemote)
{
    AutomationInterface oAI;
    const char* strDesc = NULL;

    cTestBaseModule oModule;
    ASSERT_EQ(fep::ERR_NOERROR, oModule.Create(cModuleOptions("TestMod")));
    oModule.GetStateMachine()->StartupDoneEvent();
    oModule.WaitForState(FS_IDLE);

    fep::ISignalRegistry* pSR = oModule.GetSignalRegistry();

    std::string strBase, strConflict, strOk;
    ASSERT_EQ(a_util::filesystem::OK , a_util::filesystem::readTextFile("./files/base.description", strBase));
    ASSERT_EQ(a_util::filesystem::OK , a_util::filesystem::readTextFile("./files/merge_conflict.description", strConflict));
    ASSERT_EQ(a_util::filesystem::OK , a_util::filesystem::readTextFile("./files/merge_ok.description", strOk));
    
    // register base description (contains tFEP_VU_Coord) using string variant
    ASSERT_EQ(fep::ERR_NOERROR, oAI.RegisterSignalDescription(strBase.c_str(), 
        oModule.GetName(), REM_PROP_TIMEOUT*2, ISignalRegistry::DF_REPLACE));

    // check whether the type is found
    ASSERT_EQ(fep::ERR_NOERROR, pSR->ResolveSignalType("tFEP_VU_Coord", strDesc));
    ASSERT_NE(std::string(strDesc).find("tFEP_VU_Coord") , std::string::npos);

    // try to merge conflicting type
    ASSERT_TRUE(oAI.RegisterSignalDescription(strConflict.c_str(),
        oModule.GetName(), REM_PROP_TIMEOUT*2, ISignalRegistry::DF_MERGE) == ERR_INVALID_TYPE);

    // check if the old type is still there
    ASSERT_EQ(fep::ERR_NOERROR, pSR->ResolveSignalType("tFEP_VU_Coord", strDesc));
    ASSERT_NE(std::string(strDesc).find("tFEP_VU_Coord") , std::string::npos);

    // merge type using file variant
    ASSERT_EQ(fep::ERR_NOERROR, oAI.RegisterSignalDescription("./files/merge_ok.description",
        oModule.GetName(), REM_PROP_TIMEOUT*2, ISignalRegistry::DF_MERGE | ISignalRegistry::DF_DESCRIPTION_FILE));

    // check if the old type is still there in addition to the new one
    ASSERT_EQ(fep::ERR_NOERROR, pSR->ResolveSignalType("tFEP_VU_Coord", strDesc));
    ASSERT_NE(std::string(strDesc).find("tFEP_VU_Coord") , std::string::npos);

    ASSERT_EQ(fep::ERR_NOERROR, pSR->ResolveSignalType("tFEP_VU_LightSource", strDesc));
    ASSERT_NE(std::string(strDesc).find("tFEP_VU_LightSource") , std::string::npos);

    // clear descriptions
    ASSERT_EQ(fep::ERR_NOERROR, oAI.ClearSignalDescriptions(oModule.GetName(), REM_PROP_TIMEOUT*2));

    // check if clearing worked
    ASSERT_EQ(pSR->ResolveSignalType("tFEP_VU_Coord", strDesc) , ERR_NOT_FOUND);
    ASSERT_EQ(pSR->ResolveSignalType("tFEP_VU_LightSource", strDesc) , ERR_NOT_FOUND);
}