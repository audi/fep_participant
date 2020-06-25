/**
 * Implementation of the tester for the FEP Module (Option handling)
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
 * Test Case:   TestModuleCreationWithOptions
 * Test ID:     1.2
 * Test Title:  Test FEP Module Creation using Option
 * Description: Create FEP Module with various options
 * Strategy:    Test several cModule create calls and check for correct error codes
 * Passed If:   no errors occur
 * Ticket:      #38657
 * Requirement: FEPSDK-1713 FEPSDK-1714 FEPSDK-1715 FEPSDK-1716
 */

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
#include "_common/fep_networkaddr.h"
#include <cstdarg>
using namespace fep;

/* Helper functions to create command line
 */
#define MAX_ARGS_FOR_TEST 128
static void CreateCommandLine(int& argc, const char**& argv, const char* cmd, ...)
{
    argc= 0;
    argv= reinterpret_cast<const char**>(malloc(MAX_ARGS_FOR_TEST * sizeof(char*)));

 
    va_list args;
    va_start(args, cmd);

    const char* value= cmd;
    while (value)
    {
        argv[argc++]= value;
        value= va_arg(args, const char*);
    }
    va_end(args);

    argv[argc]= NULL;
}

static void ClearCommandLine(int& argc, const char**& argv)
{
    if (argv)
    {
        free(argv);
        argv= NULL;
    }
    argc= 0;
}

/**
 * @req_id "FEPSDK-1713 FEPSDK-1714 FEPSDK-1715 FEPSDK-1716"
 */
TEST(TesterFepModule, TestModuleCreationWithOptions)
{
    cModuleOptions oModuleOptions;
    cModule oTestModule;
 
    // Test 1: Can not create without a name
    ASSERT_FALSE(ERR_NOERROR == oTestModule.Create(oModuleOptions));
    
    // Test 2: Simple create
    ASSERT_TRUE(ERR_NOERROR == oTestModule.Create("ElementName"));
    ASSERT_TRUE(ERR_NOERROR == oTestModule.Destroy());

    // Test 4: Using configured name
    std::string strElementName("ElementName");
    oModuleOptions.SetParticipantName(strElementName.c_str());
    ASSERT_TRUE(ERR_NOERROR == oTestModule.Create(oModuleOptions));
    ASSERT_TRUE(strElementName == oTestModule.GetName());
    ASSERT_TRUE(ERR_NOERROR == oTestModule.Destroy());

    // Test 5: Using domain id an parsing
    const char** argv; int argc;
    CreateCommandLine(argc, argv, "progname"
            , "--name", "OtherName"
            , "--domain", "107"
            , NULL);
    ASSERT_TRUE(ERR_NOERROR == oModuleOptions.ParseCommandLine(argc, argv));
    ASSERT_TRUE(oModuleOptions.GetDomainId() == 107);
    ASSERT_TRUE(std::string(oModuleOptions.GetParticipantName()) == "OtherName");
    ASSERT_TRUE(ERR_NOERROR == oTestModule.Create(oModuleOptions));
    ASSERT_TRUE(std::string(oTestModule.GetName()) == "OtherName");
    ASSERT_TRUE(oTestModule.GetDomainId() == 107);
    ASSERT_TRUE(ERR_NOERROR == oTestModule.Destroy());
}
