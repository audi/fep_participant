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
* Test Case:   TestErrorHelperMacros
* Test ID:     
* Test Title:  Tests the provided error helper macros in fep_error_helpers.h
* Description: 
* Strategy:    
*              
* Passed If:   no errors occur
* Ticket:      #37419
* Requirement: XXX
*/

// no min/max macros on windows
#define NOMINMAX

#include <iostream>
#include <gtest/gtest.h>

#include <fep_participant_sdk.h>
#include "fep_error_helpers.h"

using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

fep::Result returnResult(int &test, fep::Result nResult)
{
    RETURN_IF_FAILED(nResult);
    test = 5;
    return fep::ERR_NOERROR;
}

fep::Result returnPointerNull(int &test)
{
    RETURN_IF_POINTER_NULL(NULL);
    test = 5;
    return fep::ERR_NOERROR;
}

/**
 * @req_id "FEPSDK-1664"
 */
TEST(cTesterFepCommon, TestErrorHelperMacroReturnIfFailed)
{
    int test = 0;
    EXPECT_TRUE(ERR_FAILED == returnResult(test, ERR_FAILED));
    EXPECT_EQ(test, 0);
    EXPECT_TRUE(ERR_INVALID_ARG == returnResult(test, ERR_INVALID_ARG));
    EXPECT_EQ(test, 0);
}

/**
 * @req_id "FEPSDK-1664"
 */
TEST(cTesterFepCommon, TestErrorHelperMacroReturnIfNull)
{
    int test = 0;
    EXPECT_TRUE(ERR_POINTER == returnPointerNull(test));
    EXPECT_EQ(test, 0);
}