/**
 * Implementation of the tester for the integration of FEP PropertyTree with FEP Transmission Adapter
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
* Test Case:   TestSetRemotePropertyValue
* Test ID:     1.5
* Test Title:  Convenience Function SetRemotePropertyValue
* Description: This testcase checks the convenience functions SetRemotePropertyValue for multiple receivers
*              (*) and single receiver for all datatypes.
* 
* Strategy:    Three modules Alice, Bob and Bernd are created. Alice tries to set properties of all types
*              for Bob and Bernd and all return values are checked as well as the values of the properties.
*              In addition, for Alice no property should be created (remote modules only!). The following
*              three cases are checked:
*              A: create (and set) remote property for ONE module
*              B: set remote property for all remote (!) module
*              C: change value of existing remote property
* Passed If:   no errors occur
*              
* Ticket:      #30812
* Requirement: FEPSDK-1565 FEPSDK-1604 FEPSDK-1605
*/
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::component_config;

#include "messages/fep_command_set_property.h"
#include "test_module.h"

/**
 * @req_id "FEPSDK-1565 FEPSDK-1604 FEPSDK-1605 FEPSDK-1796 FEPSDK-1798"
 */
TEST(cTesterPropertyTreeTransmissionAdapter, TestSetRemotePropertyValue)
{
     /* variables for results */
    fep::Result  nResult = ERR_NOERROR;
    bool    bValue      = false;
    double f64Value    = 0.0;
    int32_t   n32Value    = 0;
    const char *  pStrValue   = NULL;

    /* build module names with time stamp */
    std::string strTimePostfix = a_util::datetime::getCurrentSystemTime().format("%H%M%S");
    std::string strModNameAlice = 
        a_util::strings::format("TestSetRemotePropertyValue_Alice_%d", 
        strTimePostfix.c_str());
    std::string strModNameBob = 
        a_util::strings::format("TestSetRemotePropertyValue_Bob_%d", 
        strTimePostfix.c_str());
    std::string strModNameBernd = 
        a_util::strings::format("TestSetRemotePropertyValue_Bernd_%d", 
        strTimePostfix.c_str());
    
    /* Create 3 modules and bring them up to state RUNNING*/
    cTestModule oModAlice, oModBob, oModBernd;
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.Create(
        strModNameAlice.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oModBob.Create(
        strModNameBob.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oModBernd.Create(
        strModNameBernd.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.WaitForRunning(200000));
    ASSERT_EQ(a_util::result::SUCCESS, oModBob.WaitForRunning(200000));
    ASSERT_EQ(a_util::result::SUCCESS, oModBernd.WaitForRunning(200000));
    a_util::system::sleepMilliseconds(1000); /* 1.0 s */

    /* ###################################################################### */
    /* A: create (and set) remote property for ONE module */
    /* ###################################################################### */
    /* set new properties of each type for Bob */
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValue(
        strModNameBob.c_str(), "TestSetRemotePropertyValue.bTestValue", true));
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValue(
        strModNameBob.c_str(), "TestSetRemotePropertyValue.fTestValue", -42.0));
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValue(
        strModNameBob.c_str(), "TestSetRemotePropertyValue.strTestValue", "foo"));
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValue(
        strModNameBob.c_str(), "TestSetRemotePropertyValue.nTestValue", (int32_t) 17));
    a_util::system::sleepMilliseconds(2000); /* 2.0 s */

    /* check Bob: new properties exist and have correct values */
    ASSERT_EQ(a_util::result::SUCCESS, oModBob.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.bTestValue", bValue));
    ASSERT_EQ(a_util::result::SUCCESS, nResult = oModBob.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.fTestValue", f64Value));
    ASSERT_EQ(a_util::result::SUCCESS, oModBob.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.strTestValue", pStrValue));
    ASSERT_EQ(a_util::result::SUCCESS, oModBob.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.nTestValue", n32Value));
    ASSERT_TRUE(bValue);
    ASSERT_DOUBLE_EQ(-42.0, f64Value);
    ASSERT_TRUE(a_util::strings::isEqual(pStrValue, "foo"));
    ASSERT_EQ(17, n32Value);

    /* check Bernd: properties do not exist */
    ASSERT_EQ(ERR_PATH_NOT_FOUND,
        oModBernd.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.bTestValue", bValue));
    ASSERT_EQ(ERR_PATH_NOT_FOUND,
        oModBernd.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.fTestValue", f64Value));
    ASSERT_EQ(ERR_PATH_NOT_FOUND, 
        oModBernd.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.strTestValue", pStrValue));
    ASSERT_EQ(ERR_PATH_NOT_FOUND, 
        oModBernd.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.nTestValue", n32Value));

    /* check Alice: properties do not exist */
    ASSERT_EQ(ERR_PATH_NOT_FOUND, 
        oModAlice.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.bTestValue", bValue));
    ASSERT_EQ(ERR_PATH_NOT_FOUND, 
        oModAlice.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.fTestValue", f64Value));
    ASSERT_EQ(ERR_PATH_NOT_FOUND,
        oModAlice.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.strTestValue", pStrValue));
    ASSERT_EQ(ERR_PATH_NOT_FOUND,
        oModAlice.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.nTestValue", n32Value));

    /* ###################################################################### */
    /* B: set remote property for all remote (!) module */
    /* C: change value of existing remote property */
    /* ###################################################################### */
    /* set properties for all remote modules (receiver = "*") */
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValue(
        "*", "TestSetRemotePropertyValue.bTestValue", false));
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValue(
        "*", "TestSetRemotePropertyValue.fTestValue", -21.0));
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValue(
        "*", "TestSetRemotePropertyValue.strTestValue", "bar"));
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValue(
        "*", "TestSetRemotePropertyValue.nTestValue", (int32_t) 37));
    a_util::system::sleepMilliseconds(2000); /* 2.0 s */

    /* check Bob: properties still exist and have new values */
    ASSERT_EQ(a_util::result::SUCCESS, oModBob.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.bTestValue", bValue));
    ASSERT_EQ(a_util::result::SUCCESS, oModBob.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.fTestValue", f64Value));
    ASSERT_EQ(a_util::result::SUCCESS, oModBob.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.strTestValue", pStrValue));
    ASSERT_EQ(a_util::result::SUCCESS, oModBob.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.nTestValue", n32Value));
    ASSERT_FALSE(bValue);
    ASSERT_DOUBLE_EQ(-21.0, f64Value);
    ASSERT_TRUE(a_util::strings::isEqual(pStrValue, "bar"));
    ASSERT_EQ(37, n32Value);

    /* check Bernd: properties do exist now and have correct values*/
    ASSERT_EQ(a_util::result::SUCCESS, oModBernd.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.bTestValue", bValue));
    ASSERT_EQ(a_util::result::SUCCESS, oModBernd.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.fTestValue", f64Value));
    ASSERT_EQ(a_util::result::SUCCESS, oModBernd.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.strTestValue", pStrValue));
    ASSERT_EQ(a_util::result::SUCCESS, oModBernd.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.nTestValue", n32Value));
    ASSERT_FALSE(bValue);
    ASSERT_DOUBLE_EQ(-21.0, f64Value);
    ASSERT_TRUE(a_util::strings::isEqual(pStrValue, "bar"));
    ASSERT_EQ(37, n32Value);

    /* check Alice: properties do still not exist */
    ASSERT_EQ(ERR_PATH_NOT_FOUND,
        oModAlice.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.bTestValue", bValue));
    ASSERT_EQ(ERR_PATH_NOT_FOUND,
        oModAlice.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.fTestValue", f64Value));
    ASSERT_EQ(ERR_PATH_NOT_FOUND,
        oModAlice.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.strTestValue", pStrValue));
    ASSERT_EQ(ERR_PATH_NOT_FOUND,
        oModAlice.GetPropertyTree()->GetPropertyValue(
        "TestSetRemotePropertyValue.nTestValue", n32Value));

    /* ###################################################################### */
    /* Test if synchronous method works */
    /* ###################################################################### */
    /* set new properties of each type for Bob with synchronous method */
    timestamp_t tmTimeOutInMS = 4000;
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValue(
        strModNameBob.c_str(), "TestSetRemotePropertyValue.bTestValue_sync", true, tmTimeOutInMS));
    // testing if property exists 
    ASSERT_EQ(ERR_NOERROR,
        oModBob.GetPropertyTree()->GetPropertyValue(
            "TestSetRemotePropertyValue.bTestValue_sync", bValue));
    ASSERT_TRUE(bValue);

    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValue(
        strModNameBob.c_str(), "TestSetRemotePropertyValue.fTestValue_sync", 13.37, tmTimeOutInMS));
    // testing if property exists 
    ASSERT_EQ(ERR_NOERROR,
        oModBob.GetPropertyTree()->GetPropertyValue(
            "TestSetRemotePropertyValue.fTestValue_sync", f64Value));
    ASSERT_DOUBLE_EQ(13.37, f64Value);

    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValue(
        strModNameBob.c_str(), "TestSetRemotePropertyValue.nTestValue_sync", 42, tmTimeOutInMS));
    // testing if property exists 
    ASSERT_EQ(ERR_NOERROR,
        oModBob.GetPropertyTree()->GetPropertyValue(
            "TestSetRemotePropertyValue.nTestValue_sync", n32Value));
    ASSERT_TRUE(n32Value == 42);

    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValue(
        strModNameBob.c_str(), "TestSetRemotePropertyValue.strTestValue_sync", "Yolo", tmTimeOutInMS));
    // testing if property exists 
    ASSERT_EQ(ERR_NOERROR,
        oModBob.GetPropertyTree()->GetPropertyValue(
            "TestSetRemotePropertyValue.strTestValue_sync", pStrValue));
    ASSERT_TRUE(a_util::strings::isEqual(pStrValue, "Yolo"));
}
