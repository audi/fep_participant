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
* Test Case:   TestGetRecentDataAsync
* Test ID:     1.3
* Test Title:  Test GetRecentData
* Description: Test buffering functionality of the DDB using GetRecentData()
* Strategy:    Data is received and transfered without any problems.
*              
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1464 FEPSDK-1467 FEPSDK-1468 FEPSDK-1469 FEPSDK-1470 FEPSDK-1471 FEPSDK-1472
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

/**
* Class for asynchronously locking and unlocking the DDB.
*/
class cLockUnlockThread
{
public:
    cLockUnlockThread(cDDB* pDDB) :
      m_pDDBRef(pDDB), m_nLockRetVal(ERR_NOERROR), m_oShutdown()
      {
          m_pThread.reset(new a_util::concurrency::thread(&cLockUnlockThread::ThreadFunc, this));
      }

      ~cLockUnlockThread()
      {
          m_oLockUnlockRequest.notify();
          m_oShutdown.notify();
          m_pThread->join();
      }

public:
    fep::Result LockBuffer()
    {
        fep::Result nResult = ERR_NOERROR;
        m_bLocked = true;
        m_oLockUnlockRequest.notify();
        nResult = m_oLockUnlockResponse.wait_for(a_util::chrono::seconds(2));
        if (fep::isOk(nResult))
        {
            nResult = m_nLockRetVal;
        }
        else
        {
            nResult = ERR_TIMEOUT;
        }

        return nResult;
    }

    fep::Result UnlockBuffer()
    {
        fep::Result nResult = ERR_NOERROR;
        m_bLocked = false;
        m_oLockUnlockRequest.notify();
        nResult = m_oLockUnlockResponse.wait_for(a_util::chrono::seconds(2));

        if (fep::isOk(nResult))
        {
            nResult = m_nLockRetVal;
        }
        else
        {
            nResult = ERR_TIMEOUT;
        }

        return nResult;
    }

    // cThread interface
    void ThreadFunc()
    {
        while (!m_oShutdown.is_set())
        {
            m_oLockUnlockRequest.wait();
            if (m_oShutdown.is_set())
            {
                break;
            }

            if (m_bLocked)
            {
                const fep::IDDBFrame* poDDBFrame;
                m_nLockRetVal = m_pDDBRef->LockData(poDDBFrame);
            }
            else
            {
                m_nLockRetVal = m_pDDBRef->UnlockData();
            }
            m_oLockUnlockResponse.notify();
        }
    }

    typedef struct
    {
        // note: without pack this results in 16bytes!
        int8_t a;
        double b;
    } tTestValue;


private:
    cDDB* m_pDDBRef;
    a_util::concurrency::semaphore m_oLockUnlockRequest;
    a_util::concurrency::semaphore m_oLockUnlockResponse;
    bool m_bLocked;
    fep::Result m_nLockRetVal;
    a_util::concurrency::semaphore m_oShutdown;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
};

// assume the asynchronous update delay to be max 5ms.
#define DDB_ASYNC_UPDATE_DELAY 5

/**
 * @req_id "FEPSDK-1464 FEPSDK-1467 FEPSDK-1468 FEPSDK-1469 FEPSDK-1470 FEPSDK-1471 FEPSDK-1472"
 */
