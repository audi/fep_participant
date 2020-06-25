/**
* Implementation of the tester for the FEP Automation Interface
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
* Test Case:   TestGetPropertyValueStringSegfault
* Test ID:     1.18
* Test Title:  GetPropertyValue give the right string back
* Description: Tests passing a std::string to the method GetPropertyValue
* Strategy:    An unitialized std::string is passed to the
*              method. The element name must be given back
*              
* Passed If:   return string is the same as the element name
* Ticket:      -
* Requirement: FEPSDK-1594
*/
#include <gtest/gtest.h>
#include <fep_test_common.h>

#include "helper_functions.h"

/**
 * @req_id "FEPSDK-1594"
 */
TEST(cTesterFepAutomation, TestGetSetPropertyValue)
{
    AutomationInterface oAI;
    cTestBaseModule oModule;
    const char* strName = "cTesterFepModule_TestPropertyValue";
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions(strName)));
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);

    // Getter and setter for string
    std::string strVar("New Description");
    std::string strResultVar;
    ASSERT_EQ(a_util::result::SUCCESS,
        oAI.SetPropertyValue(fep::g_strElementHeaderPath_strElementDescription,
            strVar, oModule.GetName()));
    ASSERT_EQ(a_util::result::SUCCESS,
        oAI.GetPropertyValue(fep::g_strElementHeaderPath_strElementDescription,
            strResultVar, oModule.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(strResultVar, "New Description");

    // Getter and setter for bool
    bool bGlobalMute = false;
    ASSERT_EQ(a_util::result::SUCCESS,
        oAI.SetPropertyValue(fep::g_strElementHeaderPath_bGlobalMute,
            true, oModule.GetName()));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetPropertyValue(fep::g_strElementHeaderPath_bGlobalMute,
            bGlobalMute, oModule.GetName(), REM_PROP_TIMEOUT));
    ASSERT_TRUE(bGlobalMute);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.SetPropertyValue(fep::g_strElementHeaderPath_bGlobalMute,
            false, oModule.GetName()));

    // Getter and setter for double
    double fVal;
    ASSERT_EQ(a_util::result::SUCCESS, oAI.SetPropertyValue(fep::g_strElementHeaderPath_fElementVersion,
            4.2, oModule.GetName()));
    a_util::system::sleepMilliseconds(100);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetPropertyValue(fep::g_strElementHeaderPath_fElementVersion,
            fVal, oModule.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(fVal, 4.2);

    // Set and delete new Property
    ASSERT_EQ(a_util::result::SUCCESS, oAI.SetPropertyValue("MyNewProperty",
            1000.42, oModule.GetName()));
    a_util::system::sleepMilliseconds(100);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetPropertyValue("MyNewProperty",
            fVal, oModule.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(fVal, 1000.42);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.DeleteProperty("MyNewProperty", oModule.GetName()));

    // Set int array property
    int32_t nVal;
    std::vector<int32_t> nVec;
    nVec.push_back(1000);
    nVec.push_back(42);
    nVec.push_back(5);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.SetPropertyValues("MyNewArrayProperty",
        nVec, oModule.GetName()));
    a_util::system::sleepMilliseconds(100);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetPropertyValue("MyNewArrayProperty",
        nVal, oModule.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(nVal, 1000);
    nVec.clear();
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetPropertyValues("MyNewArrayProperty",
        nVec, oModule.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(nVec[1], 42);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.DeleteProperty("MyNewArrayProperty", oModule.GetName()));

    // Set string array property
    std::string strVal;
    std::vector<std::string> strVec;
    strVec.push_back("Hello1");
    strVec.push_back("Hello2");
    strVec.push_back("Hello3");
    ASSERT_EQ(a_util::result::SUCCESS, oAI.SetPropertyValues("MyNewArrayProperty",
        strVec, oModule.GetName()));
    a_util::system::sleepMilliseconds(100);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetPropertyValue("MyNewArrayProperty",
        strVal, oModule.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(strVal, "Hello1");
    strVec.clear();
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetPropertyValues("MyNewArrayProperty",
        strVec, oModule.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(strVec[1], "Hello2");
    ASSERT_EQ(a_util::result::SUCCESS, oAI.DeleteProperty("MyNewArrayProperty", oModule.GetName()));

    // Set bool array property
    bool bVal;
    std::vector<bool> bVec;
    bVec.push_back(true);
    bVec.push_back(false);
    bVec.push_back(true);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.SetPropertyValues("MyNewArrayProperty",
        bVec, oModule.GetName()));
    a_util::system::sleepMilliseconds(100);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetPropertyValue("MyNewArrayProperty",
        bVal, oModule.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(bVal, true);
    bVec.clear();
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetPropertyValues("MyNewArrayProperty",
        bVec, oModule.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(bVec[1], false);
    ASSERT_EQ(bVec[2], true);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.DeleteProperty("MyNewArrayProperty", oModule.GetName()));

    // GetProperty
    std::unique_ptr<fep::IProperty> pProperty;
    oAI.GetProperty(fep::g_strElementHeaderPath_strElementDescription, pProperty, oModule.GetName());
    ASSERT_TRUE(pProperty);
    ASSERT_TRUE(pProperty->IsString());
    const char* strValue = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, pProperty->GetValue(strValue));
    ASSERT_EQ(std::string(strValue), "New Description");
}