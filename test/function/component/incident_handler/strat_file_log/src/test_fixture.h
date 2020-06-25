/**
 * Implementation of testfixture for the tester for the FEP Incident File Log Strategy
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

using namespace fep;


#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

//############################### Test Setup goes here ###################################
class TestFixtureFileStrat : public ::testing::Test
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