TEST(cTesterDDB, TestGetRecentDataAsync)
{

    // Init
    cTestIncidentHandler oTestIncidentHandler;
    cDDB oDDB(&oTestIncidentHandler);
    cSamplePreparation oSamplePreparation;
    cLockUnlockThread oLockUnlockThread(&oDDB);
    // Configure
    // We just need a handle, what ever it is
    IPreparationDataSample * pDataSample = NULL;
    const IDDBFrame* pDDBFrame = NULL;

    handle_t hSignal = &pDataSample;
    cDataSampleFactory::CreateSample(&pDataSample);
    pDataSample->SetSyncFlag(false);
    pDataSample->SetSize(sizeof(tTestValue));
    ASSERT_EQ(a_util::result::SUCCESS, pDataSample->SetSignalHandle(hSignal));
    tTestValue* pSendValue = (tTestValue*)pDataSample->GetPtr();
    pSendValue->a = 1;
    pSendValue->b = 2;
    tTestValue* pReceiveValue = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, pDataSample->SetSyncFlag(true));

    // Test with one entry, sync flag coming always
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.CreateEntry(hSignal, 1));

    // standard procedure..
    ASSERT_EQ(ERR_EMPTY , oDDB.LockData(pDDBFrame));
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());

    // note: the readWriteLock is write-preferring, i.e. the following will produce a dead-lock:
    // T1: ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData());
    // T2: lock buffer for write access (exclusively)
    // T1: ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData());

    // first test - lock buffer before updating.
    // we need to fill the buffer with something
    ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,true));
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);
    ASSERT_EQ(a_util::result::SUCCESS, oLockUnlockThread.LockBuffer());

    // the read buffer is blocked, reception is possible though (stock buffer is empty)
    // and no incident is raised
    ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,true));
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);
    ASSERT_EQ(0 , oTestIncidentHandler.GetIncidentCount());

    // after receiving a second frame, the stock buffer has to be dismissed
    ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,true));
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);
    ASSERT_EQ(1 , oTestIncidentHandler.GetIncidentCount());

    // at this point, Locking the data from this thread would result in a dead-lock:
    // The worker thread  already requires a write lock so that no further read locks can be set

    // after we unlock the buffer, the frames will be switched and we have a valid read frame
    ASSERT_EQ(a_util::result::SUCCESS, oLockUnlockThread.UnlockBuffer());
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    ASSERT_EQ(1 , pDDBFrame->GetFrameSize());
    pReceiveValue = (tTestValue*) (pDDBFrame->GetSample(0)->GetPtr());
    ASSERT_EQ(pReceiveValue->a , pSendValue->a);
    ASSERT_EQ(pReceiveValue->b , pSendValue->b);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());

    // repeat reception of single samples multiple times with proper locking
    int8_t nRepetitionCnt = 100;
    for (int8_t idx = 0; idx < nRepetitionCnt; idx++)
    {
        pSendValue->a = idx;
        pSendValue->b = idx + 2;
        oSamplePreparation.TransmitData(pDataSample,true);
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
        a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
        ASSERT_EQ(1 , pDDBFrame->GetFrameSize());
        pReceiveValue = (tTestValue*) (pDDBFrame->GetSample(0)->GetPtr());
        ASSERT_EQ(pReceiveValue->a , pSendValue->a);
        ASSERT_EQ(pReceiveValue->b , pSendValue->b);
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());
    }

    // Test with two entries, sync flag coming every sample. proper locking in place.
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.CreateEntry(hSignal, 2));
    oSamplePreparation.Reset();
    pDDBFrame = NULL;

    for (int8_t idx = 0; idx < nRepetitionCnt; idx++)
    {
        pSendValue->a = idx;
        pSendValue->b = idx + 2;
        oSamplePreparation.TransmitData(pDataSample,true);
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
        a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));

        ASSERT_EQ(1 , pDDBFrame->GetFrameSize());
        pReceiveValue = (tTestValue*) (pDDBFrame->GetSample(0)->GetPtr());
        ASSERT_EQ(pReceiveValue->a , pSendValue->a);
        ASSERT_EQ(pReceiveValue->b , pSendValue->b);
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());
    }

    // Test with several entries, sync flag coming every second sample
    uint8_t nTargetSampleCnt = 14;
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.CreateEntry(hSignal, nTargetSampleCnt));
    oSamplePreparation.Reset();
    pDDBFrame = NULL;
    for (int8_t idx = 0; idx < nRepetitionCnt; idx++)
    {
        ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,false));
        pSendValue->a = idx;
        pSendValue->b = idx + 2;
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
        ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,true));
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
        a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
        ASSERT_EQ(2 , pDDBFrame->GetFrameSize());
        pReceiveValue = (tTestValue*) (pDDBFrame->GetSample(0)->GetPtr());
        ASSERT_EQ(pReceiveValue->a , pSendValue->a);
        ASSERT_EQ(pReceiveValue->b , pSendValue->b);
        pReceiveValue = (tTestValue*) (pDDBFrame->GetSample(1)->GetPtr());
        ASSERT_EQ(pReceiveValue->a , pSendValue->a);
        ASSERT_EQ(pReceiveValue->b , pSendValue->b);
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());
    }

    // No matter the case; as soon as something has been written into
    // the buffer, LockData() will always return data - regardless of it's age.
    a_util::system::sleepMilliseconds(1000);
    pDDBFrame = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    ASSERT_EQ(2 , pDDBFrame->GetFrameSize());
    ASSERT_EQ(nTargetSampleCnt , static_cast<int8_t>(pDDBFrame->GetMaxSize()));
    pReceiveValue = (tTestValue*) (pDDBFrame->GetSample(0)->GetPtr());
    ASSERT_EQ(pReceiveValue->a , pSendValue->a);
    ASSERT_EQ(pReceiveValue->b , pSendValue->b);
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());

    // Eventually, verify extensive buffer locking for when new samples
    // arrive while having a lock set.
    int8_t nPrevValA = pReceiveValue->a;
    double fPrevValB = pReceiveValue->b;
    pDDBFrame = NULL;
    oTestIncidentHandler.Reset();
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    for (uint8_t ui32Idx = 1; ui32Idx <= nTargetSampleCnt / 2; ui32Idx++)
    {
        pSendValue->a = ui32Idx + 25;
        pSendValue->b = ui32Idx + 50;
        ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,ui32Idx == (nTargetSampleCnt / 2)));
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    }
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    // this should still hold the previous two data samples.
    ASSERT_EQ(2 , pDDBFrame->GetFrameSize());
    ASSERT_EQ(nTargetSampleCnt , static_cast<int8_t>(pDDBFrame->GetMaxSize()));
    pReceiveValue = (tTestValue*) (pDDBFrame->GetSample(0)->GetPtr());
    ASSERT_EQ(pReceiveValue->a , nPrevValA);
    ASSERT_EQ(pReceiveValue->b , fPrevValB);
    pReceiveValue = (tTestValue*) (pDDBFrame->GetSample(1)->GetPtr());
    ASSERT_EQ(pReceiveValue->a , nPrevValA);
    ASSERT_EQ(pReceiveValue->b , fPrevValB);


    // So far, no frames were dropped
    ASSERT_EQ( 0 , oTestIncidentHandler.GetIncidentCount());

    // unlocking the buffer which allows to switch the read buffer;
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    for (int8_t idx = (nTargetSampleCnt / 2) + 1; idx <= nTargetSampleCnt; idx++)
    {
        pSendValue->a = idx + 25;
        pSendValue->b = idx + 50;
        ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,idx == nTargetSampleCnt));
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    }
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    // the incident count still remains zero
    ASSERT_EQ(0 , oTestIncidentHandler.GetIncidentCount());
    // the buffer reports to have a size of nTargetSampleCnt - yet it's not fully used.
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    ASSERT_EQ((nTargetSampleCnt / 2) , pDDBFrame->GetFrameSize());
    uint8_t ui8SampleValBase = (nTargetSampleCnt / 2) + 1;
    for (uint32_t ui32Idx = 0; ui32Idx<pDDBFrame->GetMaxSize(); ++ui32Idx, ++ui8SampleValBase)
    {
        if (ui32Idx < (nTargetSampleCnt / 2u))
        {
            pReceiveValue = (tTestValue*)(pDDBFrame->GetSample(ui32Idx)->GetPtr());
            ASSERT_EQ(pReceiveValue->a , ui8SampleValBase + 25);
            ASSERT_EQ(pReceiveValue->b , ui8SampleValBase + 50);
        }
        else
        {
            ASSERT_TRUE(pDDBFrame->GetSample(ui32Idx) == NULL);
        }
    }
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());

    // Last but not least, repeat this procedure with real threads...
    // first, we reset the DDB
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.CreateEntry(hSignal, nTargetSampleCnt));

    // Need to reset the incident handler since restarting the DDB might raise an incident when
    // stopping the worker thread
    oTestIncidentHandler.Reset();
    oSamplePreparation.Reset();
    pDDBFrame = NULL;

    // Fill read buffer for locks to take effect
    ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,true));
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    cLockUnlockThread oFirstAccessThread(&oDDB);
    cLockUnlockThread oSecondAccessThread(&oDDB);

    ASSERT_EQ(a_util::result::SUCCESS, oFirstAccessThread.LockBuffer());
    ASSERT_EQ(a_util::result::SUCCESS, oSecondAccessThread.LockBuffer());

    for (uint8_t ui8Idx = 1; ui8Idx <= nTargetSampleCnt; ui8Idx++)
    {
        // we dont care for data...
        ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,ui8Idx == nTargetSampleCnt));
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    }

    // two locks in place; still, data is received and stored and will be available as soon as we
    // release the locks.
    ASSERT_EQ(0 , oTestIncidentHandler.GetIncidentCount());

    // releasing the first one, must not unlock the buffer globally!
    ASSERT_EQ(a_util::result::SUCCESS, oFirstAccessThread.UnlockBuffer());

    for (uint8_t ui8Idx = 1; ui8Idx <= nTargetSampleCnt; ui8Idx++)
    {
        // we dont care for data...
        ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,ui8Idx == nTargetSampleCnt));
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    }

    // still not working... the old sample is replaced and an incident is thrown
    ASSERT_EQ(1 , oTestIncidentHandler.GetIncidentCount());

    // now, we're good...
    ASSERT_EQ(a_util::result::SUCCESS, oSecondAccessThread.UnlockBuffer());
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    for (uint8_t ui8Idx = 1; ui8Idx <= nTargetSampleCnt; ui8Idx++)
    {
        // use static data to verify the buffer we receive now is actually new.
        pSendValue->a = ui8Idx;
        pSendValue->b = ui8Idx+2;
        ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,ui8Idx == nTargetSampleCnt));
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    }
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);

    // still one incident
    ASSERT_EQ(1 , oTestIncidentHandler.GetIncidentCount());

    ASSERT_EQ(a_util::result::SUCCESS, oDDB.LockData(pDDBFrame));
    ASSERT_EQ(nTargetSampleCnt , static_cast<int8_t>(pDDBFrame->GetValidCount()));
    // only checking for the last sample is sufficient to detect whether the buffer has
    // been updated at all...
    pReceiveValue = (tTestValue*)(pDDBFrame->GetSample(nTargetSampleCnt-1)->GetPtr());
    ASSERT_EQ(pReceiveValue->a , pSendValue->a);
    ASSERT_EQ(pReceiveValue->b , pReceiveValue->b);

    ASSERT_EQ(a_util::result::SUCCESS, oDDB.UnlockData());
    delete pDataSample;
}