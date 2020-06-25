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
* Test Case:   TestDefaultInitializationForArrays2
* Test ID:     1.4
* Test Title:  Initialization Tests with Default Values inside nested arrays
* Description: Test the automatic initialization of samples with default values in arrays
* Strategy:  Create initialized samples and check if everything is as expected
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1578
*/
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include <fep_test_common.h>
#include <ddl.h>
#include "fep_data_sample_common.h"

using namespace fep;

/**
 * @req_id "FEPSDK-1578"
 */
TEST(cTesterFepDataSample, TestDefaultInitializationForArrays2)
{
    cTestBaseModule oModule;
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions( "TestModule")));

    IUserDataAccess * poAccess = oModule.GetUserDataAccess();
    ISignalRegistry* poCSR = oModule.GetSignalRegistry();
    ASSERT_TRUE(poAccess != NULL);

    handle_t hHandle = NULL;

    // Register an input signal
    ASSERT_EQ(a_util::result::SUCCESS, poCSR->RegisterSignalDescription(s_strDescription.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, poCSR->RegisterSignal(fep::cUserSignalOptions("TestSignal3c",
        SD_Input, "tTestSignal3c"), hHandle));

    // Get default values of the registered signal (no data has been received)
    fep::IUserDataSample const * poSample = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetUserDataAccess()->LockData(hHandle, poSample));
    ASSERT_TRUE(poSample != NULL);


    // Checking values
    ddl::CodecFactory oFactory("tTestSignal3c", s_strDescription.c_str());
    ddl::StaticCodec oCoder = oFactory.makeStaticCodecFor(poSample->GetPtr(), 
        poSample->GetSize());

    for (int j= 0; j< 4; ++j) 
    {
        for (int i= 0; i< 16; ++i) 
        {
            // FIXME: Is there a way to detect length of array element (arraysize)
            //        Currently using hardcoded value (see xml of "tTestSignal3b")
            if (i < 16)
            {
                char v;;
                ASSERT_EQ(a_util::result::SUCCESS, ddl::access_element::get_value(oCoder, a_util::strings::format("fields[%d].text[%d]", j, i), &v).getErrorCode());
                ASSERT_TRUE(v == 42);
            }
        }
    }

    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetUserDataAccess()->UnlockData(poSample));
}