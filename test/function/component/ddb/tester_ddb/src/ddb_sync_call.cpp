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
* Test Case:   TestDDBSyncCallback
* Test ID:     1.6
* Test Title:  Test DDBDeliveryStrategies
* Description: Test whether incomplete frames are being delivered, depending
*              on the chosen DDBDeliveryStrategy
* Strategy:    Blocking the ProcessDDBSync() callback whilst continuously transmitting signal data
*                            
* Passed If:   Sample loss occurrs only when the buffer is full or a sync flag has been received
*              prematurely. Incidents have been issued as expected.
* Ticket:      -
* Requirement: FEPSDK-1475 FEPSDK-1476 FEPSDK-1477
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

// assume the asynchronous update delay to be max 5ms.
#define DDB_ASYNC_UPDATE_DELAY 5


class cDDBSyncListener : public ISyncListener
{
public:
    cDDBSyncListener() :
        m_nSyncCount(0), m_nSyncSampleCount(0),
        m_szFrameSize(0), m_szMaxSize(0), m_szValidCount(0)
    {

    }

    ~cDDBSyncListener()
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oLock(m_oDataUpdateGuard);
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oLock2(m_oSyncLock);
    }

public:
    void Reset()
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oLock(m_oDataUpdateGuard);

        m_nSyncCount = 0;
        m_nSyncSampleCount = 0;
        m_szFrameSize=0;
        m_szMaxSize=0;
        m_szValidCount=0;
        m_lstSamples.clear();
    }

    fep::Result WaitForSync(timestamp_t nTimeout)
    {
        if (!m_oSyncEvent.wait_for(a_util::chrono::microseconds(nTimeout)))
        {
            return ERR_TIMEOUT;
        }
        return ERR_NOERROR;
    }

    fep::Result LockSyncCallback()
    {
        if (!m_oSyncLock.try_lock())
        {
            return ERR_DEVICE_IN_USE;
        }
        return ERR_NOERROR;
    }

    void UnlockSyncCallback()
    {
        m_oSyncLock.unlock();
    }

    fep::Result ValidateSyncCount(const uint32_t nExpSyncCnt, const uint32_t nExpSyncSampleCnt,
                              const uint32_t nExpSyncFrameSize, const uint32_t nRcvBuffSz)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oLock(m_oDataUpdateGuard);
        if ((nExpSyncCnt != m_nSyncCount) || (nRcvBuffSz != m_szMaxSize))
        {
            return ERR_MEMORY;
        }

        if ( (nExpSyncSampleCnt != m_szValidCount)
           &&(nExpSyncFrameSize != m_szFrameSize) )
        {
            return ERR_FAILED;
        }

        return ERR_NOERROR;
    }

    uint32_t CalcBuffDigitSum()
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oLock(m_oDataUpdateGuard);

        uint32_t nDigitCount = 0;

        tConstDataBuffer::const_iterator itSample = m_lstSamples.begin();
        for (;itSample != m_lstSamples.end() && NULL != *itSample; itSample++)
        {
            tTestValue* pTestVal = static_cast<tTestValue*>((*itSample)->GetPtr());
            nDigitCount += pTestVal->a;
            nDigitCount += static_cast<uint32_t>(pTestVal->b);
        }
        return nDigitCount;
    }

    // ISyncListener interface
public:
    fep::Result ProcessDDBSync(const handle_t hSignal,
                           const fep::IDDBFrame& oDDBFrame)
    {
        {
            LOG_INFO("DDB Sync received");
            a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oLock(m_oDataUpdateGuard);
            m_nSyncCount++;

            m_szFrameSize=oDDBFrame.GetFrameSize();
            m_szMaxSize=oDDBFrame.GetMaxSize();
            m_szValidCount=oDDBFrame.GetValidCount();

            m_lstSamples.clear();

            for (uint32_t ui32Idx = 0;ui32Idx< oDDBFrame.GetMaxSize(); ++ui32Idx)
            {
                m_lstSamples.push_back(oDDBFrame.GetSample(ui32Idx));
            }
            m_oSyncEvent.notify();
        }
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oLock(m_oSyncLock);
        return ERR_NOERROR;
    }

public:
    typedef std::list<const fep::IUserDataSample*> tConstDataBuffer;
    tConstDataBuffer m_lstSamples;
    uint32_t m_nSyncCount;
    uint32_t m_nSyncSampleCount;
    size_t m_szFrameSize;
    size_t m_szMaxSize;
    size_t m_szValidCount;
    a_util::concurrency::recursive_mutex m_oSyncLock;
    a_util::concurrency::recursive_mutex m_oDataUpdateGuard;
    a_util::concurrency::semaphore m_oSyncEvent;
};

