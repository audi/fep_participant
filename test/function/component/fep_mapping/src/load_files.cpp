/**
 * Implementation of the tester for Mapping
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
* Test Case:   TestLoadFilesFEPMapping
* Test ID:     1.11
* Test Title:  Test loading mapping configurations with FEP Mapping
* Description: Test loading mapping configurations with FEP Mapping
*               - from filepath
*               - from configuration string.
*              Test deleting the mapping configuration
* Strategy:    
* Passed If:   no errors occur
* Ticket:      #32177
* Requirement: FEPSDK-1608 FEPSDK-1609 FEPSDK-1610
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include "fep_test_common.h"

#include "a_util/concurrency.h"
#include "mapping/fep_mapping.h"
#include "signal_registry/fep_signal_registry.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include <ddl.h>

using namespace mapping::oo;
using namespace mapping::rt;
using namespace ddl;

/**
 * @req_id "FEPSDK-1608 FEPSDK-1609 FEPSDK-1610"
 */
TEST(cTesterFEPMapping, TestLoadFilesFEPMapping)
{
    cTestBaseModule oTestModule;
    ASSERT_EQ(a_util::result::SUCCESS, oTestModule.Create(cModuleOptions( "MapModule")));
    ASSERT_EQ(a_util::result::SUCCESS, oTestModule.GetSignalRegistry()->
        RegisterSignalDescription("./files/test.description", fep::ISignalRegistry::DF_DESCRIPTION_FILE));

    fep::ISignalMapping* pMapping = oTestModule.GetSignalMapping();
    ASSERT_EQ(pMapping->RegisterMappingConfiguration(NULL), ERR_INVALID_ARG);
    ASSERT_EQ(pMapping->RegisterMappingConfiguration(""), ERR_INVALID_ARG);

    ASSERT_EQ(pMapping->RegisterMappingConfiguration("./files/base.map",
        fep::ISignalMapping::MF_MERGE | fep::ISignalMapping::MF_REPLACE), ERR_NOT_SUPPORTED);
    ASSERT_EQ(pMapping->RegisterMappingConfiguration("invalid_file",
        fep::ISignalMapping::MF_MERGE | fep::ISignalMapping::MF_MAPPING_FILE), ERR_INVALID_FILE);

    fep::cSignalMapping* pMappingImpl = dynamic_cast<fep::cSignalMapping*>(pMapping);
    ASSERT_TRUE(pMappingImpl);

    // test file based calls
    ASSERT_TRUE(!pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_EQ(a_util::result::SUCCESS, pMapping->RegisterMappingConfiguration("./files/base.map",
        fep::ISignalMapping::MF_REPLACE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_NE(a_util::result::SUCCESS, pMapping->RegisterMappingConfiguration("./files/nok_merge0.map",
        fep::ISignalMapping::MF_MERGE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_EQ(a_util::result::SUCCESS, pMapping->RegisterMappingConfiguration("./files/merge.map",
        fep::ISignalMapping::MF_MERGE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal2", "tFEP_Driver_LateralControl"));
    ASSERT_EQ(a_util::result::SUCCESS, pMapping->ClearMappingConfiguration());
    ASSERT_TRUE(!pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_TRUE(!pMappingImpl->IsSignalMappable("Signal2", "tFEP_Driver_LateralControl"));
    ASSERT_TRUE(NULL != oTestModule.GetPropertyTree());

    // test string based calls
    std::string strBase, strMergeErr, strMergeOk;
    ASSERT_EQ(a_util::filesystem::OK, a_util::filesystem::readTextFile("./files/base.map", strBase));
    ASSERT_EQ(a_util::filesystem::OK, a_util::filesystem::readTextFile("./files/nok_merge0.map", strMergeErr));
    ASSERT_EQ(a_util::filesystem::OK, a_util::filesystem::readTextFile("./files/merge.map", strMergeOk));

    ASSERT_EQ(a_util::result::SUCCESS, pMapping->RegisterMappingConfiguration(strBase.c_str(),
        fep::ISignalMapping::MF_REPLACE));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_NE(a_util::result::SUCCESS, pMapping->RegisterMappingConfiguration(strMergeErr.c_str(),
        fep::ISignalMapping::MF_MERGE));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_EQ(a_util::result::SUCCESS, pMapping->RegisterMappingConfiguration(strMergeOk.c_str(),
        fep::ISignalMapping::MF_MERGE));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal2", "tFEP_Driver_LateralControl"));
    ASSERT_EQ(a_util::result::SUCCESS, pMapping->ClearMappingConfiguration());
    ASSERT_TRUE(!pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
    ASSERT_TRUE(!pMappingImpl->IsSignalMappable("Signal2", "tFEP_Driver_LateralControl"));
}