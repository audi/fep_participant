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
* Test Case:   TestEnvCreation
* Test ID:     1.3
* Test Title:  Transmission selection by environment variable
* Description: Test the dtransmission selection with a specificially set environment variable
* Strategy:  Set the environment variable and create a new cModule.
*              
* Passed If:   no errors occur
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
TEST(cTesterTransmissionSelection, TestEnvCreation)
{
        cTestBaseModule oModuleDDS;

    // Ensure no environment variable is set.
    PutTransmissionDriverEnv("RTI_DDS");
    ASSERT_EQ(a_util::result::SUCCESS, oModuleDDS.Create("TestModuleDDS"));
}