/**
 * Declaration of the template class cWaitableQueue.
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

#ifndef _FEP_WAITABLE_QUEUE_
#define _FEP_WAITABLE_QUEUE_

#include <queue>
#include <a_util/concurrency.h>

namespace fep
{

    namespace ext
    {

        /// Template class of a waitable queue.
        /// The specialization using a std::queue is the class cWaitableQueue
        template <typename T, class QUEUE, class CONDITION_VAR, class GUARD_MUTEX> class cWaitableQueueAdaptor
        {
        private:
            QUEUE m_queue;                      ///< The internal queue
            CONDITION_VAR  m_cond;              ///< Condition variable
            GUARD_MUTEX    m_lock;              ///< Guarding mutex

        public:
            /// CTOR
            cWaitableQueueAdaptor() : m_queue(), m_cond(), m_lock() { }
            /// DTOR
            ~cWaitableQueueAdaptor() { }

        public:
            /// Push an element at end of queue and notify consumer
            /// @param [in] t Element to add to the queue
            void Enqueue(const T& t) 
            {
                a_util::concurrency::unique_lock<GUARD_MUTEX> m_guard(m_lock);
                m_queue.push(t);
                m_cond.notify_one();
            }

            /// Try to copy the first element and remove it from the queue
            /// @param [out] t The first element of the queue, if present
            /// @retval true Element found
            /// @retval false Queue is empty
            bool TryDequeue(T& t) 
            {
                a_util::concurrency::unique_lock<GUARD_MUTEX> m_guard(m_lock);
                if (m_queue.empty())
                {
                    return false;
                }
                t= m_queue.front();
                m_queue.pop();
                return true;
            }

            /// Try to copy the first element and remove it from the queue. This methods waits for
            /// the specified timeout before it returns if the the queue is empty
            /// @param [out] t The first element of the queue, if present
            /// @param [in] timeout Wait time
            /// @retval true Element found
            /// @retval false After the timeout, the queue is still empty
            bool TryDequeue(T& t, const timestamp_t& timeout) 
            {
                a_util::concurrency::unique_lock<GUARD_MUTEX> m_guard(m_lock);
                if (m_queue.empty())
                {
                    if (m_cond.wait_for(m_guard, a_util::chrono::microseconds(timeout)) ==
                            a_util::concurrency::cv_status::timeout)
                    {
                        return false;
                    }
                    // Still empty ...
                    if (m_queue.empty())
                    {
                        return false;
                    }
                }
                t= m_queue.front();
                m_queue.pop();
                return true;
            }

            /// Copy the first element and remove it from the queue. This method waits until an
            /// element can be returned.
            /// @param [out] t The first element of the queue, if present
            void Dequeue(T& t) 
            {
                a_util::concurrency::unique_lock<GUARD_MUTEX> m_guard(m_lock);
                while (m_queue.empty())
                {
                    m_cond.wait(m_guard);
                }
                t= m_queue.front();
                m_queue.pop();
            }

            // Disabled, as clear does not delete pointers
            // Clear the queue of all elements
            //void Clear()
            //{
            //    T t;
            //    while (TryDequeue(t))
            //    {
            //    }
            //}
        };

    } // namespace ext

    /// Waitable Queue Template
    template <typename T, class Alloc=std::allocator<T> >
    class cWaitableQueue :
        public ext::cWaitableQueueAdaptor<T,std::queue<T,std::deque<T,Alloc> >,
            a_util::concurrency::condition_variable, a_util::concurrency::mutex > { };

} // namespace fep

#endif // _FEP_WAITABLE_QUEUE_
