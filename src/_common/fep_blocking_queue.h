/**
 * Declaration of the template class cBlockingQueue.
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

#if !defined(_FEP_BLOCKING_QUEUE_INCLUDED)
#define _FEP_BLOCKING_QUEUE_INCLUDED

#include <atomic>   //std::atomic<int32_t>

namespace fep
{
    /// class template for a growing queue that blocks on dequeue if it's empty.
    /// Also supports cancelling to make it possible to shutdown the Reader thread
    /// Note: Multiple producers, single consumer!
    template<typename T>
    class cBlockingQueue
    {
        /// the actual queue
        fep::cLockedQueue<T> m_oData;

        /// the event used to block Dequeue
        a_util::concurrency::semaphore m_oEvent;

        /// flag to signal cancellation
        volatile bool m_bCancelled;

        /// current size of queue
#ifdef __QNX__
        std::atomic_int_fast32_t m_nSize;
#else
        std::atomic<int32_t> m_nSize;
#endif

    public:
        /// typedef for the element type
        typedef T TypeElement;

    public:
        /// CTOR
        cBlockingQueue() : m_oData(), m_oEvent(), m_bCancelled(false), m_nSize(0)
        {
        }

        /**
         * Enqueues an item into the queue
         * @param [in] data The item
         * @retval ERR_NOERROR Everything went fine
         */
        fep::Result Enqueue(const T & data)
        {
            m_oData.Enqueue(data);
            ++m_nSize;
            m_oEvent.notify();

            return ERR_NOERROR;
        }

        /**
         * Dequeues an item from the queue
         * \note This method will block until either interrupted by \c CancelDequeue
         * or a new item being enqueued into the queue.
         * 
         * @param [out] data The item destination
         * @retval ERR_NOERRROR Everything went fine, data contains the dequeued item
         * @retval ERR_CANCELLED The call was cancelled by \c CancelDequeue
         */
        fep::Result Dequeue(T & data)
        {
            // this is to reset the cancelled flag if no Dequeue was blocked when
            // CancelDequeue was called. To ensure that a worker thread can be stopped
            // return ERR_CANCELLED here too
            if (m_bCancelled)
            {
                m_bCancelled = false;
                return ERR_CANCELLED;
            }

            bool bRes = m_oData.TryDequeue(data);
            // while here to handle invalid wakeups from wait()
            while(!bRes)
            {
                m_oEvent.wait();

                if (m_bCancelled)
                {
                    m_bCancelled = false;
                    return ERR_CANCELLED;
                }

                bRes = m_oData.TryDequeue(data);
            }

            --m_nSize;

            return ERR_NOERROR;
        }

        /**
         * Cancels a pending \c Dequeue operation
         * \note: If no Dequeue call is active, the next call to \c Dequeue will
         * return ERR_CANCELLED. This is to make sure that a worker thread can be
         * cancelled in all cases.
         * @return Standard result code
         */
        fep::Result CancelDequeue()
        {
            m_bCancelled = true;
            m_oEvent.notify();
            return ERR_NOERROR;
            // We dont reset the cancelled flag here because we dont know
            // if a reader was blocked in a Dequeue at the time.
            // ERR_CANCELLED is usually used to break from a worker thread though,
            // hence we have to make sure that exactly once ERR_CANCELLED is returned.
        }

        /**
         * Clears the queue
         */
        void Clear()
        {
            while (true)
            {
                T dummy;
                if (m_oData.TryDequeue(dummy))
                {
                    --m_nSize;
                }
                else
                {
                    break;
                }
            }
        }

        /**
         * Gets the current size of the queue
         * @retval The current size
         */
        int32_t GetSize() const
        {
            return m_nSize;
        }

        /// DTOR
        ~cBlockingQueue()
        {
            CancelDequeue();
        }
    };
} // namespace fep
#endif // !defined(_FEP_BLOCKING_QUEUE_INCLUDED)
