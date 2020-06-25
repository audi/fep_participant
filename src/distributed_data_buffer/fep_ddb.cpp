/**
 * Implementation of the Class cDDB.
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

#include <algorithm>
#include <cassert>
#include <list>
#include <utility>
#include <a_util/concurrency/semaphore.h>
#include <a_util/result/result_info_decl.h>
#include <a_util/result/result_type.h>

#include "distributed_data_buffer/fep_ddb.h"
#include "distributed_data_buffer/fep_ddb_frame.h"
#include "distributed_data_buffer/fep_sync_listener_intf.h"
#include "fep_errors.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "transmission_adapter/fep_preparation_data_sample_intf.h"

using namespace fep;

#include "fep_ddb_common.h"

cDDB::cDDB (IIncidentHandler* pIncidentHandler) :
    m_hSignal(0), m_pBufferRead(NULL),m_pBufferStock(NULL),m_pBufferWrite(NULL),
    m_bStockIsFull(false), m_nDDBDeliverStrategy(DDBDS_DeliverIncomplete),
    m_nPrevFrameId(0), m_nPrevSampleNumber(0), m_bPrevSyncFlag(true),
    m_pIncidentHandler(pIncidentHandler), m_nSharedLockCount(0),
    m_oGuardBufferStock(),
    m_oGuardBufferWrite(),
    m_oLockBufferRead(),
    m_oGuardListener(),
    m_oEventFrameReady()
{
   // developer - please dont be as negligent....
   assert(NULL != m_pIncidentHandler);

   m_pBufferRead = new cDDBFrame();
   m_pBufferStock = new cDDBFrame();
   m_pBufferWrite = new cDDBFrame();
}

cDDB::~cDDB ()
{
    for (int32_t i = 0; i < m_nSharedLockCount; ++i)
    {
        m_oLockBufferRead.unlock_shared();
    }

    DeleteMemory();

    delete m_pBufferRead;
    delete m_pBufferStock;
    delete m_pBufferWrite;
}

fep::Result cDDB::CreateEntry (handle_t const hSignal, size_t const szMaxEntries,
    size_t const szSampleSize, fep::tDDBDeliveryStrategy nDDBDeliverStrategy)
{
    // note: not relevant to the incident handler; method has been called
    // by the user directly!
    fep::Result nResult = ERR_NOERROR;
    if (NULL == hSignal || 0 == szMaxEntries)
    {
        nResult = ERR_INVALID_ARG;
    }

    if (fep::isOk(nResult))
    {
        DeleteMemory();

        m_nDDBDeliverStrategy = nDDBDeliverStrategy;

        // Lock Access
        m_oLockBufferRead.lock();
        {
            a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oGuardBufferWrite);
            {
                a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync2(m_oGuardBufferStock);

                m_pBufferRead->InitMemory(szMaxEntries,szSampleSize);
                m_pBufferStock->InitMemory(szMaxEntries,szSampleSize);
                m_pBufferWrite->InitMemory(szMaxEntries,szSampleSize);
                m_hSignal = hSignal;

            }
        }
        m_oLockBufferRead.unlock();

        // create the worker thread
        m_pThread.reset(new std::thread(&cDDB::ThreadFunc, this));
    }
    return nResult;
}

fep::Result cDDB::LockData(const IDDBFrame *&poDDBFrame)
{
    fep::Result nResult = ERR_NOERROR;

    // We lock the access to the read buffer here. It is the user's responsibility to unlock the
    // buffer by calling UnlockData().
    m_oLockBufferRead.lock_shared();
    ++m_nSharedLockCount;

    if (fep::isOk(nResult))
    {
        if ( (NULL != m_pBufferRead)
          && (0 != m_pBufferRead->GetMaxSize())
          && (0 != m_pBufferRead->GetFrameSize()) )
        {
            poDDBFrame = m_pBufferRead;
        }
        else
        {
            poDDBFrame = NULL;
            nResult = ERR_EMPTY;
        }
    }

    return nResult;
}

fep::Result cDDB::UnlockData()
{
    m_oLockBufferRead.unlock_shared();
    --m_nSharedLockCount;
    return ERR_NOERROR;
}

fep::Result cDDB::Update(IPreparationDataSample const * poPreparationSample)
{
    fep::Result nResult = ERR_NOERROR;

    // lock write buffer
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oGuardBufferWrite);

    if (NULL == poPreparationSample)
    {
        nResult = ERR_POINTER;
    }
    else if ( (NULL == m_hSignal) ||
              (0 == m_pBufferWrite->GetMaxSize()) )
    {
        // initialization failed
        NotifyOfIncident(FSI_DDB_NOT_INITIALIZED, SL_Critical_Local,
                         "Receiving data without properly configured DDB!");
        nResult = ERR_INVALID_STATE;
    }
    else if (poPreparationSample->GetSignalHandle() == m_hSignal)
    {
        // check for previously missed sync flag
        // Incidents are issued by the cDataListenerAdapter
        if ( (poPreparationSample->GetFrameId() > m_nPrevFrameId) &&
             (!m_bPrevSyncFlag) )
        {
            m_pBufferWrite->AnalyseFrame();
            SwitchWriteBuffer();
        }

        // store data in entry
        nResult = m_pBufferWrite->SetSample(poPreparationSample,poPreparationSample->GetSampleNumberInFrame());
        if (fep::isFailed(nResult))
        {
            if (ERR_MEMORY == nResult)
            {
                NotifyOfIncident(FSI_DDB_RX_OVERRUN, SL_Critical_Local,
                                 "A received data sample exceeded DDB buffer size. Sample dropped.");
            }
            else
            {
                NotifyOfIncident(FSI_DDB_RX_OVERRUN, SL_Critical_Local,
                                 "Failed to properly enqueue data sample.");
            }
        }

        // check for sync flag, switch buffer
        if (poPreparationSample->GetSyncFlag())
        {
            m_pBufferWrite->AnalyseFrame();
            SwitchWriteBuffer();
        }

        m_nPrevFrameId = poPreparationSample->GetFrameId();
        m_nPrevSampleNumber = poPreparationSample->GetSampleNumberInFrame();
        m_bPrevSyncFlag = poPreparationSample->GetSyncFlag();
    }
    else
    {
        NotifyOfIncident(FSI_GENERAL_WARNING, SL_Warning,
                         "DUMP: A received signal handle does not match the DDBs registered one!");
    }

    // This is a transmission callback; there is no point in returning an error here.
    return ERR_NOERROR;
}

void cDDB::ThreadFunc()
{
    fep::Result nResult = ERR_NOERROR;

    while (!m_oShutdownThread.is_set())
    {
        nResult = ERR_NOERROR;

        m_oEventFrameReady.wait();

        // check again after condition
        if (m_oShutdownThread.is_set())
        {
            break;
        }

        // synchronize with data readers (ProcessDDBSync() callback and GetRecentData())
        // since data reception is asynchronous, we can wait
        m_oLockBufferRead.lock();
        nResult = SwitchReadBuffer();
        m_oLockBufferRead.unlock();

        // deliver to user
        if (fep::isOk(nResult))
        {
            // Lock the list of SyncListeners
            a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oGuardListener);

            // Guard read buffer (concurrent access of several readers possible)
            m_oLockBufferRead.lock_shared();
            for (tSyncListenerList::const_iterator itListener = m_lstListeners.begin();
                m_lstListeners.end() != itListener; ++itListener)
            {
                nResult = (*itListener)->ProcessDDBSync(m_hSignal, *m_pBufferRead);
                if (fep::isFailed(nResult))
                {
                    NotifyOfIncident(FSI_GENERAL_WARNING, SL_Warning,
                                     "ProcessDDBSync: An error was reported by the user!");
                    nResult = ERR_NOERROR;
                }
            }
            m_oLockBufferRead.unlock_shared();
        }
    }
}

void cDDB::ResetData()
{
    // Upon re-entering FS_IDLE, we need to reset all data
    // Lock data access: Callbacks may be active
    m_oLockBufferRead.lock();
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oGuardBufferWrite);
        {
            a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync2(m_oGuardBufferStock);

            m_pBufferRead->InvalidateData();
            m_pBufferStock->InvalidateData();
            m_pBufferWrite->InvalidateData();

            m_nPrevFrameId = 0;
            m_nPrevSampleNumber = 0;
            m_bPrevSyncFlag = true;
        }
    }
    m_oLockBufferRead.unlock();
}

fep::Result cDDB::RegisterSyncListener (ISyncListener * const poListener)
{
    // note: not relevant to the incident handler; method has been called
    // by the user directly!

    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oGuardListener);

    fep::Result nResult = ERR_NOERROR;
    if (NULL == poListener)
    {
        nResult = ERR_POINTER;
    }
    else
    {
        m_lstListeners.push_back(poListener);
    }
    return nResult;
}

fep::Result cDDB::UnregisterSyncListener (ISyncListener * const poListener)
{
    // note: not relevant to the incident handler; method has been called
    // by the user directly!

    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oGuardListener);

    fep::Result nResult = ERR_NOERROR;
    if (NULL == poListener)
    {
        nResult = ERR_POINTER;
    }
    else
    {
        tSyncListenerList::iterator itListener = std::remove(m_lstListeners.begin(),
            m_lstListeners.end(), poListener);
        if (m_lstListeners.end() == itListener)
        {
            nResult = ERR_NOT_FOUND;
        }
        else
        {
            m_lstListeners.erase(itListener);
        }
    }
    return nResult;
}

fep::Result cDDB::SwitchWriteBuffer()
{
    fep::Result nResult = ERR_NOERROR;

    // check delivery strategy:
    if ( (DDBDS_DumpIncomplete == m_nDDBDeliverStrategy)
      && (m_pBufferWrite->GetValidCount() != m_pBufferWrite->GetFrameSize()) )
    {
        NotifyOfIncident(FSI_DDB_RX_ABORT_SYNC, SL_Warning, "Reception abort! The frame is incomplete; Change "
                                         "DDBDeliveryStrategy to DDBDS_DeliverIncomplete to "
                                         "avoid this issue");
        nResult = ERR_OUT_OF_RANGE;
    }

    if (fep::isOk(nResult))
    {
        // synchronized access to StockBuffer
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oGuardBufferStock);

        // the stock buffer is full, so we have to drop a frame
        if ( (m_bStockIsFull) )
        {
            NotifyOfIncident(FSI_DDB_RX_ABORT_SYNC, SL_Warning,
                             "Dropping frame! New sync received while "
                                             "stock buffer is still full.");
            nResult = ERR_RESOURCE_IN_USE;
        }

        std::swap(m_pBufferStock,m_pBufferWrite);
        if (!m_bStockIsFull)
        {
            // notify (only once!)
            m_oEventFrameReady.notify();
        }

        m_bStockIsFull = true;

    }
    return nResult;
}

fep::Result cDDB::SwitchReadBuffer()
{
    fep::Result nResult = ERR_NOERROR;

    // synchronized access to StockBuffer
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oGuardBufferStock);

    if (m_bStockIsFull)
    {
        std::swap(m_pBufferStock,m_pBufferRead);
        m_bStockIsFull = false;
    }
    else
    {
        NotifyOfIncident(FSI_GENERAL_WARNING, SL_Warning,
                         "Failed to switch read buffer: stock buffer is empty");
        nResult = ERR_EMPTY;
    }

    return nResult;
}

fep::Result cDDB::DeleteMemory()
{
    // note: not relevant to the incident handler; method has been called
    // by the user directly!

    // terminate the thread
    if (m_pThread)
    {
        m_oShutdownThread.notify();
        m_bStockIsFull = false;
        m_oEventFrameReady.notify();
        m_pThread->join();
        m_pThread.reset();
        m_oShutdownThread.reset();
    }

    // Lock access and delete memory
    {
        m_oLockBufferRead.lock();
        {
            a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oGuardBufferWrite);
            {
                a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync2(m_oGuardBufferStock);

                m_pBufferRead->DeleteMemory();
                m_pBufferStock->DeleteMemory();
                m_pBufferWrite->DeleteMemory();
            }
        }
        m_oLockBufferRead.unlock();
    }

    return ERR_NOERROR;
}

fep::Result cDDB::NotifyOfIncident(int16_t nIncident, tSeverityLevel severity,
                                 const char* strDescription)
{
    return INVOKE_INCIDENT( m_pIncidentHandler,nIncident, severity, strDescription);
}
