/**
 * Implementation of the tester for the FEP Data Sample
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
* Test Case:   TestDefaultInitialization
* Test ID:     1.0
* Test Title:  Initialization Tests with Default Values
* Description: Test the automatic initialization of samples with default values
* Strategy:  Create initialized samples and check if everything is as expected
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1511
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
#include "fep_data_sample_common.h"
using namespace fep;

#include <ddl.h>
#include "transmission_adapter/fep_data_sample_factory.h"



/**
 * @req_id "FEPSDK-1511"
 */
TEST(cTesterFepDataSample, TestDefaultInitialization)
{
    cTestBaseModule oModule;
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions( "TestModule")));

    IUserDataAccess * poAccess = oModule.GetUserDataAccess();
    ISignalRegistry* poCSR = oModule.GetSignalRegistry();
    ASSERT_TRUE(poAccess != NULL);

    IUserDataSample * poUserSample = NULL;
    handle_t hHandle = NULL;

    ASSERT_TRUE(poAccess->CreateUserDataSample(poUserSample, 
        reinterpret_cast<handle_t>(123)) == ERR_NOT_FOUND);

    ASSERT_EQ(a_util::result::SUCCESS, poCSR->RegisterSignalDescription(s_strDescription.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, poCSR->RegisterSignal(fep::cUserSignalOptions("TestSignal2",
        SD_Input, "tTestSignal2"), hHandle));

    fep::IUserDataSample const * poSample= NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetUserDataAccess()->LockData(hHandle, poSample));

    ASSERT_TRUE(poSample != NULL);
    ASSERT_TRUE(poSample->GetSize() == sizeof(tTestSignal2));

    // Checking values
    ddl::CodecFactory oFactory("tTestSignal2", s_strDescription.c_str());
    ddl::StaticCodec oCoder = oFactory.makeStaticCodecFor(poSample->GetPtr(), 
        poSample->GetSize());

    tTestSignal2 * pSignal = reinterpret_cast<tTestSignal2*>(poSample->GetPtr());

    {
        ASSERT_TRUE(ddl::access_element::get_value(oCoder, "ui32Value1").asUInt32() == 0);
        ASSERT_TRUE(pSignal->ui32Value1 == 0);
    }

    {
        ASSERT_TRUE(ddl::access_element::get_value(oCoder, "ui32Value2").asUInt32() == 12345);
        ASSERT_TRUE(pSignal->ui32Value2 == 12345);
    }

    {
        float v = ddl::access_element::get_value(oCoder, "f32Value").asFloat(); 
        ASSERT_TRUE(std::fabs(v - 9876.12345f) 
            < std::numeric_limits<float>::epsilon());
        ASSERT_TRUE(std::fabs(pSignal->f32Value - 9876.12345f) 
            < std::numeric_limits<float>::epsilon());
    }

    {
        double v = ddl::access_element::get_value(oCoder, "f64Value").asDouble();
        ASSERT_TRUE(std::fabs(v - 129876.12345) 
            < std::numeric_limits<double>::epsilon());
        ASSERT_TRUE(std::fabs(pSignal->f64Value - 129876.12345) 
            < std::numeric_limits<double>::epsilon());
    }

    {
        ASSERT_TRUE(ddl::access_element::get_value(oCoder, "bValue").asBool());
        ASSERT_TRUE(pSignal->bValue);
    }

    {
        ASSERT_TRUE(ddl::access_element::get_value(oCoder, "cValue").asUInt8() == 42);
        ASSERT_TRUE(pSignal->cValue == 42);
    }

    {
        ASSERT_TRUE(ddl::access_element::get_value(oCoder, "ui8Value").asUInt8() == 254);
        ASSERT_TRUE(pSignal->ui8Value == 254);
    }

    {
        ASSERT_TRUE(ddl::access_element::get_value(oCoder, "i8Value").asInt8() == -127);
        ASSERT_TRUE(pSignal->i8Value == -127);
    }

    {
        ASSERT_TRUE(ddl::access_element::get_value(oCoder, "ui16Value").asUInt16() == 65533);
        ASSERT_TRUE(pSignal->ui16Value == 65533);
    }

    {
        ASSERT_TRUE(ddl::access_element::get_value(oCoder, "i16Value").asInt16() == -32720);
        ASSERT_TRUE(pSignal->i16Value == -32720);
    }

    {
        char v; 
        for (int i= 0; i< 8; ++i) {
            ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::get_value(oCoder,
                a_util::strings::format("strValue1[%d]", i), &v).getErrorCode());

            ASSERT_TRUE(v == 42);
            ASSERT_TRUE(pSignal->strValue1[i] == 42);
        }
    }

    {
        char v;
        for (int i= 0; i< 8; ++i) {
            ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::get_value(oCoder,
                a_util::strings::format("strValue2[%d]", i), &v).getErrorCode());

            ASSERT_TRUE(v == 21);
            ASSERT_TRUE(pSignal->strValue2[i] == 21);
        }

    }

    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetUserDataAccess()->UnlockData(poSample));
    delete poUserSample;
}