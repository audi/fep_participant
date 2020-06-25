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
* Test Case:   TestSetPropertyCommand
* Test ID:     1.4
* Test Title:  Test set property command
* Description: Create a pair of modules and send each other set property commands
* 
* Strategy:    
*              
* Passed If:   the property is set properly
*              
* Ticket:      -
* Requirement: FEPSDK-1564
*/
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::component_config;

#include "messages/fep_command_set_property.h"
#include "test_module.h"

/**
 * @req_id "FEPSDK-1564 FEPSDK-1803"
 */
TEST(cTesterPropertyTreeTransmissionAdapter, TestSetPropertyCommand)
{
    cTestModule oModA, oModB;
    ASSERT_EQ(a_util::result::SUCCESS, oModA.Create("A"));
    ASSERT_EQ(a_util::result::SUCCESS, oModB.Create("B"));
    ASSERT_EQ(a_util::result::SUCCESS, oModA.WaitForRunning(200000));
    ASSERT_EQ(a_util::result::SUCCESS, oModB.WaitForRunning(200000));
    a_util::system::sleepMilliseconds(1000);

    // test the string representation of the commands first, as a sanity test
    timestamp_t tmTime = a_util::system::getCurrentMicroseconds();
    std::string const strVersion = a_util::strings::format("%d.%d", FEP_SDK_PARTICIPANT_VERSION_MAJOR, FEP_SDK_PARTICIPANT_VERSION_MINOR);
    fep::cSetPropertyCommand oCmd(false, "This.Property",
        "A", "B", tmTime, 0);
    std::string strTest = a_util::strings::format("{\n"
        "	\"Command\" : {\n"
        "		\"Type\" : \"set_property\",\n"
        "		\"Property\" : \"This.Property\",\n"
        "		\"Value\" : false\n"
        "	},\n"
        "	\"Header\" : {\n"
        // Note: In '#30719' changed format when transmitting float numbers
        //       If using libjson >= 7.6.1.1 it is required to use a number
        //       followed by a decimal point when representing a float
        "		\"LanguageVersion\" : \"%s\",\n" 
        "		\"Sender\" : \"A\",\n"
        "		\"Receiver\" : \"B\",\n"
        "		\"Timestamp\" : \"%lld\",\n"
        "		\"SimTime\" : \"0\"\n"
        "	}\n"
        "}", strVersion.c_str(), tmTime);

    ASSERT_TRUE(strTest == oCmd.ToString());

    ASSERT_NE(a_util::result::SUCCESS, oCmd.AppendValue(3.14));
    ASSERT_NE(a_util::result::SUCCESS, oCmd.AppendValue("Test"));
    ASSERT_EQ(a_util::result::SUCCESS, oCmd.AppendValue(true));

    strTest = a_util::strings::format("{\n"
        "	\"Command\" : {\n"
        "		\"Type\" : \"set_property\",\n"
        "		\"Property\" : \"This.Property\",\n"
        "		\"Value\" : [\n"
        "			false,\n"
        "			true\n"
        "		]\n"
        "	},\n"
        "	\"Header\" : {\n"
        // Note: In '#30719' changed format when transmitting float numbers
        //       If using libjson >= 7.6.1.1 it is required to use a number
        //       followed by a decimal point when representing a float
        "		\"LanguageVersion\" : \"%s\",\n" 
        "		\"Sender\" : \"A\",\n"
        "		\"Receiver\" : \"B\",\n"
        "		\"Timestamp\" : \"%lld\",\n"
        "		\"SimTime\" : \"0\"\n"
        "	}\n"
        "}", strVersion.c_str(), tmTime);

    ASSERT_TRUE(strTest == oCmd.ToString());

    // now test command interpretation
    ASSERT_EQ(a_util::result::SUCCESS, oModA.GetCmdAccess()->TransmitCommand(&oCmd));

    // wait for propagation
    a_util::system::sleepMilliseconds(2000);

    // get the received property
    IProperty * poProp = oModB.GetPropTree()
        ->GetLocalProperty("This.Property");
    ASSERT_TRUE(poProp);
    ASSERT_TRUE(poProp->IsBoolean());
    ASSERT_TRUE(poProp->IsArray());
    ASSERT_TRUE(poProp->GetArraySize() == 2);
    bool bVal;
    ASSERT_EQ(a_util::result::SUCCESS, poProp->GetValue(bVal, 0));
    ASSERT_FALSE(bVal);
    ASSERT_EQ(a_util::result::SUCCESS, poProp->GetValue(bVal, 1));
    ASSERT_TRUE(bVal);
}