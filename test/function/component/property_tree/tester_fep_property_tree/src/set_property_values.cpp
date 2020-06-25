/**
 * Implementation of the tester for the FEP Property Tree extensions (setting properties test)
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
* Test Case:   TestSetPropertyValues
* Test ID:     1.2
* Test Title:  Test setting of property arrays
* Description: Tests if setting property arrays works as expected
* Strategy:    Try to set various arrays, check the Property Tree; Test invalid input
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1413 FEPSDK-1414
*/



#ifdef WIN32
// Disable min/max macros. They collide with std::numeric_limits. 
#define NOMINMAX
#endif // WIN32

#include <algorithm>
#include <iostream>
#include <limits>
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;

#include "fep3/components/legacy/property_tree/fep_propertytree.h"

#if __GNUC__
    // Avoid lots of warnings in libjson
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wreorder"
#endif
#include "libjson.h"
#if __GNUC__
    // Restore previous behaviour
    #pragma GCC diagnostic pop
#endif

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

/**
 * @req_id "FEPSDK-1413 FEPSDK-1414 FEPSDK-1792 FEPSDK-1795"
 */
TEST(cTesterFEPPropertyTree, TestSetPropertyValues)
{
    /* variables for results */
    bool    bValue      = false;
    double f64Value    = 0.0;
    int32_t   n32Value    = 0;
    const char * strValue   = NULL;
    /* Properties for results */
    IProperty *poPropResBool = NULL;
    IProperty *poPropResFloat = NULL;
    IProperty *poPropResInt = NULL;
    IProperty *poPropResStr = NULL;

    /* Create 3 Arrays which will be used for SetPropertyValues */
    bool bArray[2] = {true,false};
    double f64Array[2] = {-42.0,15.0};
    int32_t n32Array[2] = {17,18};
    const char* strArray[2] = {"foo","bar"};

    std::string strBArray("bTest");
    std::string strF64Array("f64Test");
    std::string strN32Array("n32Test");
    std::string strStrArray("strTest");

    // property tree instance
    cPropertyTree oPropertyTree;

    /* Test invalid input */
    bool *pBool = NULL;
    ASSERT_TRUE(ERR_POINTER == oPropertyTree.SetPropertyValues(NULL,bArray,2));
    ASSERT_TRUE(ERR_POINTER == oPropertyTree.SetPropertyValues(strBArray.c_str(),pBool,2));

    ASSERT_TRUE(ERR_INVALID_ARG == oPropertyTree.SetPropertyValues(strBArray.c_str(),bArray,0));

    double *pFloat = NULL;
    ASSERT_TRUE(ERR_POINTER == oPropertyTree.SetPropertyValues(NULL,f64Array,2));
    ASSERT_TRUE(ERR_POINTER == oPropertyTree.SetPropertyValues(strF64Array.c_str(),pFloat,2));

    ASSERT_TRUE(ERR_INVALID_ARG == oPropertyTree.SetPropertyValues(strF64Array.c_str(),f64Array,0));

    int32_t *pInt = NULL;
    ASSERT_TRUE(ERR_POINTER == oPropertyTree.SetPropertyValues(NULL,n32Array,2));
    ASSERT_TRUE(ERR_POINTER == oPropertyTree.SetPropertyValues(strN32Array.c_str(),pInt,2));

    ASSERT_TRUE(ERR_INVALID_ARG == oPropertyTree.SetPropertyValues(strN32Array.c_str(),n32Array,0));

    char **pStr = NULL;
    ASSERT_TRUE(ERR_POINTER == oPropertyTree.SetPropertyValues(NULL,n32Array,2));
    ASSERT_TRUE(ERR_POINTER == oPropertyTree.SetPropertyValues(strStrArray.c_str(),pStr,2));

    ASSERT_TRUE(ERR_INVALID_ARG == oPropertyTree.SetPropertyValues(strN32Array.c_str(),n32Array,0));

    /* Set Properties and retrieve */
    ASSERT_TRUE(ERR_NOERROR == oPropertyTree.SetPropertyValues(strBArray.c_str(),bArray,2));
    ASSERT_TRUE(ERR_NOERROR == oPropertyTree.SetPropertyValues(strF64Array.c_str(),f64Array,2));
    ASSERT_TRUE(ERR_NOERROR == oPropertyTree.SetPropertyValues(strN32Array.c_str(),n32Array,2));
    ASSERT_TRUE(ERR_NOERROR == oPropertyTree.SetPropertyValues(strStrArray.c_str(),strArray,2));

    poPropResBool = oPropertyTree.GetProperty(strBArray.c_str());
    poPropResFloat = oPropertyTree.GetProperty(strF64Array.c_str());
    poPropResInt = oPropertyTree.GetProperty(strN32Array.c_str());
    poPropResStr = oPropertyTree.GetProperty(strStrArray.c_str());

    ASSERT_TRUE(NULL != poPropResBool);
    ASSERT_TRUE(NULL != poPropResFloat);
    ASSERT_TRUE(NULL != poPropResInt);
    ASSERT_TRUE(NULL != poPropResStr);

    ASSERT_TRUE(ERR_NOERROR == poPropResBool->GetValue(bValue,0));
    ASSERT_TRUE(ERR_NOERROR == poPropResFloat->GetValue(f64Value,0));
    ASSERT_TRUE(ERR_NOERROR == poPropResInt->GetValue(n32Value,0));
    ASSERT_TRUE(ERR_NOERROR == poPropResStr->GetValue(strValue,0));
    ASSERT_TRUE(true == bValue);
    ASSERT_TRUE(-42.0 == f64Value);
    ASSERT_TRUE(17 == n32Value);
    ASSERT_TRUE(a_util::strings::isEqual("foo",strValue));
    ASSERT_TRUE(ERR_NOERROR == poPropResBool->GetValue(bValue,1));
    ASSERT_TRUE(ERR_NOERROR == poPropResFloat->GetValue(f64Value,1));
    ASSERT_TRUE(ERR_NOERROR == poPropResInt->GetValue(n32Value,1));
    ASSERT_TRUE(ERR_NOERROR == poPropResStr->GetValue(strValue,1));
    ASSERT_TRUE(false == bValue);
    ASSERT_TRUE(15.0 == f64Value);
    ASSERT_TRUE(18 == n32Value);
    ASSERT_TRUE(a_util::strings::isEqual("bar",strValue));
}


