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
* Test Case:   TestInitialization
* Test ID:     1.1
* Test Title:  Test Initialization
* Description: Test the initialization of the cDDB
* Strategy:    The cDDB is getting created and initialized correctly.
*              
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1584
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
 * @req_id "FEPSDK-1584"
 */
TEST(cTesterDDB, TestInitialization)
{
    cTestIncidentHandler oTestIncidentHandler;
    cDDB oDDB(&oTestIncidentHandler);
}