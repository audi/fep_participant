/**
* Optional tester implementation
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
#include "_common/fep_optional.h"
#include <gtest/gtest.h>

/**
 * @req_id ""
 */
TEST(cTesterOptional, TestString)
{
    std::string strDefaultString = "This is my default string.";
    std::string strSetString = "Just another string.";

    cOptional<std::string> oWithout;
    cOptional<int> oWithoutInt;
    cOptional<bool> oWithoutBool;

    cOptional<std::string> oStringOptional(strDefaultString);
    EXPECT_FALSE(oStringOptional.IsSet());
    EXPECT_TRUE(strDefaultString == oStringOptional.GetValue());

    oStringOptional.SetValue(strSetString);
    EXPECT_TRUE(strSetString == oStringOptional.GetValue());
    EXPECT_TRUE(oStringOptional.IsSet());
}

/**
 * @req_id ""
 */
TEST(cTesterOptional, TestBool)
{
    bool bDefaultBool = true;
    bool bSetBool = false;

    cOptional<bool> oBoolOptional(bDefaultBool);
    EXPECT_FALSE(oBoolOptional.IsSet());
    EXPECT_TRUE(bDefaultBool == oBoolOptional.GetValue());

    oBoolOptional.SetValue(bSetBool);
    EXPECT_TRUE(oBoolOptional.IsSet());
    EXPECT_TRUE(bSetBool == oBoolOptional.GetValue());

}

/**
 * @req_id ""
 */
TEST(cTesterOptional, TestInteger)
{
    int nDefaultInt = 12345;
    int nSetInt = 67890;

    cOptional<int> oIntOptional(nDefaultInt);
    EXPECT_FALSE(oIntOptional.IsSet());
    EXPECT_EQ(oIntOptional.GetValue(), nDefaultInt);

    oIntOptional.SetValue(nSetInt);
    EXPECT_TRUE(oIntOptional.IsSet());
    EXPECT_EQ(oIntOptional.GetValue(), nSetInt);
}

