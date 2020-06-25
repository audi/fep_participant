/**
 * Declaration of class cManager.
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
#ifndef _MANAGER_HEADER_
#define _MANAGER_HEADER_ 

#include <cstdint>
#include <vector>

#include "fep_result_decl.h"
#include "_common/fep_waitable_queue.h"

namespace fep
{
    class cDataReceiver;
    class cQueueWorker;

    /**
     * Manage Fep  workers
     *
     * The Fep manager serves as a manager and container for the worker threads.
     * It implements a manager/worker design pattern.
     */
    class cQueueManager
    {
        friend class cQueueWorker;

    public:
        /**
         * CTOR
         *
         */
        cQueueManager();

        /**
         * DTOR
         *
         */
        ~cQueueManager();

        /**
         * This method \c Create will set up all needed internal elements.
         * @param [in] nWorkerThreads The number of worker threads to be created
         *
         * @return Standard result code.
         */
        fep::Result Create(int32_t nWorkerThreads);

        /**
         * This method \c Create will release all internal resources.
         *
         * @return Standard result code.
         */
        fep::Result Destroy();

        /**
         * The method \c EnqueueJob will push a pointer to a cDataReceiver into
         * the JobQueue.
         *
         * @param  [in] pParticipant The pointer to the cDataReceiver
         */
        void EnqueueJob(cDataReceiver* pDataReceiver);

    private:
        /**
         * The function of the receive thread. Will try to dequeue a pointer
         * from the JobQueue and call the \ref cDataReceiver::DoJob 
         * method of the corresponding cDataReceiver.
         *
         * @return Standard result code.
         */
        fep::Result DoWork();

    private:
        /// Flag to determine if enqueue should be enabled
        bool m_bJobQueueActive;
        /// Queue containing the jobs to be done by the worker threads
        cWaitableQueue<cDataReceiver*> m_qJobQueue;
        /// Vector containing the worker threads
        std::vector<cQueueWorker*> m_vecWorkerThreads;
    };
}

#endif
