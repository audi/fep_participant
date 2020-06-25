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
* Test Case:   TestFaultyEnvCreation
* Test ID:     1.4
* Test Title:  Transmission selection fails with wrong environment variable
* Description: Test whether the transmission selection fails when the environment variable is set to an erroneous value.
* Strategy:  Set the environment variable to something incorrect and try to create the module.
* Passed If:   the module isn't created
*              
* Ticket:      -
* Requirement: FEPSDK-1510
*/

#define _CRT_SECURE_NO_WARNINGS // disable warning about _snprintf
#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include <fep_test_common.h>
#include "transmission_selection_helper.h"

/**
 * @req_id "FEPSDK-1510"
 */
TEST(cTesterTransmissionSelection, TestFaultyEnvCreation)
{
    cTestBaseModule oModule;

    // Ensure no environment variable is set.
    PutTransmissionDriverEnv("NoSuchTransmissionAdapter");
    ASSERT_NE(a_util::result::SUCCESS, oModule.Create("TestModule"));
}