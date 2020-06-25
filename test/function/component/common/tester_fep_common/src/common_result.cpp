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
* Test Case:   TestResult
* Test Title:  Test implementation of FEP Result type
* Description: 
* Strategy:    
*              
* Passed If:   no errors occur
* Ticket:      FEPSDK-765
* Requirement: <todo>
*/

#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
using namespace fep;

/**
 * @req_id "FEPSDK-1664"
 */
TEST(cTesterFepCommon, TestResult)
{
    fep::Result res = ERR_NOERROR;
    EXPECT_TRUE(fep::isOk(res));
    EXPECT_FALSE(fep::isFailed(res));

    res |= ERR_ACCESS_DENIED;
    EXPECT_EQ(res, ERR_ACCESS_DENIED);
    EXPECT_TRUE(fep::isFailed(res));
    EXPECT_FALSE(fep::isOk(res));

    res |= ERR_DAU;
    EXPECT_EQ(res, ERR_ACCESS_DENIED);
}