/**
* Implementation of the tester for the FEP cUserSignalOptions
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

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include <fep_test_common.h>
#include "signal_registry/fep_signal_registry.h"

using namespace fep;

/*
* Test Case:   TestUserSignalOptionsCheckValidity
* Test ID:     1.5
* Test Title:  Test cUserSignalOptionsClass CheckValidity method
* Description: Test basic functionality of cUserSignalOptions::CheckValidity method
* Strategy:    Create invalid oOptions add parameters until validity check passes.
* Passed If:   no errors occur
* Ticket:      -
*/
/**
 * @req_id "FEPSDK-1750 FEPSDK-1544 FEPSDK-1699"
 */
TEST(cTesterSignalRegistry, TestUserSignalOptionsCheckValidity)
{
    cUserSignalOptions oOptions("",SD_Undefined);
    EXPECT_FALSE(oOptions.CheckValidity());
    oOptions.SetSignalDirection(SD_Output);
    EXPECT_FALSE(oOptions.CheckValidity());
    oOptions.SetSignalDirection(SD_Input);
    EXPECT_FALSE(oOptions.CheckValidity());
    oOptions.SetSignalName("SomeValidName");
    EXPECT_TRUE(oOptions.CheckValidity());
    oOptions.SetSignalDirection(SD_Output);
    EXPECT_TRUE(oOptions.CheckValidity());
    oOptions.SetSignalType("");
    EXPECT_FALSE(oOptions.CheckValidity());
}

/*
* Test Case:   TestUserSignalOptionsConstructorNULL
* Test ID:     1.6
* Test Title:  Test cUserSignalOptionsClass constructors
* Description: Test cUserSignalOptions constructor when given NULL Pointer
* Strategy:    Pass Null Pointer to Constructor
* Passed If:   No Segfault
* Ticket:      -
* Requirement: XXX
*/
/**
 * @req_id "FEPSDK-1750 FEPSDK-1544"
 */
TEST(cTesterSignalRegistry, TestUserSignalOptionsConstructorNULL)
{
    cUserSignalOptions oOptions(NULL, SD_Undefined); //This should not crash!
    EXPECT_TRUE(oOptions.GetSignalName().empty());
    oOptions.SetSignalDirection(SD_Input);
    EXPECT_FALSE(oOptions.CheckValidity());   
}

/*
* Test Case:   TestUserSignalOptionsConstructorCopy
* Test ID:     1.7
* Test Title:  Test cUserSignalOptionsClass copy constructors
* Description: Test cUserSignalOptions copy and asignment constructors 
* Strategy:    Copy and assign different UserSignalOptions Configurations
* Passed If:   The copied/assigned values are correctly adopted
* Ticket:      -
* Requirement: XXX
*/
/**
 * @req_id "FEPSDK-1750 FEPSDK-1544"
 */
TEST(cTesterSignalRegistry, TestUserSignalOptionsConstructorCopy)
{
    cUserSignalOptions oOptions("", SD_Undefined);
    EXPECT_FALSE(oOptions.CheckValidity());
    cUserSignalOptions oCopyOptions(oOptions);
    EXPECT_FALSE(oCopyOptions.CheckValidity());
    cUserSignalOptions oAssignmentOptions = oOptions;
    EXPECT_FALSE(oCopyOptions.CheckValidity());
    
    cUserSignalOptions oOptionsValid("Signal", SD_Output, "SomeType");
    oOptionsValid.SetReliability(true);
    EXPECT_TRUE(oOptionsValid.CheckValidity());

    cUserSignalOptions oCopyOptionsValid(oOptionsValid);
    EXPECT_TRUE(oCopyOptionsValid.CheckValidity());
    EXPECT_TRUE(oCopyOptionsValid.GetSignalName() == oOptionsValid.GetSignalName());
    EXPECT_TRUE(oCopyOptionsValid.GetSignalType() == oOptionsValid.GetSignalType());
    EXPECT_EQ(oCopyOptionsValid.GetSignalDirection(), oOptionsValid.GetSignalDirection());
    EXPECT_EQ(oCopyOptionsValid.GetReliability(), oOptionsValid.GetReliability());
    EXPECT_EQ(oCopyOptionsValid.IsSignalRaw(), oOptionsValid.IsSignalRaw());


    cUserSignalOptions oAssignOptionsValid = oOptionsValid;
    EXPECT_TRUE(oAssignOptionsValid.CheckValidity());
    EXPECT_TRUE(oAssignOptionsValid.GetSignalName() == oOptionsValid.GetSignalName());
    EXPECT_TRUE(oAssignOptionsValid.GetSignalType() == oOptionsValid.GetSignalType());
    EXPECT_EQ(oAssignOptionsValid.GetSignalDirection(), oOptionsValid.GetSignalDirection());
    EXPECT_EQ(oAssignOptionsValid.GetReliability(), oOptionsValid.GetReliability());
    EXPECT_EQ(oAssignOptionsValid.IsSignalRaw(), oOptionsValid.IsSignalRaw());
}

