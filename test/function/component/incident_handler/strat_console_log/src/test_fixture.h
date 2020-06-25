/**
 * Implementation of the test fixture for the tester for the FEP Console Log Strategy
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
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include <fep_test_common.h>
#include <fep_ih_test_common.h>

#include <fstream>
#include <iostream>

using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

static std::streambuf* pStreambufStdout= NULL;
static std::streambuf* pStreambufStderr= NULL;
static std::ofstream oRedirectedStdout;
static std::ofstream oRedirectedStderr;

static void RedirectStdoutToFile(const char* strTestLogFile)
{
    // Redirect console output to file
    fflush(stdout);
    if (!pStreambufStdout)
    {
        pStreambufStdout = std::cout.rdbuf();
    }
    oRedirectedStdout.open(strTestLogFile);
    std::cout.rdbuf(oRedirectedStdout.rdbuf());
}

static void RedirectToStdout()
{
    // Sleeping a second to allow the async console log (RT compliant) to dequeue the invoked incidents.
    a_util::system::sleepMilliseconds(1000);

    std::cout.rdbuf(pStreambufStdout);
    oRedirectedStdout.close();
}

static void RedirectStderrToFile(const char* strTestLogFile)
{
    // Redirect console output to file
    fflush(stderr);

    if (!pStreambufStderr)
    {
        pStreambufStderr = std::cerr.rdbuf();
    }
    oRedirectedStderr.open(strTestLogFile);
    std::cerr.rdbuf(oRedirectedStderr.rdbuf());
}

static void RedirectToStderr()
{
    // Sleeping a second to allow the async console log (RT compliant) to dequeue the invoked incidents.
    a_util::system::sleepMilliseconds(1000);

    std::cerr.rdbuf(pStreambufStderr);
    oRedirectedStderr.close();
}

//############################### Test Setup goes here ###################################
class TestFixtureConsoleStrat : public ::testing::Test
{
protected:
    cTestBaseModule* m_pTestModule;

    void SetUp()
    {
        m_pTestModule = new cTestBaseModule();
        ASSERT_OR_THROW(NULL == m_pTestModule->GetIncidentHandler());
        ASSERT_RESULT_OR_THROW(m_pTestModule->Create(cModuleOptions( strTestModuleName.c_str())));
        ASSERT_OR_THROW(NULL != m_pTestModule->GetIncidentHandler());
    }

    void TearDown()
    {
        m_pTestModule->Destroy();
        if (m_pTestModule)
        {
            delete m_pTestModule;
            m_pTestModule = NULL;
        }
    }
};