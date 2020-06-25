/**
* Implementation of the tester for the FEP Common Functions and Classes
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
* Test Case:   TestTimestamp
* Test ID:     1.7
* Test Title:  Test implementation of FEP Timestamp functions
* Description: 
* Strategy:    
*              
* Passed If:   no errors occur
* Ticket:      #37419
* Requirement: FEPSDK-1616 FEPSDK-1617 FEPSDK-1618
*/

// no min/max macros on windows
#define NOMINMAX

#include <iostream>
#include <cstdarg>
#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include "fep_test_common.h"
#include "tester_fep_common_helper.h"
using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

/**
 * @req_id "FEPSDK-1616 FEPSDK-1617 FEPSDK-1618"
 */
TEST(cTesterFepCommon, TestTimestamp)
{
    time_t tmTimestampSeconds1UTC = time(NULL);

    timestamp_t tmTimestamp = GetTimeStampMicrosecondsUTC();
    tmTimestamp /= 1000000;

    // allow 10ms difference, since time() does not have the same accuracy as GetTimeStampMicrosecondsUTC()
    EXPECT_NEAR((double)tmTimestampSeconds1UTC, (double)tmTimestamp, 10);
}