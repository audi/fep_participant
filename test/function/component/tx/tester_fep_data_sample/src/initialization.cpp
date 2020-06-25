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
* Test Case:   TestInitialization
* Test ID:     1.2
* Test Title:  Initialization Tests
* Description: Test the automatic initialization of samples
* Strategy:  Create initialized samples and check if everything is as expected
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1510
*/
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include <fep_test_common.h>

#include "fep_data_sample_common.h"

using namespace fep;

/**
 * @req_id "FEPSDK-1510"
 */
TEST(cTesterFepDataSample, TestInitialization)
{
    cTestBaseModule oModule;
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions( "TestModule")));

    IUserDataAccess * poAccess = oModule.GetUserDataAccess();
    ISignalRegistry* poCSR = oModule.GetSignalRegistry();
    handle_t hHandle = NULL;
    IUserDataSample * poUserSample = NULL;

    ASSERT_TRUE(poAccess->CreateUserDataSample(poUserSample, reinterpret_cast<handle_t>(123)) == ERR_NOT_FOUND);
    ASSERT_EQ(a_util::result::SUCCESS, poCSR->RegisterSignalDescription(s_strDescription.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, poCSR->RegisterSignal(fep::cUserSignalOptions("TestSignal",
        SD_Output, "tTestSignal"), hHandle));

    ASSERT_EQ(a_util::result::SUCCESS, poAccess->CreateUserDataSample(poUserSample, hHandle));

    ASSERT_TRUE(poUserSample->GetSignalHandle() == hHandle);
    ASSERT_TRUE(poUserSample->GetSize() == 8);

    delete poUserSample;
}