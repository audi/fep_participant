/**
 * Implementation of fast latch
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

#ifndef __FAST_LATCH_H__
#define __FAST_LATCH_H__

#include <cstddef>

#include "_common/fast/spinlock.h"
#include "_common/fast/condition_variable.h"

// API modelled like http://www.boost.org/doc/libs/1_64_0/doc/html/thread/synchronization.html#thread.synchronization.latches

namespace fep
{
    namespace fast
    {
/// @cond nodoc
        class latch
        {
            typedef spinlock LOCK;
            typedef lock_guard<LOCK> LOCK_GUARD;
            
        public:
            latch(const std::size_t initial_value = 0)
                : m_lock()
                , m_condition_variable()
                , m_value(initial_value)
            {
            }

        public:
            void reset(const std::size_t i)
            {
                // assert(i >= 0);
                LOCK_GUARD locker(m_lock);
                m_value= i;
            }

            void count_down()
            {
                LOCK_GUARD locker(m_lock);
                // assert(m_value > 0);
                if (m_value > 0)
                {
                    if (--m_value == 0)
                    {
                        m_condition_variable.notify_all();
                    }
                }
            }

            void count_down_and_wait()
            {
                LOCK_GUARD locker(m_lock);
                // assert(m_value > 0);
                if (m_value > 0)
                {
                    if (--m_value == 0)
                    {
                        m_condition_variable.notify_all();
                    }
                    else
                    {
                        m_condition_variable.wait(m_lock);
                    }
                }
            }

            std::size_t value()
            {
                LOCK_GUARD locker(m_lock);
                return m_value;
            }

            void wait()
            {
                LOCK_GUARD locker(m_lock);
                while (m_value > 0)
                {
                    m_condition_variable.wait(m_lock);
                }
                // assert(m_value == 0);
            }

            bool try_wait()
            {
                LOCK_GUARD locker(m_lock);
                if (m_value > 0)
                {
                    return false;
                }
                // assert(m_value == 0);
                return true;
            }

            bool wait_for(const timestamp_t& timestamp)
            {
                LOCK_GUARD locker(m_lock);
                if (m_value > 0)
                {
                    if (!m_condition_variable.wait_for(m_lock, timestamp))
                    {
                        return false;
                    }
                }
                if (m_value > 0)
                {
                    return false;
                }
                // assert(m_value == 0);
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

#endif // __FAST_LATCH_H__
