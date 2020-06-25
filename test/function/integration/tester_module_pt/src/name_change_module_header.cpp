/**
 * Implementation of the tester for the integration of FEP Module with the FEP Property Tree.
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
/**
* Test Case:   TestNameChangeModuleHeader
* Test ID:     1.4
* Test Title:  Test the participant header in the property tree after a name change
* Description: This Test checks the participant header after a name change
* Strategy:    A FEP Element is started, its name is changed and its participant header is checked
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1601
*/

#include <cstring>

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

#include <ddl.h>

/**
 * @req_id "FEPSDK-1601"
 */
TEST(cTesterModulePropertyTree, TestNameChangeModuleHeader)
{
    cTestBaseModule oMod;
    ASSERT_EQ(a_util::result::SUCCESS, oMod.Create(cModuleOptions("Test")));

    // initial value check
    const char* strModuleName = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oMod.GetPropertyTree()->
        GetPropertyValue(fep::g_strElementHeaderPath_strElementName, strModuleName));
    ASSERT_TRUE(a_util::strings::isEqual(strModuleName, MakePlatformDepName("Test").c_str()));

    // rename
    ASSERT_EQ(a_util::result::SUCCESS, oMod.GetPropertyTree()->SetPropertyValue(
        fep::g_strElementHeaderPath_strElementName, MakePlatformDepName("TestNew").c_str()));

    // check participant header again
    ASSERT_EQ(a_util::result::SUCCESS, oMod.GetPropertyTree()->
        GetPropertyValue(fep::g_strElementHeaderPath_strElementName, strModuleName));
    ASSERT_TRUE(a_util::strings::isEqual(strModuleName, MakePlatformDepName("TestNew").c_str()));
}