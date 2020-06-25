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

#ifndef __FAST_SPINLOCK_H__
#define __FAST_SPINLOCK_H__

#ifdef WIN32
#   include <windows.h>
#else
#   include <pthread.h>
#endif

namespace fep
{
    namespace fast
    {
        class condition_variable;

/// @cond nodoc
#ifdef WIN32
        class spinlock
        {
            friend class condition_variable;

        public:
            spinlock()
            {
                ::InitializeCriticalSection(&m_win32_cs);
            }

            ~spinlock()
            {
                ::DeleteCriticalSection(&m_win32_cs);
            }

        private:
#if __cplusplus >= 201103L
            spinlock(const spinlock&) = delete;
            spinlock& operator=(const spinlock&)= delete;
#else
            spinlock(const spinlock&) { }
            spinlock& operator=(const spinlock&) { return *this; }
#endif

        public:
            void lock()
            {
                ::EnterCriticalSection(&m_win32_cs);
            }

            bool try_lock()
            {
                BOOL res= ::TryEnterCriticalSection(&m_win32_cs);
                return (res != 0);
            }

            void unlock()
            {
                ::LeaveCriticalSection(&m_win32_cs);
            }

        private:
            ::CRITICAL_SECTION m_win32_cs;
        };
#else
        class spinlock
        {
        public:
            spinlock()
            {
                int res= ::pthread_spin_init(&m_pthread_spin, PTHREAD_PROCESS_PRIVATE);
                // assert(res == 0);
                (void) res;
            }

            ~spinlock()
            {
                int res= ::pthread_spin_destroy(&m_pthread_spin);
                // assert(res == 0);
                (void) res;
            }

        private:
#if __cplusplus >= 201103L
            spinlock(const spinlock&) = delete;
            spinlock& operator=(const spinlock&)= delete;
#else
            spinlock(const spinlock&) { }
            spinlock& operator=(const spinlock&) { return *this; }
#endif

        public:
            void lock()
            {
#ifdef __QNX__
                int res= pthread_spin_lock(&m_pthread_spin);
#else
                int res= ::pthread_spin_lock(&m_pthread_spin);
#endif
                // assert(res == 0);
                (void) res;
            }

            bool try_lock()
            {
#ifdef __QNX__
                int res= pthread_spin_trylock(&m_pthread_spin);
#else
                int res= ::pthread_spin_trylock(&m_pthread_spin);
#endif
                return (res == 0);
            }

            void unlock()
            {
#ifdef __QNX__
                int res= pthread_spin_unlock(&m_pthread_spin);
#else
                int res= ::pthread_spin_unlock(&m_pthread_spin);
#endif
                // assert(res == 0);
                (void) res;
            }

        private:
            ::pthread_spinlock_t m_pthread_spin;
        };
#endif
/// @endcond
    }
}

#endif // __FAST_SPINLOCK_H__
