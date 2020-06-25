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
 * Test Case:   TestModuleOptionsClass
 * Test ID:     1.1
 * Test Title:  Test FEP Module Options for Interface selection
 * Description: Create/Copy/Set/Parse options using module options class
 * Strategy:    Test several cModuleOptions calls and check for correct error codes
 * Passed If:   no errors occur
 * Ticket:      #38657
 * Requirement: FEPSDK-1713 FEPSDK-1714 FEPSDK-1715
 */

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
#include "_common/fep_networkaddr.h"
#include <cstdarg>

#ifdef __QNX__
#undef EXPECT_EXIT
#define EXPECT_EXIT(f, rc, msg) \
            ADD_FAILURE() << "EXPECT_EXIT DeathTest not supported on QNX. Not done ...";
#endif

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
 * @req_id "FEPSDK-1713 FEPSDK-1714 FEPSDK-1715"
 */
TEST(TesterFepModule, TestModuleOptionsClass)
{
    // Test 1: Create and copy of cModuleOptions
    cModuleOptions oModuleOptions1;
    oModuleOptions1.SetParticipantName("Hans");
    ASSERT_TRUE(std::string(oModuleOptions1.GetParticipantName()) == "Hans");

    cModuleOptions oModuleOptions2(oModuleOptions1);
    oModuleOptions2.SetParticipantName("Sepp");
    ASSERT_TRUE(std::string(oModuleOptions1.GetParticipantName()) == "Hans");
    ASSERT_TRUE(std::string(oModuleOptions2.GetParticipantName()) == "Sepp");

    oModuleOptions2 = oModuleOptions1;
    oModuleOptions1.SetParticipantName("Kurt");
    ASSERT_TRUE(std::string(oModuleOptions1.GetParticipantName()) == "Kurt");
    ASSERT_TRUE(std::string(oModuleOptions2.GetParticipantName()) == "Hans");

    // Test 2: Parsing simple options
    const char** argv; int argc;
    CreateCommandLine(argc, argv, "progname"
            , "--name", "Bert"
            , NULL);
    ASSERT_TRUE(ERR_NOERROR == oModuleOptions1.ParseCommandLine(argc, argv));
    ASSERT_TRUE(std::string(oModuleOptions1.GetParticipantName()) == "Bert");
    ClearCommandLine(argc, argv);

    // Test 3: Unknown options handling
    CreateCommandLine(argc, argv, "progname"
            , "-other", "X"
            , "--more"
            , "--name", "Josef"
            , "--last", "Forever"
            , NULL);
    ASSERT_TRUE(ERR_INVALID_ARG == oModuleOptions1.ParseCommandLine(argc, argv));
    ClearCommandLine(argc, argv);

    // Parsing domain id
    CreateCommandLine(argc, argv, "progname"
            , "--domain", "107"
            , NULL);
    ASSERT_TRUE(ERR_NOERROR == oModuleOptions1.ParseCommandLine(argc, argv));
    ASSERT_TRUE(oModuleOptions1.GetDomainId() == 107);
    ClearCommandLine(argc, argv);

    // Parsing transmission type
    CreateCommandLine(argc, argv, "progname"
            , "--transmission", "RTI_DDS"
            , NULL);
    ASSERT_TRUE(ERR_NOERROR == oModuleOptions1.ParseCommandLine(argc, argv));
    ASSERT_TRUE(oModuleOptions1.GetTransmissionType() == fep::TT_RTI_DDS);
    ClearCommandLine(argc, argv);

    // Printing help
    CreateCommandLine(argc, argv, "progname"
        , "--help"
        , NULL);
    EXPECT_EXIT(oModuleOptions1.ParseCommandLine(argc, argv), ::testing::ExitedWithCode(0), "");
    ClearCommandLine(argc, argv);

    // Printing version
    CreateCommandLine(argc, argv, "progname"
        , "--version"
        , NULL);
    EXPECT_EXIT(oModuleOptions1.ParseCommandLine(argc, argv), ::testing::ExitedWithCode(0), "");
    ClearCommandLine(argc, argv);
}
