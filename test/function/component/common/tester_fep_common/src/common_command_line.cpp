/**
* Implementation of the tester for the FEP Common Functions and Classes
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
* Test Case:   TestProgramOptions
* Test ID:     1.9
* Test Title:  Test implementation of FEP Program Options
* Description: Test class cProgramOptions.
* Strategy:    Configure the class cProgramOptions and check if options are parsed correctly.
*              
* Passed If:   no errors occur
* Ticket:      #38657
* Requirement: FEPSDK-1714
*/

// no min/max macros on windows
#define NOMINMAX

#include <iostream>
#include <cstdarg>
#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include "fep_test_common.h"
#include "tester_fep_common_helper.h"
using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

#define MAX_ARGS_FOR_TEST 128

// be carefull with these strings! If you make any changes to this test you need to adjust also
// this reference string, specially care about auto format!
// tabs are here necessary and allowed!
static const char* strHelpTextOutput = R"(usage:
  progname  options

where options are:
  -?, -h, --help                  display usage information
  -v, --version                   Print FEP SDK version and exit.
  -t, --transmission <string>     Set transmission driver used. Available
                                  options are: RTI_DDS (Default) and ZMQ.
  -d, --domain <integer>          Set domain to be used. Valid values are 0 .
                                  . 232. Default is 0.
  -i, --interface <stringlist>    List of network interfaces to be used.
                                  Default is all.
  -n, --name <string>             Set participant name.
  -sl, --stringlist               HilfeText! StringListe!
  -str, --string                  HilfeText! String!
  -fl, --float                    HilfeText! Float!
  -num, --integer                 HilfeText! String!
  -b, --balu                      HilfeText! Hilfe Balu!

Examples of valid option styles are: 
	progname --name=AnyValidParticipantName 	 or
	progname --name AnyValidParticipantName 	 or
	progname --name:AnyValidParticipantName 	 or
	progname -n=AnyValidParticipantName 		 or
	progname -n AnyValidParticipantName 		 or
	progname -n:AnyValidParticipantName
)";

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

static void DumpCommandLine(int argc, const char** argv)
{
    for (int i= 0; i< argc; ++i)
    {
        std::cerr << "argv[" << i << "]= \"" << argv[i] << "\"" << std::endl;
    }
}

static void ClearCommandLine(int& argc, const char**& argv)
{
    if (argv)
    {
        free(argv);
        argv= NULL;
    }
}

/**
 * @req_id "FEPSDK-1714"
 */