#define FEP_DDB_SYNC_TIMEOUT static_cast<timestamp_t>(500e3)

/**
 * @req_id "FEPSDK-1475 FEPSDK-1476 FEPSDK-1477"
 */
TEST(cTesterDDB, TestDDBSyncCallback)
{
    // Init
    cTestIncidentHandler oTestIncidentHandler;
    cDDB oDDB(&oTestIncidentHandler);
    cDDBSyncListener oDDBListener;
    cSamplePreparation oSamplePreparation;
    uint16_t nDDBBuffSz = 25;
    uint32_t nTxDigitSum = 0;
    double fRandomSalt = 0;

    // Configure
    // We just need a handle, what ever it is
    IPreparationDataSample * pDataSample = NULL;
    handle_t hSignal = &pDataSample;
    cDataSampleFactory::CreateSample(&pDataSample);
    pDataSample->SetSize(sizeof(tTestValue));
    ASSERT_EQ(a_util::result::SUCCESS, pDataSample->SetSignalHandle(hSignal));

    tTestValue* pSendValue = (tTestValue*)pDataSample->GetPtr();
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.CreateEntry(hSignal, nDDBBuffSz));
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.RegisterSyncListener(&oDDBListener));

    // Sanity test up front!
    ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,true));
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.WaitForSync(FEP_DDB_SYNC_TIMEOUT));
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.ValidateSyncCount(1, 1, 1, nDDBBuffSz));
    ASSERT_NE(a_util::result::SUCCESS, oDDBListener.WaitForSync(FEP_DDB_SYNC_TIMEOUT));
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.ValidateSyncCount(1, 1, 1, nDDBBuffSz));

    // Recall sync flag and try again - this should result in no samples being synced.
    oDDBListener.Reset();
    nTxDigitSum = 0;
    fRandomSalt = static_cast<double>(rand() % 200);
    LOG_INFO(a_util::strings::format("Random Salt: %lf", fRandomSalt).c_str());
    pSendValue->a = 1;
    pSendValue->b = fRandomSalt;
    nTxDigitSum += 1 + static_cast<uint32_t>(fRandomSalt);
    ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,false));
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    ASSERT_NE(a_util::result::SUCCESS, oDDBListener.WaitForSync(FEP_DDB_SYNC_TIMEOUT));
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.ValidateSyncCount(0, 0, 0, 0));

    // fill up the DDB some more (but not completely); then sync it.
    for (int8_t nSampleCnt = 0; nSampleCnt < 10; nSampleCnt++)
    {
        pSendValue->a = nSampleCnt;
        pSendValue->b = fRandomSalt;
        nTxDigitSum += nSampleCnt + static_cast<uint32_t>(fRandomSalt);
        ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,false));
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    }

    ASSERT_NE(a_util::result::SUCCESS, oDDBListener.WaitForSync(FEP_DDB_SYNC_TIMEOUT));
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.ValidateSyncCount(0, 0, 0, 0));

    // sync it (with a total of 12 samples)
    pSendValue->a += 1;
    pSendValue->b = fRandomSalt;
    nTxDigitSum += pSendValue->a;
    nTxDigitSum += static_cast<uint32_t>(fRandomSalt);
    ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,true));
    ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));

    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.WaitForSync(FEP_DDB_SYNC_TIMEOUT));
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.ValidateSyncCount(1,12, 12,nDDBBuffSz));
    ASSERT_EQ(oDDBListener.CalcBuffDigitSum() , nTxDigitSum) << 
                    a_util::strings::format("Digitsum did not match: Exp: %d, Seen: %d",
                                    oDDBListener.CalcBuffDigitSum(),
                                    nTxDigitSum).c_str();

    // Re-fill it again but lock the callback asynchronoulsy!
    oDDBListener.Reset();
    nTxDigitSum = 0;
    fRandomSalt = static_cast<double>(rand() % 200);
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.LockSyncCallback());
    int32_t nTargetSampleCnt = 10;
    for (int8_t nSampleCnt = 1; nSampleCnt <= nTargetSampleCnt; nSampleCnt++)
    {
        pSendValue->a = nSampleCnt;
        pSendValue->b = fRandomSalt;
        nTxDigitSum += nSampleCnt + static_cast<uint32_t>(fRandomSalt);
        ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,nSampleCnt == nTargetSampleCnt));
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    }

    // the first run was successful; but the callback remains locked and does not return.
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.WaitForSync(FEP_DDB_SYNC_TIMEOUT));
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.ValidateSyncCount(1,nTargetSampleCnt,nTargetSampleCnt, nDDBBuffSz));
    ASSERT_EQ(oDDBListener.CalcBuffDigitSum() , nTxDigitSum);

    // triggering another DDB sync.
    uint32_t nPrevDigitSum = nTxDigitSum;
    uint32_t nPrevSampleCnt = nTargetSampleCnt;
    nTargetSampleCnt = 15; // even increasing the target sample cnt
    nTxDigitSum = 0;
    fRandomSalt = static_cast<double>(rand() % 200);
    for (int8_t nSampleCnt = 1; nSampleCnt <= nTargetSampleCnt; nSampleCnt++)
    {
        pSendValue->a = nSampleCnt;
        pSendValue->b = fRandomSalt;
        nTxDigitSum += nSampleCnt + static_cast<uint32_t>(fRandomSalt);
        ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,nSampleCnt == nTargetSampleCnt));
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    }

    ASSERT_NE(a_util::result::SUCCESS, oDDBListener.WaitForSync(FEP_DDB_SYNC_TIMEOUT));

    // stock buffer was empty, so no problem
    ASSERT_EQ(0 , oTestIncidentHandler.GetIncidentCount());

    // in terms of received data; nothing has to have changed...
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.ValidateSyncCount(1, nPrevSampleCnt,nPrevSampleCnt, nDDBBuffSz));
    ASSERT_EQ(oDDBListener.CalcBuffDigitSum() , nPrevDigitSum);

    // ok, this must work multiple times (e.g. not result in a complete deadlock)
    int16_t nRepeat = 100;
    for (int16_t nRetryCnt = 0; nRetryCnt < nRepeat; nRetryCnt++)
    {
        nTxDigitSum = 0;
        fRandomSalt = static_cast<double>(rand()  % 200);
        for (int8_t nSampleCnt = 1; nSampleCnt <= nTargetSampleCnt; nSampleCnt++)
        {
            pSendValue->a = nSampleCnt;
            pSendValue->b = fRandomSalt;
            nTxDigitSum += nSampleCnt + static_cast<uint32_t>(fRandomSalt);
            oSamplePreparation.TransmitData(pDataSample,nSampleCnt == nTargetSampleCnt);
            oDDB.Update(pDataSample);
        }

        oDDBListener.WaitForSync(FEP_DDB_SYNC_TIMEOUT);
    }
    a_util::system::sleepMilliseconds(DDB_ASYNC_UPDATE_DELAY);
    ASSERT_EQ(static_cast<uint32_t>(nRepeat) , oTestIncidentHandler.GetIncidentCount());

    // the received data must not have changed.
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.ValidateSyncCount(1, nPrevSampleCnt,nPrevSampleCnt, nDDBBuffSz));
    ASSERT_EQ(oDDBListener.CalcBuffDigitSum() , nPrevDigitSum);

    // unlock the callback and retry
    oDDBListener.UnlockSyncCallback();
    // hm.. releasing the mutex appears to take a while.
    a_util::system::sleepMilliseconds(50);

    // The read buffer should switch and enter the callback with the most recent data sample
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.WaitForSync(FEP_DDB_SYNC_TIMEOUT));
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.ValidateSyncCount(2, nTargetSampleCnt,nTargetSampleCnt, nDDBBuffSz));
    ASSERT_EQ(oDDBListener.CalcBuffDigitSum() , nTxDigitSum);

    // Afterwards, the DDB works as expected
    nTxDigitSum = 0;
    fRandomSalt = static_cast<double>(rand()  % 200);
    for (int8_t nSampleCnt = 1; nSampleCnt <= nTargetSampleCnt; nSampleCnt++)
    {
        pSendValue->a = nSampleCnt;
        pSendValue->b = fRandomSalt;
        nTxDigitSum += nSampleCnt + static_cast<uint32_t>(fRandomSalt);
        ASSERT_EQ(a_util::result::SUCCESS, oSamplePreparation.TransmitData(pDataSample,nSampleCnt == nTargetSampleCnt));
        ASSERT_EQ(a_util::result::SUCCESS, oDDB.Update(pDataSample));
    }
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.WaitForSync(FEP_DDB_SYNC_TIMEOUT));
    ASSERT_EQ(static_cast<uint32_t>(nRepeat) , oTestIncidentHandler.GetIncidentCount());
    ASSERT_EQ(a_util::result::SUCCESS, oDDBListener.ValidateSyncCount(3, nTargetSampleCnt,nTargetSampleCnt, nDDBBuffSz));
    ASSERT_EQ(oDDBListener.CalcBuffDigitSum() , nTxDigitSum);

    delete pDataSample;
}

