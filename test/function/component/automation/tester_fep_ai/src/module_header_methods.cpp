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
* Test Case:   TestModuleHeaderMethods
* Test ID:     1.5
* Test Title:  Test the participant header convenience functions of cModule
* Description: Tests GetModuleXX methods
* Strategy:    The modlue header is set by SetPropertyValue. The information is retrieved via the 
*              convenience functions locally and remotely. The convenience functions are called 
*              with invaild/improper arguments.
*              
* Passed If:   The convenience functions return the values that were set (for remote aswell as
*              local retrieval). The functions return the expected error codes.
* Ticket:      -
* Requirement: FEPSDK-1487, FEPSDK-1621
*/
#include <gtest/gtest.h>
#include <fep_test_common.h>

#include "helper_functions.h"

/**
 * @req_id "FEPSDK-1487, FEPSDK-1621 FEPSDK-1804"
 */
TEST(cTesterFepAutomation, TestModuleHeaderMethods)
{
    AutomationInterface oAI;
    cTestBaseModule oMod;
    ASSERT_EQ(a_util::result::SUCCESS, oMod.Create(cModuleOptions("Mod2")));
    // leave some time for discovery
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);

    // Fill property tree
    IPropertyTree * poTree = oMod.GetPropertyTree();
    ASSERT_EQ(a_util::result::SUCCESS, poTree->SetPropertyValue(
        fep::g_strElementHeaderPath_fElementVersion, 0.1));
    ASSERT_EQ(a_util::result::SUCCESS, poTree->SetPropertyValue(
        fep::g_strElementHeaderPath_strElementDisplayName, "displayname"));
    ASSERT_EQ(a_util::result::SUCCESS, poTree->SetPropertyValue(
        fep::g_strElementHeaderPath_strElementDescription, "description"));
    ASSERT_EQ(a_util::result::SUCCESS, poTree->SetPropertyValue(
        fep::g_strElementHeaderPath_strElementVendor, "vendor"));
    ASSERT_EQ(a_util::result::SUCCESS, poTree->SetPropertyValue(
        fep::g_strElementHeaderPath_strElementCompilationDate, "01.02.03"));
    ASSERT_EQ(a_util::result::SUCCESS, poTree->SetPropertyValue(
        fep::g_strElementHeaderPath_fFEPVersion, 0.2));
    ASSERT_EQ(a_util::result::SUCCESS, poTree->SetPropertyValue(
        fep::g_strElementHeaderPath_strElementPlatform, "platform"));
    ASSERT_EQ(a_util::result::SUCCESS, poTree->SetPropertyValue(
        fep::g_strElementHeaderPath_strElementContext, "context"));
    ASSERT_EQ(a_util::result::SUCCESS, poTree->SetPropertyValue(
        fep::g_strElementHeaderPath_fElementContextVersion, 0.3));
    ASSERT_EQ(a_util::result::SUCCESS, poTree->SetPropertyValue(
        fep::g_strElementHeaderPath_strElementHost, "hostname"));
    ASSERT_EQ(a_util::result::SUCCESS, poTree->SetPropertyValue(
        fep::g_strElementHeaderPath_strInstanceID, "UUID"));
    ASSERT_EQ(a_util::result::SUCCESS, poTree->SetPropertyValue(
        fep::g_strElementHeaderPath_strTypeID, "anotherUUID"));

    double fVal = 0;
    std::string strVal;

    // test remote access
    ASSERT_EQ(a_util::result::SUCCESS, oAI.
        GetParticipantVersion(fVal, oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(fVal , 0.1);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.
        GetParticipantDisplayName(strVal, oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ("displayname", strVal);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.
        GetParticipantDescription(strVal, oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ("description", strVal);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.
        GetParticipantVendor(strVal, oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ("vendor", strVal);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.
        GetParticipantCompilationDate(strVal, oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ("01.02.03", strVal);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.
        GetParticipantFEPVersion(fVal, oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_DOUBLE_EQ(fVal , 0.2);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.
        GetParticipantPlatform(strVal, oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ("platform", strVal);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.
        GetParticipantContext(strVal, oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ("context", strVal);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.
        GetParticipantContextVersion(fVal, oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_DOUBLE_EQ(fVal , 0.3);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.
        GetParticipantHostName(strVal, oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ("hostname", strVal);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.
        GetParticipantInstanceID(strVal, oMod.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ("UUID", strVal);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.GetParticipantTypeID(strVal, oMod.GetName(),
        REM_PROP_TIMEOUT));
    ASSERT_EQ("anotherUUID", strVal);

    // test error cases
    // invalid module names
    ASSERT_EQ(oAI.GetParticipantVersion(fVal, "", -1), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.GetParticipantVersion(fVal, "?", 1), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.GetParticipantVersion(fVal, "*", 1), ERR_INVALID_ARG);
    ASSERT_EQ(oAI.GetParticipantVersion(fVal, "", 1), ERR_INVALID_ARG);
    // invalid timeout
    ASSERT_EQ(ERR_INVALID_ARG, oAI.GetParticipantVersion(fVal, oMod.GetName(), -1));
    ASSERT_EQ(ERR_INVALID_ARG, oAI.GetParticipantVersion(fVal, oMod.GetName(), 0));
    // timeout (because of missing module)
    ASSERT_EQ(ERR_TIMEOUT, oAI.GetParticipantVersion(fVal, 
        "SomeNonExistingModuleName", REM_PROP_TIMEOUT));
    // timeout (because of missing property)
    ASSERT_EQ(a_util::result::SUCCESS, oMod.GetPropertyTree()->
        DeleteProperty(fep::g_strElementHeaderPath_fElementVersion));
    ASSERT_EQ(ERR_TIMEOUT, oAI.GetParticipantVersion(fVal, oMod.GetName(), REM_PROP_TIMEOUT));
    // corrupted participant header
    ASSERT_EQ(a_util::result::SUCCESS, oMod.GetPropertyTree()->
        SetPropertyValue(fep::g_strElementHeaderPath_fElementVersion, "anyString"));
    ASSERT_EQ(ERR_INVALID_TYPE, oAI.GetParticipantVersion(fVal, oMod.GetName(), REM_PROP_TIMEOUT));
}
