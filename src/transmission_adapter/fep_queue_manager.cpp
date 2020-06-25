/**
 * Implementation of the Class cManager.
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

#include <cstddef>
#include <memory>
#include <thread>
#include <a_util/concurrency/semaphore.h>
#include <a_util/result/result_type.h>

#include "transmission_adapter/fep_receiver.h"
#include "fep_errors.h"
#include "fep_queue_manager.h"
#include "fep_result_decl.h"

namespace fep
{
    class cQueueWorker
    {
        friend class cQueueManager;

    public:
        cQueueWorker(cQueueManager* pFepQueueManager);

    public:
        void ThreadFunc();

    private:
        cQueueManager* m_pFepQueueManager;
        a_util::concurrency::semaphore m_oShutdown;
        a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
    };

cQueueManager::cQueueManager() :
    m_qJobQueue(),
    m_vecWorkerThreads()
{
}

cQueueManager::~cQueueManager()
{
    Destroy();
}

fep::Result cQueueManager::Create(int32_t nWorkerThreads)
{
    m_bJobQueueActive= true;

    for(int32_t i = 0; i < nWorkerThreads; i++)
    {
        cQueueWorker* pFepQueueWorker = new cQueueWorker(this);
        m_vecWorkerThreads.push_back(pFepQueueWorker);
    }

    return ERR_NOERROR;
}

fep::Result cQueueManager::Destroy()
{
    // Signal all threads shutdown
    for(std::vector<cQueueWorker*>::iterator it = m_vecWorkerThreads.begin(); it != m_vecWorkerThreads.end(); ++it)
    {
        cQueueWorker* pFepWorker = (*it);
        pFepWorker->m_oShutdown.notify();
    }

    // Join and delete all threads
    for(std::vector<cQueueWorker*>::iterator it = m_vecWorkerThreads.begin(); it != m_vecWorkerThreads.end(); ++it)
    {
        cQueueWorker* pFepWorker = (*it);
        pFepWorker->m_pThread->join();
        delete pFepWorker;
    }

    m_vecWorkerThreads.clear();

    m_bJobQueueActive= false;
    cDataReceiver* pDataReceiver;
    while(m_qJobQueue.TryDequeue(pDataReceiver, (1000*100)))
    {
        //we just want to clear the queue
    }
    return ERR_NOERROR;
}

void cQueueManager::EnqueueJob(cDataReceiver* pDataReceiver)
{
    if (m_bJobQueueActive)
    {
        m_qJobQueue.Enqueue(pDataReceiver);
    }
}

fep::Result cQueueManager::DoWork()
{
    cDataReceiver* pDataReceiver = NULL;
    if(true == m_qJobQueue.TryDequeue(pDataReceiver, (1000 * 100)))
    {
        pDataReceiver->DoJob();
    }

    return ERR_NOERROR;
}

cQueueWorker::cQueueWorker(cQueueManager* pFeQueueManager)
    : m_pFepQueueManager(pFeQueueManager), m_oShutdown()
{
    m_pThread.reset(new a_util::concurrency::thread(&cQueueWorker::ThreadFunc, this));
}

void cQueueWorker::ThreadFunc()
{
    fep::Result nResult= ERR_NOERROR;
    while (fep::isOk(nResult) && !m_oShutdown.is_set())
    {
        nResult= m_pFepQueueManager->DoWork();
    }
}

}
