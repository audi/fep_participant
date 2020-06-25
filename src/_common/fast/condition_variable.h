/**
 * Implementation of fast condition variable
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

#ifndef __FAST_CRITICAL_SECTION_H__
#define __FAST_CRITICAL_SECTION_H__

#include "_common/fast/spinlock.h"
#include "_common/fast/mutex.h"
#include "a_util/base/types.h"

#ifdef WIN32
#else
#   include <errno.h>
#endif


namespace fep
{
    namespace fast
    {
/// @cond nodoc
#ifdef WIN32
        class condition_variable
        {
        public:
            condition_variable()
            {
                InitializeConditionVariable(&m_win32_cv);
            }

            ~condition_variable()
            {        
            }

        private:
#if __cplusplus >= 201103L
            condition_variable(const condition_variable&) = delete;
            condition_variable& operator=(const condition_variable&)= delete;
#else
            condition_variable(const condition_variable&) { }
            condition_variable& operator=(const condition_variable&) { return *this; }
#endif

        public:
            template<typename lock_type> void wait(lock_type& another_spinlock);

            template<> void wait<spinlock>(spinlock& another_spinlock)
            {
                SleepConditionVariableCS(&m_win32_cv, &another_spinlock.m_win32_cs, INFINITE);
            }

            template<typename lock_type> bool wait_for(lock_type& another_spinlock, const timestamp_t& timestamp);

            template<> bool wait_for<spinlock>(spinlock& another_spinlock, const timestamp_t& timestamp)
            {
                DWORD dwMilliseconds = static_cast<DWORD>(timestamp / 1000);
                if (dwMilliseconds < 0)
                {
                    return true;
                }
                return SleepConditionVariableCS(&m_win32_cv, &another_spinlock.m_win32_cs, dwMilliseconds) != 0;
            }


            void notify_one()
            {
                WakeConditionVariable(&m_win32_cv);
            }

            void notify_all()
            {
                WakeAllConditionVariable(&m_win32_cv);
            }

        private:
            CONDITION_VARIABLE m_win32_cv;
        };
#else
        class condition_variable
        {
            template<typename lock_type> class on_exit_lock
            {
            public:
                on_exit_lock()
                    : m_pMutex(NULL)
                {
                }

                on_exit_lock(lock_type& oMutex, bool bUnlockNow = true)
                    : m_pMutex(&oMutex)
                {
                    if (bUnlockNow)
                    {
                        m_pMutex->unlock();
                    }
                }

                ~on_exit_lock()
                {
                    if (m_pMutex)
                    {
                        m_pMutex->lock();
                    }
                }

                void activate(lock_type& oMutex)
                {
                    //assert(!m_pMutex);
                    m_pMutex= &oMutex;
                    m_pMutex->unlock();
                }

            private:
                lock_type* m_pMutex;
            };

        public:
            condition_variable()
                : m_internal_lock()
            {
                pthread_condattr_t attr;
                pthread_condattr_init(&attr);
                pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);

                int res = pthread_cond_init(&m_pthread_cond, &attr);
                if (res)
                {
                    //throw SystemException("Failed to initialize condidition variable");
                }
            }

            ~condition_variable()
            {
                int res = pthread_cond_destroy(&m_pthread_cond);
                if (res)
                {
                    //throw SystemException("Failed to destroy condition variable");
                }
            }

        private:
#if __cplusplus >= 201103L
            condition_variable(const condition_variable&) = delete;
            condition_variable& operator=(const condition_variable&)= delete;
#else
            condition_variable(const condition_variable&) { }
            condition_variable& operator=(const condition_variable&) { return *this; }
#endif

        public:
            template<typename lock_type> void wait(lock_type& another_spinlock)
            {
                int res = 0;
                {
                    on_exit_lock<lock_type> another_guard;
                    lock_guard<recursive_mutex> internal_guard(m_internal_lock);
                    another_guard.activate(another_spinlock);
                    res = pthread_cond_wait(&m_pthread_cond, &(m_internal_lock.m_pthread_mutex));
                }

                if (res)
                {
                    //throw SystemException("Failed to wait on condition variable");
                }
            }

            template<typename lock_type> bool wait_for(lock_type& another_spinlock, const timestamp_t& timestamp)
            {
                ::timespec until_time;
                ::clock_gettime(CLOCK_MONOTONIC, &until_time);
                until_time.tv_sec+= timestamp / 1000000;
                until_time.tv_nsec+= (timestamp % 1000000) * 1000;
                if (until_time.tv_nsec >= 1000000000)
                {
                    until_time.tv_sec+= 1;
                    until_time.tv_nsec-= 1000000000;
                }

                int res = 0;
                {
                    on_exit_lock<lock_type> another_guard;
                    lock_guard<recursive_mutex> internal_guard(m_internal_lock);
                    another_guard.activate(another_spinlock);
                    res = pthread_cond_timedwait(&m_pthread_cond, &(m_internal_lock.m_pthread_mutex), &until_time);
                }

                if (res)
                {
                    if (res == ETIMEDOUT)
                    {
                        return false;
                    }
                    //throw SystemException("Failed to wait on condition variable");
                }

                return true;

            }

            void notify_one()
            {
                int res;
                lock_guard<recursive_mutex> internal_guard(m_internal_lock);
                res = ::pthread_cond_signal(&m_pthread_cond);
                if (res)
                {
                    //throw SystemException("Failed to notify one on condition variable");
                }
            }

            void notify_all()
            {
                int res;
                lock_guard<recursive_mutex> internal_guard(m_internal_lock);
                res = ::pthread_cond_broadcast(&m_pthread_cond);
                if (res)
                {
                    //throw SystemException("Failed to notify all on condition variable");
                }
            }

        private:
            pthread_cond_t m_pthread_cond;
            recursive_mutex m_internal_lock;
        };
#endif
/// @endcond
    } 
}

#endif // __FAST_CRITICAL_SECTION_H__