TEST(cTesterFepCommon, TestCommandLineOptions)
{
    // Test 01: Overall Test ... parse different command line formats
    {
        cCommandLine oCmdLine;

        const char** argv; int argc;
        CreateCommandLine(argc, argv, "progname",
            "--domain=42",
            "--name shenlong",
            "--stringlist:blub3,blub4",
            "--string bla",
            "--float:0.123456789012345",
            "--integer=12",
            "--balu",
            "--help"
            , NULL);

        std::string strStringList = "";
        oCmdLine.SetAdditionalOption(strStringList, "-sl", "--stringlist", "HilfeText! StringListe!");
        std::string strBla = "";
        oCmdLine.SetAdditionalOption(strBla, "-str", "--string", "HilfeText! String!");
        std::string strDouble = "";
        oCmdLine.SetAdditionalOption(strDouble, "-fl", "--float", "HilfeText! Float!");
        std::string strInt = "";
        oCmdLine.SetAdditionalOption(strInt, "-num", "--integer", "HilfeText! String!");
        bool bFlagTest = false;
        oCmdLine.SetAdditionalOption(bFlagTest, "-b", "--balu", "HilfeText! Hilfe Balu!");
        // Now parse the command line
        ASSERT_EQ(a_util::result::SUCCESS, oCmdLine.ParseArgs(argc, argv));
        // Have all options been parsed
        // check --stringlist
        ASSERT_EQ(strStringList, "blub3,blub4");
        // check -str
        ASSERT_EQ(strBla, "bla");
        // check --float
        ASSERT_EQ(strDouble, "0.123456789012345");
        // check -num
        ASSERT_EQ(strInt, "12");
        // check --help
        ASSERT_EQ(oCmdLine.IsHelpRequested(), true);
        // check --domain
        ASSERT_EQ(oCmdLine.GetDomain(), "42");
        // check --name
        ASSERT_EQ(oCmdLine.GetParticipantName(), "shenlong");
        // check executable name
        ASSERT_EQ(oCmdLine.GetExeName(), "progname");
        // check that nobody asked for a version
        ASSERT_EQ(oCmdLine.IsVersionRequested(), false);


        // Check help output
        // create ostringstream and read buffer to catch output
        std::ostringstream oss;
        std::streambuf* p_cout_streambuf = std::cout.rdbuf();
        std::cout.rdbuf(oss.rdbuf());

        oCmdLine.PrintHelp();

        std::cout.rdbuf(p_cout_streambuf); // restore

        // check help content
        std::string strTempContent = oss.str();
#ifndef __QNX__
        ASSERT_EQ(strTempContent, strHelpTextOutput);
#else
        // fep_module_options.cpp s_nMaxDomainId actually is 232, which is also provided by fep_commandline.cpp
        // TODO fix strHelpTextOutput above if this is not only valid for __QNX__
        std::string str(strHelpTextOutput);
        str.replace(str.find(". 120"), 5, ". 232");
        ASSERT_EQ(strTempContent, str);
#endif

        std::cout << oss.str(); // release for debugging purpose

        // clear command line
        ClearCommandLine(argc, argv);
    }
    // Test 02: Check also other style types and short type and version
    {
        cCommandLine oCmdLine;

        const char** argv; int argc;
        CreateCommandLine(argc, argv, "progname",
            "-d 42",
            "-n:shenlong",
            "-sl=blub3,blub4",
            "-str:bla",
            "-fl 0.123456789012345",
            "-num 12",
            "-b",
            "-v" // short for --version
            , NULL);

        std::string strStringList = "";
        oCmdLine.SetAdditionalOption(strStringList, "-sl", "--stringlist", "HilfeText! StringListe!");
        std::string strBla = "";
        oCmdLine.SetAdditionalOption(strBla, "-str", "--string", "HilfeText! String!");
        std::string strDouble = "";
        oCmdLine.SetAdditionalOption(strDouble, "-fl", "--float", "HilfeText! Float!");
        std::string strInt = "";
        oCmdLine.SetAdditionalOption(strInt, "-num", "--integer", "HilfeText! String!");
        bool bFlagTest = false;
        oCmdLine.SetAdditionalOption(bFlagTest, "-b", "--balu", "HilfeText! Hilfe Balu!");

        // Now parse the command line
        ASSERT_EQ(a_util::result::SUCCESS, oCmdLine.ParseArgs(argc, argv));

        // Have all options been parsed
        // check --stringlist
        ASSERT_EQ(strStringList, "blub3,blub4");
        // check -str
        ASSERT_EQ(strBla, "bla");
        // check --float
        ASSERT_EQ(strDouble, "0.123456789012345");
        // check -num
        ASSERT_EQ(strInt, "12");
        // check --help
        ASSERT_EQ(oCmdLine.IsVersionRequested(), true);
        // check --domain
        ASSERT_EQ(oCmdLine.GetDomain(), "42");
        // check --name
        ASSERT_EQ(oCmdLine.GetParticipantName(), "shenlong");
        // check executable name
        ASSERT_EQ(oCmdLine.GetExeName(), "progname");
        // check that nobody asked for a version
        ASSERT_EQ(oCmdLine.IsHelpRequested(), false);

        // version output can't be tested here, this will be set by module_options 
        // and is also done there

        ClearCommandLine(argc, argv);
    }
}
