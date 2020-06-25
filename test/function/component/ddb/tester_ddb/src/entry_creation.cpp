/**
* Implementation of the tester for the FEP Distributed Data Buffer
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
* Test Case:   TestEntryCreation
* Test ID:     1.2
* Test Title:  Test entry creation
* Description: Test the creation of entries in the DDB
* Strategy:    Entries are created without any problems.
*              
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1464 FEPSDK-1466
*/
#include <iostream>
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include "fep_test_common.h"
using namespace fep;

#include "distributed_data_buffer/fep_ddb.h"
#include "distributed_data_buffer/fep_ddb_frame.h"
#include "distributed_data_buffer/fep_ddb_frame_factory.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include "test_fixture.h"
#include "helper_functions.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

/**
 * @req_id "FEPSDK-1464 FEPSDK-1466"
 */
TEST(cTesterDDB, TestEntryCreation)
{
    cTestIncidentHandler oTestIncidentHandler;
    handle_t hSignal = (void*)static_cast<std::uintptr_t>(0xDEADBEEF);

    cDDB oDDB(&oTestIncidentHandler);
    ASSERT_EQ(ERR_NOERROR     , oDDB.CreateEntry(hSignal, 1));
    ASSERT_EQ(ERR_INVALID_ARG , oDDB.CreateEntry(hSignal, 0));
    ASSERT_EQ(ERR_INVALID_ARG , oDDB.CreateEntry(NULL, 1));
    ASSERT_EQ(ERR_INVALID_ARG , oDDB.CreateEntry(NULL, 0));
}