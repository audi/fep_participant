/**
 * Implementation of fast semaphore
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

#ifndef __FAST_SEMAPHORE_H__
#define __FAST_SEMAPHORE_H__

#include <cstddef>

#include "_common/fast/spinlock.h"
#include "_common/fast/condition_variable.h"

// API modelled like http://www.boost.org/doc/libs/1_64_0/doc/html/boost/interprocess/interprocess_semaphore.html

namespace fep
{
    namespace fast
    {
/// @cond nodoc
        class semaphore
        {
            typedef spinlock LOCK;
            typedef lock_guard<LOCK> LOCK_GUARD;
            
        public:
            semaphore(const std::size_t available = 0)
                : m_lock()
                , m_condition_variable()
                , m_value(available)
            {
            }

        public:
            void post()
            {
                LOCK_GUARD locker(m_lock);
                if (m_value == 0)
                {
                    ++m_value;
                    m_condition_variable.notify_one();
                }
                else
                {
                    ++m_value;
                }
            }

            // Extension
            void post_n(const std::size_t i)
            {
                // assert(i >= 0);
                LOCK_GUARD locker(m_lock);
                if (m_value == 0)
                {
                    m_value += i;
                    m_condition_variable.notify_all();
                }
                else
                {
                    m_value += i;
                }
            }

            // Extension
            std::size_t value()
            {
                LOCK_GUARD locker(m_lock);
                return m_value;
            }

            void wait()
            {
                LOCK_GUARD locker(m_lock);
                while (m_value == 0)
                {
                    m_condition_variable.wait(m_lock);
                }
                // assert(m_value > 0);
                --m_value;
            }

            bool try_wait()
            {
                LOCK_GUARD locker(m_lock);
                if (m_value == 0)
                {
                    return false;
                }
                // assert(m_value > 0);
                --m_value;
                return true;
            }

            bool timed_wait(const timestamp_t& timestamp)
            {
                LOCK_GUARD locker(m_lock);
                if (m_value == 0)
                {
                    m_condition_variable.wait_for(m_lock, timestamp);
                    if (m_value == 0)
                    {
                        return false;
                    }
                }
                // assert(m_value > 0);
                --m_value;
                return true;
            }

        private:
            LOCK m_lock;
            condition_variable m_condition_variable;
            std::size_t m_value;
        };
/// @endcond
    }
}

#endif // #define __FAST_SEMAPHORE_H__