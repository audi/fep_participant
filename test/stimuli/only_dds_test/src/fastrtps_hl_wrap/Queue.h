/**

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
 */
#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <chrono>
#include <condition_variable>

namespace fastrtps_hl_wrap
{
    template <typename T> class Queue
    {
    public:
        Queue() : m_queue(), m_cond(), m_lock() { }
        ~Queue()
        {
        }

    public:
        void enqueue(const T& t)
        {
            std::unique_lock<std::mutex> guard(m_lock);
            m_queue.push(t);
            m_cond.notify_one();
        }

        void dequeue(T& t)
        {
            std::unique_lock<std::mutex> guard(m_lock);
            while (m_queue.empty())
            {
                m_cond.wait(guard);
            }
            t = m_queue.front();
            m_queue.pop();
        }

        bool tryDequeue(T& t)
        {
            std::unique_lock<std::mutex> guard(m_lock);
            if (m_queue.empty())
            {
                return false;
           }
            t = m_queue.front();
            m_queue.pop();

            return true;
        }

        template< class Rep, class Period > bool tryDequeueFor(T& t, const std::chrono::duration<Rep, Period>& rel_time)
        {
            std::unique_lock<std::mutex> guard(m_lock);
            while (m_queue.empty())
            {
                if (m_cond.wait_for(guard, rel_time) == std::cv_status::timeout)
                {
                    if (m_queue.empty())
                    {
                        // Nothing dequeued
                        return false;
                    }
                }
            }
            t = m_queue.front();
            m_queue.pop();

            return true;
        }

    private:
        std::queue<T> m_queue;
        std::condition_variable  m_cond;
        std::mutex    m_lock;
    };

}