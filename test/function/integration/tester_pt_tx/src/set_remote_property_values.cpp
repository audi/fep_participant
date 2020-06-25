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
* Test Case:   TestSetRemotePropertyValues
* Test ID:     1.6
* Test Title:  Convenience Function SetRemotePropertyValues
* Description: This testcase checks the convenience functions SetRemotePropertyValues for multiple receivers 
*              (*) and single receiver for all datatypes.
* 
* Strategy:    Three modules Alice, Bob and Bernd are created. Alice tries to set properties of all types
*              for Bob and Bernd and all return values are checked as well as the values of the properties.
*              In addition, for Alice no property should be created (remote modules only!). The following
*              three cases are checked:
*              A: create (and set) remote property for ONE module
*              B: set remote property for all remote (!) module
*              C: change value of existing remote property
*              
* Passed If:   no errors occur
*              
* Ticket:      #31084
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
 * @req_id "FEPSDK-1565 FEPSDK-1604 FEPSDK-1605"
 */
TEST(cTesterPropertyTreeTransmissionAdapter, TestSetRemotePropertyValues)
{

    /* variables for results */
    bool    bValue      = false;
    double f64Value    = 0.0;
    int32_t   n32Value    = 0;
    const char *  strValue   = NULL;
    /* Properties for results */
    IProperty *poPropResBool = NULL;
    IProperty *poPropResFloat = NULL;
    IProperty *poPropResInt = NULL;
    IProperty *poPropResStr = NULL;

    /* build module names with time stamp */
    std::string strTimePostfix = a_util::datetime::getCurrentSystemTime().format("%H%M%S");
    std::string strModNameAlice = 
        a_util::strings::format("TestSetRemotePropertyValues_Alice_%d", 
        strTimePostfix.c_str());
    std::string strModNameBob = 
        a_util::strings::format("TestSetRemotePropertyValues_Bob_%d", 
        strTimePostfix.c_str());
    std::string strModNameBernd = 
        a_util::strings::format("TestSetRemotePropertyValues_Bernd_%d", 
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

    /* Create 4 Arrays which will be used for SetRemotePropertyValues */
    bool bArray[2] = {true,false};
    double f64Array[2] = {-42.0,15.0};
    int32_t n32Array[2] = {17,18};
    const char* strArray[2] = {"foo","bar"};

    /* Check invalid use */
    bool *pBool = NULL;
    /* invalid pointer */
    ASSERT_EQ(ERR_POINTER, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.bTestValues",pBool,2));
    ASSERT_EQ(ERR_POINTER, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        NULL, "TestSetRemotePropertyValues.bTestValues", bArray,2));
    ASSERT_EQ(ERR_POINTER, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), NULL, bArray,2));

    /* invalid size */
    ASSERT_EQ(ERR_INVALID_ARG, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.bTestValues",bArray,0));

    double *pFloat = NULL;
    /* invalid pointer */
    ASSERT_EQ(ERR_POINTER, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.fTestValues",pFloat,2));
    ASSERT_EQ(ERR_POINTER, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        NULL, "TestSetRemotePropertyValues.fTestValues", f64Array,2));
    ASSERT_EQ(ERR_POINTER, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), NULL, f64Array,2));

    /* invalid size */
    ASSERT_EQ(ERR_INVALID_ARG, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.fTestValues",f64Array,0));

    int32_t *pInt = NULL;
    /* invalid pointer */
    ASSERT_EQ(ERR_POINTER, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.nTestValues",pInt,2));
    ASSERT_EQ(ERR_POINTER, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        NULL, "TestSetRemotePropertyValues.nTestValues", n32Array,2));
    ASSERT_EQ(ERR_POINTER, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), NULL, n32Array,2));

    /* invalid size */
    ASSERT_EQ(ERR_INVALID_ARG, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.nTestValues",n32Array,0));

    char **pStr = NULL;
    /* invalid pointer */
    ASSERT_EQ(ERR_POINTER, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.strTestValues",pStr,2));
    ASSERT_EQ(ERR_POINTER, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        NULL, "TestSetRemotePropertyValues.strTestValues", strArray,2));
    ASSERT_EQ(ERR_POINTER, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), NULL, strArray,2));

    /* invalid size */
    ASSERT_EQ(ERR_INVALID_ARG, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.strTestValues",strArray,0));

    /* ###################################################################### */
    /* A: create (and set) remote property values for ONE module */
    /* ###################################################################### */
    /* set new properties of each type for Bob */
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.bTestValues", bArray,2));
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.fTestValues", f64Array,2));
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.nTestValues", n32Array,2));
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.strTestValues", strArray,2));
    a_util::system::sleepMilliseconds(2000); /* 2.0 s */

    /* check Bob: new properties exist and have correct values */
    /* we have to retreive the property and _then_ check the values since 
    * IPropertyTree::GetPropertyValue does not support indices... 
    * (has to be adjusted in package PropertyTree) */
    poPropResBool = NULL;
    poPropResBool = oModBob.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.bTestValues");
    ASSERT_TRUE(NULL != poPropResBool);
    poPropResFloat = NULL;
    poPropResFloat = oModBob.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.fTestValues");
    ASSERT_TRUE(NULL != poPropResFloat);
    poPropResInt = NULL;
    poPropResInt = oModBob.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.nTestValues");
    ASSERT_TRUE(NULL != poPropResInt);
    poPropResStr = NULL;
    poPropResStr = oModBob.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.strTestValues");
    ASSERT_TRUE(NULL != poPropResStr);

    ASSERT_EQ(a_util::result::SUCCESS, poPropResBool->GetValue(bValue,0));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResFloat->GetValue(f64Value,0));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResInt->GetValue(n32Value,0));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResStr->GetValue(strValue,0));
    ASSERT_TRUE(bValue);
    ASSERT_EQ(-42.0 , f64Value);
    ASSERT_EQ(17 , n32Value);
    ASSERT_TRUE(a_util::strings::isEqual("foo",strValue));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResBool->GetValue(bValue,1));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResFloat->GetValue(f64Value,1));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResInt->GetValue(n32Value,1));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResStr->GetValue(strValue,1));
    ASSERT_FALSE(bValue);
    ASSERT_EQ(15.0 , f64Value);
    ASSERT_EQ(18 , n32Value);
    ASSERT_TRUE(a_util::strings::isEqual("bar",strValue));

    /* check Bernd: properties do not exist */
    poPropResBool = NULL;
    poPropResBool = oModBernd.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.bTestValues");
    ASSERT_TRUE(NULL == poPropResBool);
    poPropResFloat = NULL;
    poPropResFloat = oModBernd.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.fTestValues");
    ASSERT_TRUE(NULL == poPropResFloat);
    poPropResInt = NULL;
    poPropResInt = oModBernd.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.nTestValues");
    ASSERT_TRUE(NULL == poPropResInt);
    poPropResStr = NULL;
    poPropResStr = oModBernd.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.strTestValues");
    ASSERT_TRUE(NULL == poPropResStr);

    /* check Alice: properties do not exist */
    poPropResBool = NULL;
    poPropResBool = oModAlice.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.bTestValues");
    ASSERT_TRUE(NULL == poPropResBool);
    poPropResFloat = NULL;
    poPropResFloat = oModAlice.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.fTestValues");
    ASSERT_TRUE(NULL == poPropResFloat);
    poPropResInt = NULL;
    poPropResInt = oModAlice.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.nTestValues");
    ASSERT_TRUE(NULL == poPropResInt);
    poPropResStr = NULL;
    poPropResStr = oModAlice.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.strTestValues");
    ASSERT_TRUE(NULL == poPropResStr);


    /* ###################################################################### */
    /* B: set remote property for all remote (!) module */
    /* C: change value of existing remote property */
    /* ###################################################################### */
    /* Update 3 Arrays which will be used for SetRemotePropertyValues */
    bArray[0] = false;
    bArray[1] = true;
    f64Array[0] = 15.0;
    f64Array[1] = -42.0;
    n32Array[0] = 18;
    n32Array[1] = 17;
    strArray[0] = "bar";
    strArray[1] = "foo";

    /* set properties for all remote modules (receiver = "*") */
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        "*", "TestSetRemotePropertyValues.bTestValues", bArray,2));
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        "*", "TestSetRemotePropertyValues.fTestValues", f64Array,2));
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        "*", "TestSetRemotePropertyValues.nTestValues", n32Array,2));
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        "*", "TestSetRemotePropertyValues.strTestValues", strArray,2));
    a_util::system::sleepMilliseconds(2000); /* 2.0 s */

    /* check Bob: properties still exist and have new values */
    poPropResBool = NULL;
    poPropResBool = oModBob.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.bTestValues");
    ASSERT_TRUE(NULL != poPropResBool);
    poPropResFloat = NULL;
    poPropResFloat = oModBob.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.fTestValues");
    ASSERT_TRUE(NULL != poPropResFloat);
    poPropResInt = NULL;
    poPropResInt = oModBob.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.nTestValues");
    ASSERT_TRUE(NULL != poPropResInt);
    poPropResStr = NULL;
    poPropResStr = oModBob.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.strTestValues");
    ASSERT_TRUE(NULL != poPropResStr);

    ASSERT_EQ(a_util::result::SUCCESS, poPropResBool->GetValue(bValue,0));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResFloat->GetValue(f64Value,0));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResInt->GetValue(n32Value,0));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResStr->GetValue(strValue,0));
    ASSERT_FALSE(bValue);
    ASSERT_EQ(15.0 , f64Value);
    ASSERT_EQ(18 , n32Value);
    ASSERT_TRUE(a_util::strings::isEqual("bar",strValue));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResBool->GetValue(bValue,1));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResFloat->GetValue(f64Value,1));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResInt->GetValue(n32Value,1));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResStr->GetValue(strValue,1));
    ASSERT_TRUE(bValue);
    ASSERT_EQ(-42.0 , f64Value);
    ASSERT_EQ(17 , n32Value);
    ASSERT_TRUE(a_util::strings::isEqual("foo",strValue));

    /* check Bernd: properties do exist now and have correct values*/
    poPropResBool = NULL;
    poPropResBool = oModBernd.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.bTestValues");
    ASSERT_TRUE(NULL != poPropResBool);
    poPropResFloat = NULL;
    poPropResFloat = oModBernd.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.fTestValues");
    ASSERT_TRUE(NULL != poPropResFloat);
    poPropResInt = NULL;
    poPropResInt = oModBernd.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.nTestValues");
    ASSERT_TRUE(NULL != poPropResInt);
    poPropResStr = NULL;
    poPropResStr = oModBernd.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.strTestValues");
    ASSERT_TRUE(NULL != poPropResStr);

    ASSERT_EQ(a_util::result::SUCCESS, poPropResBool->GetValue(bValue,0));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResFloat->GetValue(f64Value,0));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResInt->GetValue(n32Value,0));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResStr->GetValue(strValue,0));
    ASSERT_FALSE(bValue);
    ASSERT_EQ(15.0 , f64Value);
    ASSERT_EQ(18 , n32Value);
    ASSERT_TRUE(a_util::strings::isEqual("bar",strValue));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResBool->GetValue(bValue,1));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResFloat->GetValue(f64Value,1));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResInt->GetValue(n32Value,1));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResStr->GetValue(strValue,1));
    ASSERT_TRUE(bValue);
    ASSERT_EQ(-42.0 , f64Value);
    ASSERT_EQ(17 , n32Value);
    ASSERT_TRUE(a_util::strings::isEqual("foo",strValue));

    /* check Alice: properties do still not exist */
    poPropResBool = NULL;
    poPropResBool = oModAlice.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.bTestValues");
    ASSERT_TRUE(NULL == poPropResBool);
    poPropResFloat = NULL;
    poPropResFloat = oModAlice.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.fTestValues");
    ASSERT_TRUE(NULL == poPropResFloat);
    poPropResInt = NULL;
    poPropResInt = oModAlice.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.nTestValues");
    ASSERT_TRUE(NULL == poPropResInt);
    poPropResStr = oModAlice.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.strTestValues");
    ASSERT_TRUE(NULL == poPropResStr);

    /* ###################################################################### */
    /* Test if synchronous method works */
    /* ###################################################################### */
    /* set new properties of each type for Bob with synchronous method and directly verify if it exists */
    timestamp_t tmTimeOutInMS = 4000;
    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.bTestValues_sync", bArray, 2, tmTimeOutInMS));    
    // testing if property exists 
    poPropResBool = oModBob.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.bTestValues_sync");
    ASSERT_TRUE(NULL != poPropResBool);
    ASSERT_EQ(a_util::result::SUCCESS, poPropResBool->GetValue(bValue, 0));
    ASSERT_FALSE(bValue);
    ASSERT_EQ(a_util::result::SUCCESS, poPropResBool->GetValue(bValue, 1));
    ASSERT_TRUE(bValue);

    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.fTestValues_sync", f64Array, 2, tmTimeOutInMS));
    // testing if property exists     
    poPropResFloat = NULL;
    poPropResFloat = oModBob.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.fTestValues_sync");
    ASSERT_TRUE(NULL != poPropResFloat);
    ASSERT_EQ(a_util::result::SUCCESS, poPropResFloat->GetValue(f64Value, 0));
    ASSERT_EQ(15.0, f64Value);
    ASSERT_EQ(a_util::result::SUCCESS, poPropResFloat->GetValue(f64Value, 1));
    ASSERT_EQ(-42.0, f64Value);

    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.nTestValues_sync", n32Array, 2, tmTimeOutInMS));
    // testing if property exists 
    poPropResInt = NULL;
    poPropResInt = oModBob.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.nTestValues_sync");
    ASSERT_TRUE(NULL != poPropResInt);
    ASSERT_EQ(a_util::result::SUCCESS, poPropResInt->GetValue(n32Value, 0));
    ASSERT_EQ(18, n32Value);
    ASSERT_EQ(a_util::result::SUCCESS, poPropResInt->GetValue(n32Value, 1));
    ASSERT_EQ(17, n32Value);

    ASSERT_EQ(a_util::result::SUCCESS, oModAlice.GetPropertyTree()->SetRemotePropertyValues(
        strModNameBob.c_str(), "TestSetRemotePropertyValues.strTestValues_sync", strArray, 2, tmTimeOutInMS));
    // testing if property exists 
    poPropResStr = NULL;
    poPropResStr = oModBob.GetPropertyTree()->GetProperty(
        "TestSetRemotePropertyValues.strTestValues_sync");
    ASSERT_TRUE(NULL != poPropResStr);
    ASSERT_EQ(a_util::result::SUCCESS, poPropResStr->GetValue(strValue, 0));
    ASSERT_TRUE(a_util::strings::isEqual("bar", strValue));
    ASSERT_EQ(a_util::result::SUCCESS, poPropResStr->GetValue(strValue, 1));
    ASSERT_TRUE(a_util::strings::isEqual("foo", strValue));
}