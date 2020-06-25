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
* Test Case:   TestSetRemotePropertyLimits_MultipleBigStrings
* Test ID:     1.8
* Test Title:  Test Transmission of big property trees
* Description: This testcase checks the current limits for sending 
*              multiple property values (property trees).
* 
* Strategy:    Two FEP elements Hans and Sepp are created. Sepp creates a property tree with
*              a lot of different keys and values. After setting Hans retrieves (get) the complete
*              tree and compares all values with the expected ones.
*              
* Passed If:   no errors occur
*              
* Ticket:      #34833
* Requirement: FEPSDK-1602
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
 * @req_id "FEPSDK-1602"
 */
TEST(cTesterPropertyTreeTransmissionAdapter, TestSetRemotePropertyLimits_MultipleBigStrings)
{
    // Test configuration

    // Size of the property value 
    // Together with the key the total size is 1024 bytes
    const size_t szSizeOfPropertyContent = 1024; 
    const size_t szSizeOfPropertyKey = 32; 
    // Additional spaces needed to serialize a value
    const size_t szExtraSpaceForEachProperty = 32; 
    const size_t szSizeOfPropertyValue = 
        szSizeOfPropertyContent - szSizeOfPropertyKey - szExtraSpaceForEachProperty;

    // Reserved size for json serialization, header, element name, etc.
    // 1024 bytes should be enough for most cases
    const size_t szDDSAndJSONReservedSize= 1024; 

    // Calculate maximum transmission size to test
    // The maximum possible transmission size for dds is defined in 
    // file: "src/transmission_adapter/dds/dds_message_type.h"
    // static const DDS_Long MESSAGE_STRING_LENGTH = 2097152
    // Calculated size is this maximum size minus above reserved space.
    const size_t szMaxDDSTransmissionSize = 2097152 - szDDSAndJSONReservedSize;

    // Calculate maximum number of properties
    const size_t szNumberOfSubproperties = szMaxDDSTransmissionSize / szSizeOfPropertyContent;

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

    const std::string propValueRoot= "RootValue";
    ASSERT_EQ(a_util::result::SUCCESS, oModSepp.GetPropertyTree()->SetPropertyValue("SeppsTree",
                                                                    propValueRoot.c_str()));

    a_util::system::sleepMilliseconds(1000); /* 1.0 s */

    fep::Result nResult = ERR_NOERROR;
    for (size_t i= 0; i< szNumberOfSubproperties; ++i)
    {
        std::string strSubKeyString = a_util::strings::format("Top%03d.Sub%02d.SubSub%02d.Last%02d",
                                                 i/1000, (i/100)%10, (i/10)%10, i%10); // Nested Case

        std::string strKeyString= std::string("SeppsTree.") + strSubKeyString;

        std::string strValueString = make_big_string(szSizeOfPropertyValue);
        nResult|= oModSepp.GetPropertyTree()->SetPropertyValue(strKeyString.c_str(),
                                                               strValueString.c_str());
    }
    ASSERT_EQ(a_util::result::SUCCESS, nResult);

    IProperty* pPropertyRes;
    // GetRemoteProperty timeout is defined using ms !!!
    ASSERT_EQ(a_util::result::SUCCESS, oModHans.GetPropertyTree()->GetRemoteProperty(strModNameSepp.c_str(),
                                                                     "SeppsTree",
                                                                     &pPropertyRes, 
                                                                     30 * 1000)); /* 30.0 s */
    ASSERT_TRUE(NULL != pPropertyRes);


    for (size_t i= 0; i< szNumberOfSubproperties; ++i)
    {
        std::string strSubKeyString= a_util::strings::format("Top%03d.Sub%02d.SubSub%02d.Last%02d",
                                                  i/1000, (i/100)%10, (i/10)%10, i%10); // Nested Case

        std::string strLocalKeyString= std::string("SeppsTree.") + strSubKeyString;

           const char* resLocalStrValue;
        ASSERT_EQ(a_util::result::SUCCESS, oModSepp.GetPropertyTree()->GetPropertyValue(strLocalKeyString.c_str(), 
                                                                        resLocalStrValue));

        const char* resReceivedStrValue;
        IProperty* pSubProperty= pPropertyRes->GetSubproperty(strSubKeyString.c_str());
        ASSERT_TRUE(NULL != pSubProperty);
        ASSERT_EQ(a_util::result::SUCCESS, pSubProperty->GetValue(resReceivedStrValue));

        ASSERT_TRUE(std::string(resLocalStrValue) == resReceivedStrValue);
    }
}