/**
 * Implementation of the tester for the FEP Transmission Selection
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
* Test Case:   TestEnumToStringAndStringToEnum
* Test ID:     1.1
* Test Title:  cFEPTransmissionType helper class
* Description: Testing functionality of the cFEPTransmissionType helper class.
* Strategy:  All FEP core type enums are correctly transformed into string representation
*            and all string representations are correctly transformed into enum.
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1510
*/

#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include <fep_test_common.h>

/**
 * @req_id "FEPSDK-1510"
 */
TEST(cTesterTransmissionSelection, TestEnumToStringAndStringToEnum)
{
    //testing enum to string

    std::string strEnumString;
    eFEPTransmissionType eFEPTransmissionType;

    // Enum to String 
    strEnumString = "RTI_DDS";
    ASSERT_TRUE(strEnumString == fep::cFEPTransmissionType::ToString(TT_RTI_DDS));
#ifdef WITH_ZYRE
    strEnumString = "ZMQ";
    ASSERT_TRUE(strEnumString == fep::cFEPTransmissionType::ToString(TT_ZMQ));
#endif

    // String to enum. Part 1: Test correct strings
    strEnumString = "RTI_DDS";
    ASSERT_EQ(a_util::result::SUCCESS, fep::cFEPTransmissionType::FromString(strEnumString.c_str(), eFEPTransmissionType));
    ASSERT_TRUE(eFEPTransmissionType == TT_RTI_DDS);

#ifdef WITH_ZYRE
    strEnumString = "ZMQ";
    ASSERT_EQ(a_util::result::SUCCESS, fep::cFEPTransmissionType::FromString(strEnumString.c_str(), eFEPTransmissionType));
    ASSERT_TRUE(eFEPTransmissionType == TT_ZMQ);
#endif

    // String to enum. Part 2: Test DEFAULT parameter
    strEnumString = "DEFAULT";
    ASSERT_EQ(a_util::result::SUCCESS, fep::cFEPTransmissionType::FromString(strEnumString.c_str(), eFEPTransmissionType));
    ASSERT_TRUE(eFEPTransmissionType == TT_RTI_DDS);

    // String to enum. Part 3: Test INVALID
    ASSERT_NE(a_util::result::SUCCESS, fep::cFEPTransmissionType::FromString(NULL, eFEPTransmissionType));
    strEnumString = "NoSuchTransmissionAdapter";
   ASSERT_NE(a_util::result::SUCCESS, fep::cFEPTransmissionType::FromString(strEnumString.c_str(), eFEPTransmissionType));
}
