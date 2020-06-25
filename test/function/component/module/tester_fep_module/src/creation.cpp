/**
 * Implementation of the tester for the FEP Module
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
 * Test Case:   TestCreation
 * Test ID:     1.1
 * Test Title:  Test the creation of cModule
 * Description: 
 * Strategy:    Test several concurrent cModule Create calls and check for correct error codes
 * Passed If:   Function calls respond with no error or with the provoked error code
 * Ticket:      
 * Requirement: FEPSDK-1541 FEPSDK-1542 FEPSDK-1674 FEPSDK-1675
 */
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
#include "transmission_adapter/RTI_DDS/fep_dds_driver.h"

class cConcurrentInit
{
public:
    cTestBaseModule & m_oMod;
    bool m_bCreate;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
    bool m_bErrorsOccured;

    cConcurrentInit(cTestBaseModule& oMod):
      m_oMod(oMod), m_bCreate(true), m_bErrorsOccured(false)
    {
    }

    void ThreadFunc()
    {
        for (int i = 1; i <= 3 && !m_bErrorsOccured; ++i)
        {
            if (m_bCreate)
            {
                m_bErrorsOccured = m_oMod.Create(cModuleOptions("TestModule")) != ERR_RESOURCE_IN_USE;
            }
            else
            {
                m_bErrorsOccured = m_oMod.Destroy() != ERR_NOT_INITIALISED;
            }
        }
    }

    fep::Result Create()
    {
        m_pThread.reset(new a_util::concurrency::thread(&cConcurrentInit::ThreadFunc, this));
        return ERR_NOERROR;
    }

    void Join()
    {
        m_pThread->join();
    }
};

/**
 * @req_id "FEPSDK-1541 FEPSDK-1542 FEPSDK-1674 FEPSDK-1675"
 */
TEST(TesterFepModule, TestCreation)
{

    cTestBaseModule oMod;
    ASSERT_TRUE(ERR_NOERROR == oMod.Create(cModuleOptions("TestModule")));
    // test second create call
    ASSERT_TRUE(oMod.Create(cModuleOptions("TestModule")) == ERR_RESOURCE_IN_USE);

    ASSERT_TRUE(ERR_NOERROR == oMod.Destroy());
    // test second destroy call
    ASSERT_TRUE(oMod.Destroy() == ERR_NOT_INITIALISED);

    // test create after successful destroy
    ASSERT_TRUE(ERR_NOERROR == oMod.Create(cModuleOptions("TestModule")));
    ASSERT_TRUE(ERR_NOERROR == oMod.Destroy());

    // test create with external dds driver
    RTI_DDS::cDDSDriver oAdap(42);
    ASSERT_TRUE(ERR_NOERROR == oMod.Create(cModuleOptions("TestModule"), &oAdap));
    ASSERT_TRUE(oMod.Create(cModuleOptions("TestModule"), &oAdap) == ERR_RESOURCE_IN_USE);

    // test concurrent creates
    cConcurrentInit o1(oMod);
    cConcurrentInit o2(oMod);
    cConcurrentInit o3(oMod);
    ASSERT_TRUE(ERR_NOERROR == o1.Create());
    ASSERT_TRUE(ERR_NOERROR == o2.Create());
    ASSERT_TRUE(ERR_NOERROR == o3.Create());
    o1.Join();
    o2.Join();
    o3.Join();
    ASSERT_FALSE(o1.m_bErrorsOccured);
    ASSERT_FALSE(o2.m_bErrorsOccured);
    ASSERT_FALSE(o3.m_bErrorsOccured);

    // test concurrent destroys
    ASSERT_TRUE(ERR_NOERROR == oMod.Destroy());
    o1.m_bCreate = false;
    o2.m_bCreate = false;
    o3.m_bCreate = false;
    ASSERT_TRUE(ERR_NOERROR == o1.Create());
    ASSERT_TRUE(ERR_NOERROR == o2.Create());
    ASSERT_TRUE(ERR_NOERROR == o3.Create());
    o1.Join();
    o2.Join();
    o3.Join();
    ASSERT_FALSE(o1.m_bErrorsOccured);
    ASSERT_FALSE(o2.m_bErrorsOccured);
    ASSERT_FALSE(o3.m_bErrorsOccured);
}