/*
* Test Case : TestUserSignalOptionsRawDDL
* Test ID : 1.8
* Test Title : Test cUserSignalOptionsClass raw to ddl
* Description : Test cUserSignalOptions making raw options ddl options and vice versa
* Strategy : Create a RAW signal add type and check that IsSignalRaw returns false.
*            Set this DDL Signal back to Raw. IsSignal Raw returns true and the type
*            is empty.
* Passed If : no errors occur
* Ticket : -
*/
/**
 * @req_id "FEPSDK-1750 FEPSDK-1544"
 */
TEST(cTesterSignalRegistry, TestUserSignalOptionsRawDDL)
{
    cUserSignalOptions oOptions("AName", SD_Input);
    EXPECT_TRUE(oOptions.IsSignalRaw());
    oOptions.SetSignalType("SomeFancyType");
    EXPECT_FALSE(oOptions.IsSignalRaw());
    std::string strTest = oOptions.GetSignalType();
    EXPECT_TRUE(a_util::strings::compare(strTest.c_str(), "SomeFancyType") == 0);

    oOptions.SetSignalRaw();
    EXPECT_TRUE(oOptions.IsSignalRaw());
    EXPECT_TRUE(oOptions.GetSignalType().empty());
}

/*
* Test Case : TestUserSignalOptionsRTILowLat
* Test ID : 1.9
* Test Title : Test cUserSignalOptionsClass LowLatencyProfile
* Description : Test wether the cUserSignalOptions class deals correctly with 
*               low latency profil of RTI DDS
* Strategy : Check for correct default, check wether setting is stored
* Passed If : no errors occur
* Ticket : -
*/
/**
 * @req_id "FEPSDK-1750 FEPSDK-1544"
 */
TEST(cTesterSignalRegistry, TestUserSignalOptionsRTILowLat)
{
    cUserSignalOptions oOptions("AName", SD_Input);
    EXPECT_TRUE(oOptions.GetLowLatencySetting());
    oOptions.SetLowLatencyProfile(false);
    EXPECT_FALSE(oOptions.GetLowLatencySetting());
    oOptions.SetLowLatencyProfile(true);
    EXPECT_TRUE(oOptions.GetLowLatencySetting());
}

/*
* Test Case : TestUserSignalOptionsRTIAsyncPub
* Test ID : 1.10
* Test Title : Test cUserSignalOptionsClass Async Publisher mode
* Description : Test wether the cUserSignalOptions class deals correctly with
*               asyncpublisher mode of RTI DDS
* Strategy : Check for correct default, check wether setting is stored
* Passed If : no errors occur
* Ticket : -
*/
/**
 * @req_id "FEPSDK-1750 FEPSDK-1544"
 */
TEST(cTesterSignalRegistry, TestUserSignalOptionsRTIAsyncPub)
{
    cUserSignalOptions oOptions("AName", SD_Input);
    EXPECT_FALSE(oOptions.GetAsyncPublisherSetting());
    oOptions.SetLowLatencyProfile(true);
    EXPECT_TRUE(oOptions.GetLowLatencySetting());
    oOptions.SetLowLatencyProfile(false);
    EXPECT_FALSE(oOptions.GetLowLatencySetting());
}