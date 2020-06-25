/**
* Implementation of the tester for the integration of FEP PropertyTree with FEP Transmission Adapter
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
* Test Case:   TestSetRemotePropertyLimits_SingleBigString
* Test ID:     1.7
* Test Title:  Test Transmission of a single big property
* Description: This testcase checks the current limits for sending
*              single property values.
* 
* Strategy:    Two FEP elements Hans and Sepp are created. Hans sets a remote property value 
*              on element Sepp. After setting Hans retrieves (gets) the value and compares it
*              with the expected value. 
*              
* Passed If:   no errors occur
*              
* Ticket:      #34833
* Requirement: FEPSDK-1603
*/
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::component_config;

#include "messages/fep_command_set_property.h"
#include "test_module.h"
#include "helper_functions.h"

/**
 * @req_id "FEPSDK-1603"
 */
TEST(cTesterPropertyTreeTransmissionAdapter, TestSetRemotePropertyLimits_SingleBigString)
{

    // Test configuration

    // Reserved size for json serialization, header, element name, etc.
    // 1024 bytes should be enough for most cases
    const size_t szDDSAndJSONReservedSize = 1024; 

    // Calculate maximum transmission size to test
    // The maximum possible transmission size for dds is defined in 
    // file: "src/transmission_adapter/dds/dds_message_type.h"
    // static const DDS_Long MESSAGE_STRING_LENGTH = 2097152
    // Calculated size is this maximum size minus above reserved space.
    const size_t szMaxDDSTransmissionSize = 2097152 - szDDSAndJSONReservedSize;

    // Test itself
    std::string strTimePostfix = a_util::datetime::getCurrentSystemTime().format("%H%M%S");
    std::string strModNameHans = 
        a_util::strings::format("TestSetRemotePropertyValues_Hans_%d", 
        strTimePostfix.c_str());
    std::string strModNameSepp = 
        a_util::strings::format("TestSetRemotePropertyValues_Sepp_%d", 
        strTimePostfix.c_str());

    /* Create 2 modules and bring them up to state RUNNING*/
    cTestModule oModHans, oModSepp;
    ASSERT_EQ(a_util::result::SUCCESS, oModHans.Create(
        strModNameHans.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oModSepp.Create(
        strModNameSepp.c_str()));

    ASSERT_EQ(a_util::result::SUCCESS, oModHans.WaitForRunning(200000));
    ASSERT_EQ(a_util::result::SUCCESS, oModSepp.WaitForRunning(200000));
    a_util::system::sleepMilliseconds(1000); /* 1.0 s */

    const std::string strPropertyName("TestSetRemotePropertyValue.bTestValue");

    size_t szTest= szMaxDDSTransmissionSize-strPropertyName.size();
    {
        std::string strBigString= make_big_string(szTest);

        ASSERT_EQ(a_util::result::SUCCESS, oModHans.GetPropertyTree()->SetRemotePropertyValue(
                                                        strModNameSepp.c_str(),
                                                        strPropertyName.c_str(),
                                                        strBigString.c_str()));

        a_util::system::sleepMilliseconds(20 * 1000); /* 20.0 s */

        IProperty* pPropertyRes;
        const char* resStrValue;

        pPropertyRes = oModSepp.GetPropertyTree()->GetProperty(strPropertyName.c_str());
        ASSERT_TRUE(NULL != pPropertyRes);
        ASSERT_EQ(a_util::result::SUCCESS, pPropertyRes->GetValue(resStrValue));
        ASSERT_EQ(strBigString , resStrValue);

        // GetRemoteProperty timeout is defined using ms !!!
        ASSERT_EQ(a_util::result::SUCCESS, oModHans.GetPropertyTree()->GetRemoteProperty(
                                                        strModNameSepp.c_str(),
                                                        strPropertyName.c_str(),
                                                        &pPropertyRes, 20 * 1000));
        ASSERT_EQ(a_util::result::SUCCESS, pPropertyRes->GetValue(resStrValue));
        ASSERT_EQ(strBigString , resStrValue);
    }
}