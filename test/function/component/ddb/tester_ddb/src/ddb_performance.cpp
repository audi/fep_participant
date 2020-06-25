/**
* Implementation of the tester for the FEP Distributed Data Buffer
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
* Test Case:   TestDDBPerformance
* Test ID:     1.9
* Test Title:  Test Buffer Performance
* Description: Test performance of the DDB Buffer. Also good test for leaking memory. 
*              Enable logging to std::cerr to verify data loss during transmission
* Strategy:    Run this test using VLD/Valgrind and check if memory was leaked.
*              
* Passed If:   no errors occur and all memory is freed.
* Ticket:      -
* Requirement: FEPSDK-1587
*/
#include <iostream>
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include "fep_test_common.h"
using namespace fep;

#include "distributed_data_buffer/fep_ddb.h"
#include "distributed_data_buffer/fep_ddb_frame.h"
#include "distributed_data_buffer/fep_ddb_frame_factory.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include "test_fixture.h"
#include "helper_functions.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

//#include <iostream>

class testFillDDB
{
public:
    testFillDDB(cDDB* pDDB) : m_pDDB(pDDB), m_cnt(0)
    {
        m_pDataSample = NULL;
        m_hSignal     = &m_pDataSample;
        cDataSampleFactory::CreateSample(&m_pDataSample);
        m_pDataSample->SetSize(sizeof(tTestValue));
        m_pDataSample->SetSignalHandle(m_hSignal);

        m_pDDB->CreateEntry(m_hSignal, 100);
    }
    ~testFillDDB()
    {
        delete m_pDataSample;
    }
    void Start()
    {
        m_pThread.reset(new a_util::concurrency::thread(&testFillDDB::ThreadFunc, this));
    }
    void Stop()
    {
        m_oShutdown.notify();
        m_pThread->join();
    }

 protected:
     void ThreadFunc()
     {
         while (!m_oShutdown.is_set())
         {
             tTestValue* value= reinterpret_cast<tTestValue*>(m_pDataSample->GetPtr());
             value->a= 'X';
             value->b= m_cnt;
             ++m_cnt;
             m_oSamplePreparation.TransmitData(m_pDataSample,m_cnt % 10 == 0);
             m_pDDB->Update(m_pDataSample);

             a_util::system::sleepMilliseconds(10);
         }
     }

private:
    cDDB* m_pDDB;
    IPreparationDataSample* m_pDataSample;
    handle_t m_hSignal;
    cSamplePreparation m_oSamplePreparation;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
    a_util::concurrency::semaphore m_oShutdown;
public:
    int32_t m_cnt;
};

class testListenDDB : public ISyncListener
{
public:
    testListenDDB() : m_cnt(0)
    {
    }

protected:
    fep::Result ProcessDDBSync(
                handle_t const hSignal,
                const fep::IDDBFrame& oDDBFrame)
    {
        if (oDDBFrame.GetFrameSize())
        {
//            std::cerr << "ProcessDDBSync called with " << oDDBFrame.GetFrameSize() << " samples:" <<  std::endl;
        }

        for (uint32_t ui32Idx = 0; ui32Idx<oDDBFrame.GetFrameSize(); ++ui32Idx)
        {
            const fep::IUserDataSample* pUserData = oDDBFrame.GetSample(ui32Idx);
            if (pUserData) {
                const tTestValue* value= reinterpret_cast<const tTestValue*>(pUserData->GetPtr());
//                std::cerr << "  Received with value a='" << value->a << "' b=" << value->b << " (cnt=" << m_cnt << ")" << std::endl;
                (void) value;
                ++m_cnt;
            }
        }
        return ERR_NOERROR;
    }

public:
    int32_t m_cnt;
};


/**
 * @req_id "FEPSDK-1587"
 */
TEST(cTesterDDB, TestDDBPerformance)
{
     // Init
    cTestIncidentHandler oTestIncidentHandler;
    cDDB oDDB(&oTestIncidentHandler);

    testFillDDB oFillDDB(&oDDB);
    testListenDDB oListenDDB;

    ASSERT_EQ(a_util::result::SUCCESS, oDDB.RegisterSyncListener(&oListenDDB));

    oFillDDB.Start();

    a_util::system::sleepMilliseconds(10 * 1000);

    oFillDDB.Stop();

    oDDB.UnregisterSyncListener(&oListenDDB);

    // We did not receive more than we sent
    ASSERT_FALSE((oListenDDB.m_cnt > oFillDDB.m_cnt));
    // We did not lose more than 2 Frames
    ASSERT_TRUE(20 > (oFillDDB.m_cnt - oListenDDB.m_cnt));

    //std::cerr << "Sent " << oFillDDB.m_cnt << std::endl;
    //std::cerr << "Recv " << oListenDDB.m_cnt << std::endl;
}