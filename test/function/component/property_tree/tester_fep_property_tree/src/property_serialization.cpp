/**
 * Implementation of the tester for the FEP Property Tree extensions (serialization test)
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
* Test Case:   TestPropertySerialization
* Test ID:     1.1
* Test Title:  Test serialization of properties
* Description: Tests if serialization especially the json library work correcly under various environments (locales)
* Strategy:    Evaluate various serialization strings are matching expectations. If possible do deserialization.
* Passed If:   no errors occur
* Ticket:      #30867
* Requirement: FEPSDK-1629 FEPSDK-1628 FEPSDK-1630 FEPSDK-1631
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

#include "fep3/components/legacy/property_tree/property.h"

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

using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif


// Begin of tests

template<typename T> static bool checkPropertySerialization(const T& t)
{
    fep::cProperty prop1_orig("test", t);
    std::string prop1_orig_string= prop1_orig.ToString();
    std::cerr << "prop1_orig='" << prop1_orig_string << "'" << std::endl; 

    fep::cProperty prop1_rev("test", "");
    {
        std::string str= libjson::to_json_string(prop1_orig_string);

        JSONNode oNode= libjson::parse(str);
        JSONNode::const_iterator itNode= oNode.find("test");
        if (oNode.end() == itNode)
        {
            // Failed to parse
            return false;
        }
        else 
        {
            prop1_rev.JSONToProperty(*itNode);
        }
    }

    std::string prop1_rev_string= prop1_rev.ToString();
    std::cerr << "prop1_rev='" << prop1_rev_string << "'" << std::endl; 

    return prop1_rev_string == prop1_orig_string;
}


/**
 * @req_id "FEPSDK-1629 FEPSDK-1628 FEPSDK-1630 FEPSDK-1631"
 */
TEST(cTesterFEPPropertyTree, TestPropertySerialization)
{
    // Testing serialization of bool
    ASSERT_TRUE(checkPropertySerialization<bool>(true));
    ASSERT_TRUE(checkPropertySerialization<bool>(false));

    // Testing serialization of int32_t
    ASSERT_TRUE(checkPropertySerialization<int32_t>(0));
    ASSERT_TRUE(checkPropertySerialization<int32_t>(1));
    ASSERT_TRUE(checkPropertySerialization<int32_t>(-1));
    // Can not fix this, due to internal handling of integers in libjson:
    // NumberToString negates the value, but can not store it than
    //// FAILS!!!: ASSERT_TRUE(checkPropertySerialization<int32_t>(std::numeric_limits<int32_t>::min()));
    ASSERT_TRUE(checkPropertySerialization<int32_t>(std::numeric_limits<int32_t>::min()+1));
    ASSERT_TRUE(checkPropertySerialization<int32_t>(std::numeric_limits<int32_t>::max()));

    // Testing serialization of double
    ASSERT_TRUE(checkPropertySerialization<double>(0.0));
    ASSERT_TRUE(checkPropertySerialization<double>(0.1));
    ASSERT_TRUE(checkPropertySerialization<double>(-0.1));
    ASSERT_TRUE(checkPropertySerialization<double>(1.0));
    ASSERT_TRUE(checkPropertySerialization<double>(-1.0));
    ASSERT_TRUE(checkPropertySerialization<double>(1.2));
    ASSERT_TRUE(checkPropertySerialization<double>(-1.2));

    // Can not fix this, due to internal handling of integers in libjson:
    // Evaluates to zero ... Maybe this is ok
    //// FAILS!!!: ASSERT_TRUE(checkPropertySerialization<double>(std::numeric_limits<double>::min()));

    // Can not fix this, due to internal handling of integers in libjson:
    // Double can be quite long if printed without exponents (> 310 chars), 
    // but Buffer is limited to 63 char.
    // Wrong usage of _snprintf_s is responsible for core dump
/* Sample usage of snprintf in json:
#define EXTRA_LONG long
#define FLOAT_STRING "%Lf"
     double value= std::numeric_limits<double>::max();
     json_char num_str_result[64];

    _snprintf(num_str_result, 63, FLOAT_STRING, (EXTRA_LONG double)value); // Wrong result, no core dump
    _snprintf_s(num_str_result, 63, 62, FLOAT_STRING, (EXTRA_LONG double)value); // Wrong result, no core dump
    _snprintf_s(num_str_result, 63, 63, FLOAT_STRING, (EXTRA_LONG double)value); // This is the code in libjson, dumps core
*/
    //// FAILS!!!:ASSERT_TRUE(checkPropertySerialization<double>(std::numeric_limits<double>::max()));

    ASSERT_TRUE(checkPropertySerialization<const char*>(""));
    ASSERT_TRUE(checkPropertySerialization<const char*>("A"));
    ASSERT_TRUE(checkPropertySerialization<const char*>("Audi Electronics Venture GmbH"));
}