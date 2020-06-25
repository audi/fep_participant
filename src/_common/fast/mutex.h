/**
 * Implementation of fast spinlock (using critical section / spinlock)
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

#ifndef __FAST_MUTEX_H__
#define __FAST_MUTEX_H__

#ifdef WIN32
#   include <windows.h>
#else
#   include <pthread.h>
#endif

namespace fep
{
    namespace fast
    {
/// @cond nodoc
#ifdef WIN32
        class recursive_mutex
        {
        public:
            recursive_mutex()
            {
                m_win32_mutex= ::CreateMutexA(NULL, FALSE, NULL);
                // assert(m_win32_mutex != INVALID_HANDLE_VALUE);
            }

            ~recursive_mutex()
            {
                if(m_win32_mutex != INVALID_HANDLE_VALUE)
                {
                    ::CloseHandle(m_win32_mutex);
                }
            }

        private:
#if __cplusplus >= 201103L
            recursive_mutex(const recursive_mutex&) = delete;
            recursive_mutex& operator=(const recursive_mutex&)= delete;
#else
            recursive_mutex(const recursive_mutex&) { }
            recursive_mutex& operator=(const recursive_mutex&) { return *this; }
#endif

        public:
            void lock()
            {
                while (WaitForSingleObject(m_win32_mutex, INFINITE) != WAIT_OBJECT_0)
                {
                }
            }

            bool try_lock()
            {
                return (WaitForSingleObject(m_win32_mutex, 0) == WAIT_OBJECT_0);
            }

            void unlock()
            {
                ::ReleaseMutex(m_win32_mutex);
            }

        private:
            ::HANDLE m_win32_mutex;
        };
#else
        class condition_variable;
        class recursive_mutex
        {
            friend class condition_variable;

        public:
            recursive_mutex()
            {
                pthread_mutexattr_t pthread_mutex_attr;
                int res;

                res= pthread_mutexattr_init(&pthread_mutex_attr);
                // assert(res >= 0);

                // Make it recursive
                res= pthread_mutexattr_settype(&pthread_mutex_attr, PTHREAD_MUTEX_RECURSIVE);
                // assert(res >= 0);
            
                // Mutexes are only used by the current process.
                res= pthread_mutexattr_setpshared(&pthread_mutex_attr, PTHREAD_PROCESS_PRIVATE);
                // assert(res >= 0);

                res=pthread_mutex_init(&m_pthread_mutex, &pthread_mutex_attr);
                // assert(res >= 0);

                res= pthread_mutexattr_destroy(&pthread_mutex_attr);
                // assert(res >= 0);

                (void) res;
            }

            ~recursive_mutex()
            {
                pthread_mutex_destroy(&m_pthread_mutex);
            }

        private:
#if __cplusplus >= 201103L
            recursive_mutex(const recursive_mutex&) = delete;
            recursive_mutex& operator=(const recursive_mutex&)= delete;
#else
            recursive_mutex(const recursive_mutex&) { }
            recursive_mutex& operator=(const recursive_mutex&) { return *this; }
#endif


        public:
            void lock()
            {
                int res= pthread_mutex_lock(&m_pthread_mutex);
                // assert(res >= 0);
                (void) res;
            }

            bool try_lock()
            {
                int res= ::pthread_mutex_trylock(&m_pthread_mutex);
                return res == 0;
            }

            void unlock()
            {
                int res= pthread_mutex_unlock(&m_pthread_mutex);
                // assert(res >= 0);
                (void) res;
            }

        private:
            pthread_mutex_t m_pthread_mutex;
        };
#endif


        template<typename MUTEX> class lock_guard
        {
        public:
            explicit lock_guard(MUTEX& any_spinlock) : m_spinlock(any_spinlock)
            {
                m_spinlock.lock();
            }

            ~lock_guard()
            {
                m_spinlock.unlock();
            }

        private:
#if __cplusplus >= 201103L
            lock_guard(const lock_guard&)= delete;
            lock_guard& operator=(const lock_guard&)= delete;
#else
            lock_guard(const lock_guard&) { }
            lock_guard& operator=(const lock_guard&) { return *this; }
#endif

        public:
            MUTEX& m_spinlock;
        };
/// @endcond
    }
}


#endif // __FAST_MUTEX_H__

