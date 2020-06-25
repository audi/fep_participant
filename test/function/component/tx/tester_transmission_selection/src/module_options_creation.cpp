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
* Test Case:   TestModuleOptionsCreation
* Test ID:     1.5
* Test Title:  Select transmission using module options
* Description: Test the transmission selection using module options
* Strategy:  Test the transmission selection using module options
* Passed If:   no errors occur
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
TEST(cTesterTransmissionSelection, TestModuleOptionsCreation)
{
    {
        cTestBaseModule oModule;

        cModuleOptions oModuleOptions("TestModule");
        oModuleOptions.SetTransmissionType(static_cast<tTransmissionType>(1)); // Previous value for: fep::TT_ConnLib:

        ASSERT_TRUE(oModule.Create(oModuleOptions) == ERR_NOT_SUPPORTED);
    }

    {
        cTestBaseModule oModule;

        cModuleOptions oModuleOptions("TestModule");
        oModuleOptions.SetTransmissionType(static_cast<tTransmissionType>(999)); // Invalid value

        ASSERT_TRUE(oModule.Create(oModuleOptions) == ERR_INVALID_ARG);
    }

    {
        cTestBaseModule oModule;

        cModuleOptions oModuleOptions("TestModule");
        oModuleOptions.SetTransmissionType(fep::TT_RTI_DDS); // DDS Transmission

        ASSERT_TRUE(oModule.Create(oModuleOptions) == ERR_NOERROR);
    }
}