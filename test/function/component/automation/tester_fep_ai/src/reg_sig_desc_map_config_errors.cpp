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
* Test Case:   TestRegSigDescMapConfigErrors
* Test ID:     1.20
* Test Title:  Testing Error code RegisterSignalDescription/MappingConfiguration
* Description: Tests calls to RegisterSignalDescritpion/RegisterMappingConfiguration with invalid arguments
* Strategy:    Call RegisterSignalDescription and RegisterMappingConfiguration with empty element
*              name, wildcard in element name and with a negative output
*              
* Passed If:   ERR_INVALID_ARG is returned
* Ticket:      -
* Requirement: FEPSDK-1737
*/
#include <gtest/gtest.h>
#include <fep_test_common.h>
#include "helper_functions.h"
#include "mapping/fep_mapping.h"

/**
 * @req_id "FEPSDK-1737"
 */
TEST(cTesterFepAutomation, TestRegSigDescMapConfigErrors)
{
    AutomationInterface oAI;
    //Testing RegisterSignalDescription
    //Creating Modules
    cTestBaseModule oModuleA;
    cTestBaseModule oModuleB;
    ASSERT_EQ(a_util::result::SUCCESS, oModuleA.Create(cModuleOptions("ModuleA")));
    ASSERT_EQ(a_util::result::SUCCESS, oModuleB.Create(cModuleOptions("ModuleB")));

    std::string strName_In1 = "TestRequestSignalNames_In1";
    std::string strName_In1_type= "tTestRequestSignalName_In1";

    //RegisterSignalDescription with invalid element name
    ASSERT_EQ(ERR_INVALID_ARG, oAI.RegisterSignalDescription(RETURN_MEDIA_DESC(strName_In1_type.c_str(), "ui8Value", "tUInt8")
                                                                                             ,"",REM_PROP_TIMEOUT, ISignalRegistry::DF_REPLACE));
    ASSERT_EQ(ERR_INVALID_ARG, oAI.RegisterSignalDescription(RETURN_MEDIA_DESC(strName_In1_type.c_str(), "ui8Value", "tUInt8")
                                                                                             ,"?ABC",REM_PROP_TIMEOUT, ISignalRegistry::DF_REPLACE));
    ASSERT_EQ(ERR_INVALID_ARG, oAI.RegisterSignalDescription(RETURN_MEDIA_DESC(strName_In1_type.c_str(), "ui8Value", "tUInt8")
                                                                                             ,"*ABC",REM_PROP_TIMEOUT, ISignalRegistry::DF_REPLACE));
    ASSERT_EQ(ERR_INVALID_ARG, oAI.RegisterSignalDescription(RETURN_MEDIA_DESC(strName_In1_type.c_str(), "ui8Value", "tUInt8")
                                                                                             ,"ABC?",REM_PROP_TIMEOUT, ISignalRegistry::DF_REPLACE));
    ASSERT_EQ(ERR_INVALID_ARG, oAI.RegisterSignalDescription(RETURN_MEDIA_DESC(strName_In1_type.c_str(), "ui8Value", "tUInt8")
                                                                                             ,"ABC*",REM_PROP_TIMEOUT, ISignalRegistry::DF_REPLACE));
    //RegisterSignalDescription with invalid timeout
    ASSERT_EQ(a_util::result::SUCCESS, oModuleB.GetStateMachine()->StartupDoneEvent());
    ASSERT_EQ(ERR_INVALID_ARG, oAI.RegisterSignalDescription(RETURN_MEDIA_DESC(strName_In1_type.c_str(), "ui8Value", "tUInt8")
                                                                                             ,oModuleB.GetName(),-100, ISignalRegistry::DF_REPLACE));

    //Testing RegisterMappingConfiguration

    //Register signal description
    ASSERT_EQ(a_util::result::SUCCESS, oModuleB.GetSignalRegistry()->
        RegisterSignalDescription("./files/test.description",
        fep::ISignalRegistry::DF_REPLACE | fep::ISignalRegistry::DF_DESCRIPTION_FILE));

    fep::ISignalMapping* pMapping = oModuleB.GetSignalMapping();
    fep::cSignalMapping* pMappingImpl = dynamic_cast<fep::cSignalMapping*>(pMapping);
    ASSERT_TRUE(pMappingImpl);

    // test file based call
    //test that signal is not already mapped
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    //Non-existing element name
    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration("./files/base.map",
        "NotExisting",REM_PROP_TIMEOUT, 
        fep::ISignalMapping::MF_REPLACE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    //empty element-name
    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration("./files/base.map",
        "",REM_PROP_TIMEOUT, 
        fep::ISignalMapping::MF_REPLACE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    //various element names with wildcards
    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration("./files/base.map",
        "ABC?",REM_PROP_TIMEOUT, 
        fep::ISignalMapping::MF_REPLACE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration("./files/base.map",
        "ABC*",REM_PROP_TIMEOUT, 
        fep::ISignalMapping::MF_REPLACE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration("./files/base.map",
        "?ABC",REM_PROP_TIMEOUT, 
        fep::ISignalMapping::MF_REPLACE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration("./files/base.map",
        "*ABC",REM_PROP_TIMEOUT, 
        fep::ISignalMapping::MF_REPLACE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration("./files/base.map",
        "ModuleB*",REM_PROP_TIMEOUT, 
        fep::ISignalMapping::MF_REPLACE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration("./files/base.map",
        "Mod?leB",REM_PROP_TIMEOUT, 
        fep::ISignalMapping::MF_REPLACE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration("./files/base.map",
        "Mod?le*",REM_PROP_TIMEOUT, 
        fep::ISignalMapping::MF_REPLACE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    //with improper timeout
    ASSERT_TRUE(ERR_INVALID_ARG == oAI.RegisterMappingConfiguration("./files/base.map",
        oModuleB.GetName(),-100, 
        fep::ISignalMapping::MF_REPLACE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    ASSERT_EQ(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration("./files/base.map",
        oModuleB.GetName(),REM_PROP_TIMEOUT, 
        fep::ISignalMapping::MF_REPLACE | fep::ISignalMapping::MF_MAPPING_FILE));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    //Clear Mapping to not interfere with string base call test
    ASSERT_EQ(a_util::result::SUCCESS, 
        oAI.ClearMappingConfiguration(oModuleB.GetName(),REM_PROP_TIMEOUT));

    //Test string based call
    //Read-in mapping file as string
    std::string strBase;
    ASSERT_EQ(a_util::filesystem::readTextFile("./files/base.map", strBase) , a_util::filesystem::OK);

    //test that signal is not already mapped
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));


    //Non-existing element name
    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration(strBase.c_str(),
        "NonExisting", REM_PROP_TIMEOUT, fep::ISignalMapping::MF_REPLACE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    //empty element-name
    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration(strBase.c_str(),
        "", REM_PROP_TIMEOUT, fep::ISignalMapping::MF_REPLACE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    //various element names with wildcards
    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration(strBase.c_str(),
        "ABC?", REM_PROP_TIMEOUT, fep::ISignalMapping::MF_REPLACE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration(strBase.c_str(),
        "ABC*", REM_PROP_TIMEOUT, fep::ISignalMapping::MF_REPLACE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration(strBase.c_str(),
        "?ABC", REM_PROP_TIMEOUT, fep::ISignalMapping::MF_REPLACE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration(strBase.c_str(),
        "*ABC", REM_PROP_TIMEOUT, fep::ISignalMapping::MF_REPLACE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration(strBase.c_str(),
        "ModuleB*", REM_PROP_TIMEOUT, fep::ISignalMapping::MF_REPLACE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration(strBase.c_str(),
        "Mod?leB", REM_PROP_TIMEOUT, fep::ISignalMapping::MF_REPLACE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    ASSERT_NE(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration(strBase.c_str(),
        "Mod?le*", REM_PROP_TIMEOUT, fep::ISignalMapping::MF_REPLACE));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    //with improper timeout
    ASSERT_TRUE(ERR_INVALID_ARG == oAI.RegisterMappingConfiguration(strBase.c_str(),
       oModuleB.GetName(),-100));
    ASSERT_FALSE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));

    //RegisterMappingConfiguration with correct element name and propper timeout
    ASSERT_EQ(a_util::result::SUCCESS, oAI.RegisterMappingConfiguration(strBase.c_str(),
        oModuleB.GetName(), REM_PROP_TIMEOUT, fep::ISignalMapping::MF_REPLACE));
    ASSERT_TRUE(pMappingImpl->IsSignalMappable("Signal", "tFEP_Driver_DriverCtrl"));
}