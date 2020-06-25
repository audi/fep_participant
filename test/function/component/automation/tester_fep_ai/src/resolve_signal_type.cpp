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
* Test Case:   TestResolveSignalType
* Test ID:     1.17
* Test Title:  AI Signal Type Resolving
* Description: Tests retrieving a signal description from a registered signal type.
* Strategy:    A signal description is registered by a module. Now we make local and remote
*              requests to resolve a signal type by the CSR. Error codes are checked by faulty 
*              requests
*              
* Passed If:   see strategy
* Ticket:      -
* Requirement: FEPSDK-1507
*/
#include <gtest/gtest.h>

#include <fep_test_common.h>
#include "helper_functions.h"

/**
 * @req_id "FEPSDK-1507"
 */
TEST(cTesterFepAutomation, TestResolveSignalType)
{
    AutomationInterface oAI;
    cTestBaseModule oModuleA; // registers a signal description

    /* signal type */
    std::string strName_SignalType_1= "tTestResolveSignalType_SignalType_1";
    std::string strName_SignalType_faulty = "tTestResolveSignalType_UnknownSignalType";

    /* create the modules */
    ASSERT_EQ(a_util::result::SUCCESS, oModuleA.Create(
        cModuleOptions("cTesterFepModule_TestResolveSignalType_A")));

    /* register signal type in module A */
    ASSERT_EQ(a_util::result::SUCCESS, oModuleA.GetSignalRegistry()->RegisterSignalDescription(
        RETURN_MEDIA_DESC(strName_SignalType_1.c_str(), "ui8Value", "tUInt8")));
    
    /* prerequisite: CSR is tested */
    /* hence we compare against the description directly retrieved from the CSR */
    char const * strSignalDescriptionCSR = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oModuleA.GetSignalRegistry()->ResolveSignalType(
        strName_SignalType_1.c_str(),strSignalDescriptionCSR));
    ASSERT_TRUE(NULL != strSignalDescriptionCSR);

    std::string strSignalDescription;

    /* invalid module names */
    // empty module name
    ASSERT_EQ(ERR_INVALID_ARG, oAI.ResolveSignalType(strName_SignalType_1.c_str(),
        strSignalDescription,"",REM_PROP_TIMEOUT));       
    // wild card '*'
    ASSERT_EQ(ERR_INVALID_ARG, oAI.ResolveSignalType(
        strName_SignalType_1, strSignalDescription, "*", REM_PROP_TIMEOUT));          
    // wild card '?'
    ASSERT_EQ(ERR_INVALID_ARG, oAI.ResolveSignalType(
        strName_SignalType_1.c_str(),strSignalDescription, "?", REM_PROP_TIMEOUT));          
    /* invalid timeouts */
    ASSERT_EQ(ERR_INVALID_ARG, oAI.ResolveSignalType(strName_SignalType_1,
        strSignalDescription, oModuleA.GetName(), 0));
    ASSERT_EQ(ERR_INVALID_ARG, oAI.ResolveSignalType(strName_SignalType_1,
        strSignalDescription, oModuleA.GetName(), -1));
    /* timeout */
    ASSERT_EQ(ERR_TIMEOUT, oAI.ResolveSignalType(strName_SignalType_1,
        strSignalDescription, "nonexisting_element", REM_PROP_TIMEOUT));
    /* empty type */
    ASSERT_NE(a_util::result::SUCCESS, oAI.ResolveSignalType(strSignalDescription,
        strSignalDescription, oModuleA.GetName(), REM_PROP_TIMEOUT));
    /* unknown signal type */
    ASSERT_EQ(ERR_NOT_FOUND, oAI.ResolveSignalType(strName_SignalType_faulty, 
        strSignalDescription, oModuleA.GetName(), REM_PROP_TIMEOUT));

    /* valid call */
    ASSERT_TRUE(strSignalDescription.empty());
    ASSERT_EQ(a_util::result::SUCCESS, oAI.ResolveSignalType(strName_SignalType_1,
        strSignalDescription, oModuleA.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(strSignalDescriptionCSR, strSignalDescription);
}