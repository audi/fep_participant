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
* Test Case:   TestSignalMapping
* Test ID:     1.16
* Test Title:  Tests mapping methods
* Description: Signal Mapping via the Automation Interface is tested.
* Strategy:    Signal descriptions are loaded. Then a mapping is applied via AI locally and remotley.
*              Once using the file base and once using the string based implementaion. Replacement and merging
*              of a mapping are tested. After each mapping a check is done whether the signal is mapped.
*              This is done via the IsSignalMappable Method.
*              
* Passed If:   The signal is mappable after each mapping. Exception to this is the intentionally conflicting mapping.
*              The signals are not mappable after the call to ClearMappingConfiguration.
* Ticket:      -
* Requirement: FEPSDK-1505 FEPSDK-1506
*/
#include <gtest/gtest.h>

#include <fep_test_common.h>
#include "helper_functions.h"
#include "mapping/fep_mapping.h"

/**
 * @req_id "FEPSDK-1505 FEPSDK-1506"
 */
TEST(cTesterFepAutomation, TestSignalMapping)
{
    AutomationInterface oAI;
    cTestBaseModule oMaster, oModule;
    ASSERT_EQ(a_util::result::SUCCESS, oMaster.Create(cModuleOptions("Master")));
    ASSERT_EQ(a_util::result::SUCCESS, oMaster.GetSignalRegistry()->
        RegisterSignalDescription("./files/test.description",
        fep::ISignalRegistry::DF_REPLACE | fep::ISignalRegistry::DF_DESCRIPTION_FILE));
    ASSERT_EQ(a_util::result::SUCCESS, oMaster.StartUpModule(true));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions("TestMod")));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->
        RegisterSignalDescription("./files/test.description",
        fep::ISignalRegistry::DF_REPLACE | fep::ISignalRegistry::DF_DESCRIPTION_FILE));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.StartUpModule(true));
    
    fep::ISignalMapping* pMapping = oMaster.GetSignalMapping();
    fep::cSignalMapping* pMappingImpl = dynamic_cast<fep::cSignalMapping*>(pMapping);
    ASSERT_TRUE(pMappingImpl);

    // test string based calls
    std::string strBase, strMergeErr, strMergeOk;
    ASSERT_EQ(a_util::filesystem::OK, a_util::filesystem::readTextFile("./files/base.map", strBase));
    ASSERT_EQ(a_util::filesystem::OK, a_util::filesystem::readTextFile("./files/merge_conflict.map", strMergeErr));
    ASSERT_EQ(a_util::filesystem::OK, a_util::filesystem::readTextFile("./files/merge_ok.map", strMergeOk));

    // remote calls
    pMapping = oModule.GetSignalMapping();
    pMappingImpl = dynamic_cast<fep::cSignalMapping*>(pMapping);
    ASSERT_TRUE(pMappingImpl);

    ASSERT_EQ(oAI.RegisterMappingConfiguration(strBase, "bullshitmodule",
        REM_PROP_TIMEOUT, fep::ISignalMapping::MF_REPLACE), ERR_TIMEOUT);
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);

    // test file based calls
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration("./files/base.map", 
        oModule.GetName(), REM_PROP_TIMEOUT*5,
        fep::ISignalMapping::MF_REPLACE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration("./files/merge_conflict.map",
        oModule.GetName(), REM_PROP_TIMEOUT,
        fep::ISignalMapping::MF_MERGE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration("./files/merge_ok.map",
        oModule.GetName(), REM_PROP_TIMEOUT,
        fep::ISignalMapping::MF_MERGE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal2", "tFEP_Driver_LateralControl"));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.ClearMappingConfiguration(oModule.GetName(), 
        REM_PROP_TIMEOUT));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal2", "tFEP_Driver_LateralControl"));

    // test string based calls
    ASSERT_EQ(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration(strBase.c_str(), 
        oModule.GetName(), REM_PROP_TIMEOUT, fep::ISignalMapping::MF_REPLACE));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration(strMergeErr.c_str(),
        oModule.GetName(), REM_PROP_TIMEOUT, fep::ISignalMapping::MF_MERGE));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration(strMergeOk.c_str(),
        oModule.GetName(), REM_PROP_TIMEOUT, fep::ISignalMapping::MF_MERGE));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal2", "tFEP_Driver_LateralControl"));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.ClearMappingConfiguration(oModule.GetName(), 
        REM_PROP_TIMEOUT));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal2", "tFEP_Driver_LateralControl"));
}