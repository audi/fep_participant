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

#ifndef _FEP_LOCKED_QUEUE_
#define _FEP_LOCKED_QUEUE_

#include <queue>
#include <a_util/concurrency/fast_mutex.h>

namespace fep
{

    namespace ext
    {

        /// Template class of a waitable queue.
        /// The specialization using a std::queue is the class cWaitableQueue
        template <typename T, class QUEUE, class GUARD_MUTEX> class cLockedQueueAdaptor
        {
        private:
            QUEUE m_queue;                      ///< The internal queue
            GUARD_MUTEX    m_lock;              ///< Guarding mutex

        public:
            /// CTOR
            cLockedQueueAdaptor() : m_queue(), m_lock() { }
            /// DTOR
            ~cLockedQueueAdaptor() { }

        public:
            /// Push an element at end of queue and notify consumer
            /// @param [in] t Element to add to the queue
            void Enqueue(const T& t) 
            {
                m_lock.lock();
                m_queue.push(t);
                m_lock.unlock();
            }

            /// Try to copy the first element and remove it from the queue
            /// @param [out] t The first element of the queue, if present
            /// @retval true Element found
            /// @retval false Queue is empty
            bool TryDequeue(T& t) 
            {
                m_lock.lock();
                if (m_queue.empty())
                {
                    m_lock.unlock();
                    return false;
                }
                t= m_queue.front();
                m_queue.pop();
                m_lock.unlock();
                return true;
            }

            /// Try to copy the first element and remove it from the queue.
            /// Unlock guard, when queue is empty.
            /// @param [out] t The first element of the queue, if present
            /// @param [in] guard Mutex to guard the queue
            /// @retval true Element found
            /// @retval false Queue is empty
            template <class GUARD> bool TryDequeueAndUnlockGuardIfEmpty(T& t, GUARD& guard)
            {
                m_lock.lock();
                if (m_queue.empty())
                {
                    m_lock.unlock();
                    guard.unlock();
                    return false;
                }
                t= m_queue.front();
                m_queue.pop();
                m_lock.unlock();
                return true;
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

    /// Template class of a waitable queue.
    /// The specialization using a std::queue is the class cWaitableQueue
    template <typename T, class QUEUE> class cUnlockedQueueAdaptor
    {
    private:
        QUEUE m_queue;                      ///< The internal queue

    public:
        /// CTOR
        cUnlockedQueueAdaptor() : m_queue() { }
        /// DTOR
        ~cUnlockedQueueAdaptor() { }

    public:
        /// Push an element at end of queue and notify consumer
        /// @param [in] t Element to add to the queue
        void Enqueue(const T& t)
        {
            m_queue.push(t);
        }

        /// Try to copy the first element and remove it from the queue
        /// @param [out] t The first element of the queue, if present
        /// @retval true Element found
        /// @retval false Queue is empty
        bool TryDequeue(T& t)
        {
            if (m_queue.empty())
            {
                return false;
            }
            t= m_queue.front();
            m_queue.pop();
            return true;
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

    /// Locked Queue Template
    template <typename T, class Alloc=std::allocator<T> >
        class cLockedQueue : public ext::cLockedQueueAdaptor<T,std::queue<T,std::deque<T,Alloc> >, a_util::concurrency::fast_mutex > { };
    /// Unlocked Queue Template
    template <typename T, class Alloc=std::allocator<T> >
        class cUnlockedQueue : public ext::cUnlockedQueueAdaptor<T,std::queue<T,std::deque<T,Alloc> > > { };

} // namespace fep

#endif // _FEP_LOCKED_QUEUE_
