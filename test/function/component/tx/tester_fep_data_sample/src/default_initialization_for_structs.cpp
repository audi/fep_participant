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
* Test Case:   TestDefaultInitializationForNestedStructs
* Test ID:     1.5
* Test Title:  Initialization Tests with Default Values using Nested Structs
* Description: Test the automatic initialization of samples with default values in nested structs
* Strategy:  Create initialized samples and check if everything is as expected
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1579
*/
#include <gtest/gtest.h>

#include <fep_participant_sdk.h>
#include <ddl.h>

#include <fep_test_common.h>
#include "fep_data_sample_common.h"


using namespace fep;

/**
 * @req_id "FEPSDK-1579"
 */
TEST(cTesterFepDataSample, TestDefaultInitializationForNestedStructs)
{
    cTestBaseModule oModule;
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions( "TestModule")));

    IUserDataAccess * poAccess = oModule.GetUserDataAccess();
    ISignalRegistry* poCSR = oModule.GetSignalRegistry();
    ASSERT_TRUE(poAccess != NULL);

    handle_t hHandle = NULL;

    // Register an input signal
    ASSERT_EQ(a_util::result::SUCCESS, poCSR->RegisterSignalDescription(s_strDescription.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, poCSR->RegisterSignal(fep::cUserSignalOptions("TestSignal4",
        SD_Input, "tTestSignal4"), hHandle));

    // Get default values of the registered signal (no data has been received)
    fep::IUserDataSample const * poSample = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetUserDataAccess()->LockData(hHandle, poSample));
    ASSERT_TRUE(poSample != NULL);


    // Checking values
    ddl::CodecFactory oFactory("tTestSignal4", s_strDescription.c_str());
    ddl::StaticCodec oCoder = oFactory.makeStaticCodecFor(poSample->GetPtr(), 
        poSample->GetSize());

    {
        for (int j= 0; j< 4; ++j) 
        {
            {
                uint32_t v; 
                v= 0;
                ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::get_value(oCoder,
                    a_util::strings::format("arrayInside.ui32Values[%d]", j), &v).getErrorCode());
                ASSERT_TRUE(v == 12345);
            }
            {
                int32_t v; 
                v= -1;
                ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::get_value(oCoder,
                    a_util::strings::format("arrayInside.i32Values[%d]", j), &v).getErrorCode());
                ASSERT_TRUE(v == 0);
            }
            {
                double v; 
                v= std::numeric_limits<double>::infinity();
                ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::get_value(oCoder,
                    a_util::strings::format("arrayInside.f64Values[%d]", j), &v).getErrorCode());
                ASSERT_TRUE(std::fabs(v - 129876.12345) 
                    < std::numeric_limits<double>::epsilon());
            }
        }
    }

    for (int i= 0; i< 4; ++i) 
    {
        {
            uint32_t v; 
            v= 0;
            ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::get_value(oCoder,
                a_util::strings::format("somethings[%d].ui32DeepValue", i), &v).getErrorCode());
            ASSERT_TRUE(v == 65789);
        }
        {
            int32_t v; 
            v= -1;
            ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::get_value(oCoder,
                a_util::strings::format("somethings[%d].i32DeepValue", i), &v).getErrorCode());
            ASSERT_TRUE(v == 0);
        }

        for (int j= 0; j< 4; ++j) 
        {
            {
                uint32_t v; 
                v= 0;
                ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::get_value(oCoder,
                    a_util::strings::format("somethings[%d].arrayInside.ui32Values[%d]", i, j), &v).getErrorCode());
                ASSERT_TRUE(v == 12345);
            }
            {
                int32_t v; 
                v= -1;
                ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::get_value(oCoder,
                    a_util::strings::format("somethings[%d].arrayInside.i32Values[%d]", i, j), &v).getErrorCode());
                ASSERT_TRUE(v == 0);
            }
            {
                double v; 
                v= std::numeric_limits<double>::infinity();
                ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::get_value(oCoder,
                    a_util::strings::format("somethings[%d].arrayInside.f64Values[%d]", i, j), &v).getErrorCode());
                ASSERT_TRUE(std::fabs(v - 129876.12345) 
                    < std::numeric_limits<double>::epsilon());
            }
        }
    }

    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetUserDataAccess()->UnlockData(poSample));
}