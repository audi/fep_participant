/**
 * Declaration of the Class cStateEventWorker.
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

#if !defined(_FEP_STATE_EVENT_WORKER_INCLUDED)
#define _FEP_STATE_EVENT_WORKER_INCLUDED

#include "a_util/memory.h"
#include "_common/fep_waitable_queue.h"

namespace fep
{
    /// Time to wait for dequeue
    static const int32_t s_nQueueWaitTime = 1000 * 1000;
    /**
     * This class implements the worker thread that executes the events of the state machine.
     */
    template<typename Handler>
    class cStateEventWorker
    {

    private:
        ///Typedef for the event queue type
        typedef fep::cWaitableQueue<Handler> tStateEventQueue;

    private:
        /// The queue
        tStateEventQueue* m_poQueue;
        /// Worker Thread
        a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
        /// Shutdown signal
        a_util::concurrency::semaphore m_oSignal;

    public:
        /// CTOR
        cStateEventWorker()
            : m_poQueue(NULL)
        {

        }
        /// DTOR
        ~cStateEventWorker()
        {
            if (m_poQueue)
            {
                SetEventQueue(NULL);
            }
        }

    public:
        /**
         * The method \ref SetEventQueue sets the event queue for use in the worker and starts the
         * worker thread. In case of a queue change the thread is restarted. In case of a NULL argument
         * the thread is shut down and joined.
         *
         * @param [in] poQueue  the event queue
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_UNEXPECTED  Thread is active while trying to set event queue
         */
        fep::Result SetEventQueue(tStateEventQueue * poQueue)
        {
            fep::Result nRes = ERR_NOERROR;
            if (NULL == poQueue)
            {
                m_oSignal.notify();
                if (m_pThread)
                {
                    m_pThread->join();
                    m_pThread.reset();
                }
                m_poQueue = poQueue;
            }
            else
            {
                if (m_pThread)
                {
                    nRes = ERR_UNEXPECTED;
                }

                if (fep::isOk(nRes))
                {
                    m_poQueue = poQueue;
                    try
                    {
                        m_pThread.reset(new a_util::concurrency::thread(&cStateEventWorker::ThreadFunc, this));
                    }
                    catch (...)
                    {
                        // out of memory, just leave quick
                        exit(0);
                    }
                }
            }

            return nRes;
        }

    private:
        /// The method called by fep::cSystemThread
        void ThreadFunc()
        {
            while (!m_oSignal.is_set())
            {
                Handler oHandler;
                if (m_poQueue->TryDequeue(oHandler, s_nQueueWaitTime))
                {
                    oHandler();
                }
            }
            m_oSignal.reset();
        }
    };
}
#endif // !defined(_FEP_STATE_EVENT_WORKER_INCLUDED)
