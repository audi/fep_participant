/**
 *
 * Kernel Timer scheduler
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

#include <a_util/result/error_def.h>
#include <stddef.h>
#include <chrono>
#include <mutex>
#include <thread>
#ifdef __QNX__
#include <sched.h>
#endif

#include "fep_errors.h"
#include "timer_scheduler_impl.h"

namespace fep
{
namespace detail
{

cTimerScheduler::cTimerScheduler(IClockService& clock) : _clock(&clock)
{
    m_bCancelled = false;
    m_bStarted = false;
    m_StartUpResetTime = -1;
    _clock->registerEventSink(*this);
}

cTimerScheduler::~cTimerScheduler()
{
    Stop();
    _clock->unregisterEventSink(*this);
}


fep::Result cTimerScheduler::AddTimer(ITimer& oTimer, timestamp_t nPeriod, timestamp_t nInitialDelay)
{
    std::unique_lock<std::mutex> oLock(m_oTimerLock);
    m_oTimers.push_back({&oTimer, GetTime() + nInitialDelay, nPeriod});
    m_oProcessingTriggerEvent.notify_all();
    return fep::Result();
}

fep::Result cTimerScheduler::RemoveTimer(ITimer& oTimer)
{
    std::unique_lock<std::mutex> oLock(m_oTimerLock);
    for (auto itTimerInfo = m_oTimers.begin();
         itTimerInfo != m_oTimers.end(); ++itTimerInfo)
    {
        if (itTimerInfo->pTimer == &oTimer)
        {
            m_oTimers.erase(itTimerInfo);
            return fep::Result();
        }
    }

    RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "Timer not found");
}

fep::Result cTimerScheduler::Start()
{
    if (m_StartUpResetTime > -1 && GetClockType() == IClock::discrete)
    {
        timestamp_t tmTimeToWait = -1;
        // process all scheduler items synchronously NOW the first step after reset!
        ProcessSchedulerQueueSynchron(m_StartUpResetTime, tmTimeToWait);
    }
    m_bStarted = true;
    return {};
}

fep::Result cTimerScheduler::Stop()
{
    m_bCancelled = true;
    m_bStarted = false;
    m_StartUpResetTime = -1;
    m_oProcessingTriggerEvent.notify_all();
    return fep::Result();
}

void cTimerScheduler::ProcessSchedulerQueueSynchron(timestamp_t tmCurrent, timestamp_t& tmTimeToWait)
{
    // ATTENTION: This is the old implementation of ProcessSchedulerQueue for the synchronous case.
    // If you have to change anything in this method have also a look at the asynchron version!!!

    tmTimeToWait = -1;

    // the scheduler thread or the OnTimeUpdate method must process the queue exclusivly.
    std::lock_guard<std::mutex> oProcessingLock(m_oTimerProcessingLock);

    // because the asynchronous version does not sort the list we must do this first
    {
        std::lock_guard<std::mutex> oLock(m_oTimerLock);

        // have a look at the operator implementation in the header
        m_oTimers.sort();
    }

    while (true)
    {
        std::unique_lock<std::mutex> oLock(m_oTimerLock);

        auto itTimer = m_oTimers.begin();
        if (itTimer == m_oTimers.end())
        {
            break; // while
        }

        if (itTimer->tmNextInstant != 0 &&
            itTimer->tmNextInstant > tmCurrent)
        {
            tmTimeToWait = itTimer->tmNextInstant - tmCurrent;
            break; // while
        }

        tTimerInfo sTimerInfo = *itTimer;   // copy timer info
        //we remember the simulated time step 
        timestamp_t current_time_for_call = sTimerInfo.tmNextInstant;

        if (itTimer->tmPeriod != 0)
        {
            // if the scheduler item has a period time, we have to
            // reinsert with a new timestamp
            itTimer->tmNextInstant += itTimer->tmPeriod;
            // don't resynchronize with the clock because
            // WE MUST CALL ALL THREADLOOPS of the item
            // maybe the item will resynchronize it self

            // iterate over the other items to find the next execution slot
            auto itOther = m_oTimers.begin();
            for (; itOther != m_oTimers.end(); ++itOther)
            {
                if(itOther->tmNextInstant <= tmCurrent)
                {
                    // skip tasks which are delayed on the planned execution time
                    // to give them a chance to work (e.g. OneShotTimer). See #22389
                    // for more information
                    continue;
                }
                if (itTimer->tmNextInstant < itOther->tmNextInstant)
                {
                    // break if the eventtime is smaller than the eventtime of the next item
                    break;
                }
            }

            if (itOther != itTimer)
            {
                // insert the scheduleritem at the found place
                m_oTimers.splice(itOther, m_oTimers, itTimer);
            }

        }
        else
        {
            // erase the scheduler item from list (OneShotTimer)
            m_oTimers.erase(itTimer);
        }

        std::promise<void> oFinished;
        sTimerInfo.pTimer->WakeUp(current_time_for_call, &oFinished);
        oLock.unlock();
        // in this case we have to wait until the timer has finished processing
        oFinished.get_future().wait();
    }
}

void cTimerScheduler::ProcessSchedulerQueueAsynchron(timestamp_t tmCurrent, timestamp_t& tmTimeToWait)
{
    // ATTENTION: This is the new implementation of ProcessSchedulerQueue for the asynchronous case.
    // If you have to change anything in this method have also a look at the synchron version!!!

    tmTimeToWait = -1;

    // the scheduler thread or the OnTimeUpdate method must process the queue exclusivly.
    std::lock_guard<std::mutex> oProcessingLock(m_oTimerProcessingLock);

    bool bLoopAgain = true;
    timestamp_t tmMinWaitTime = -1;
    // the value for max loop count is 1000 because assuming that one loop takes at least
    // 1µs (I think it takes longer time), than the resync with the current time will be
    // performed every 1ms which is needed to guarantee that other (new) timers will be scheduled.
    // see #23270 for more information
    const size_t nMaxLoopCount = 1000;
    size_t nLoopCnt = 0;
    while (bLoopAgain && nLoopCnt < nMaxLoopCount)
    {
        std::lock_guard<std::mutex> oLock(m_oTimerLock);

        bLoopAgain = false;
        for (auto itTimer = m_oTimers.begin(); itTimer != m_oTimers.end();
            /*increment on iterator will be done within the loop*/)
        {
            // variable to detect necessary deletion
            bool bDelete = false;

            // check the eventtimes for timeout calculation
            if (itTimer->tmNextInstant != 0 &&
                itTimer->tmNextInstant > tmCurrent)
            {
                // diff time is always greater than 0
                // remember the shortest time to wait
                timestamp_t tmDiffTime = itTimer->tmNextInstant - tmCurrent;
                if (tmMinWaitTime < 0 || tmDiffTime < tmMinWaitTime)
                {
                    tmMinWaitTime = tmDiffTime;
                }
                else
                {
                    // do nothing. the last remembered time was smaller.
                }

            }
            // check if the eventtime is smaller than the current time
            else if(itTimer->tmNextInstant <= tmCurrent)
            {
                // the item must be triggered

                // wakeup the thread
                itTimer->pTimer->WakeUp(tmCurrent);

                if (itTimer->tmPeriod <= 0)
                {
                    // OneShotTimer: delete from list
                    bDelete = true;
                }
                else
                {
                    // periodic timer: increment the event time by period time
                    itTimer->tmNextInstant += itTimer->tmPeriod;
                    // if at least one period timer was found in the list, we have to loop again
                    // because it is possible that the timer has to be triggered again
                    bLoopAgain = true;
                }
            }

            if (bDelete)
            {
                // erase the scheduler item from list
                // and get the next valid iterator
                itTimer = m_oTimers.erase(itTimer);
            }
            else
            {
                // increment iterator
                ++itTimer;
            }

        }

        tmTimeToWait = tmMinWaitTime;
        ++nLoopCnt;
    }

    // check if the loop ends because the max loop count was reached
    if (nLoopCnt >= nMaxLoopCount)
    {
        // in that case there are already some loops which have to be
        // executed. So we set the wait time to 0 (causes just execution yield)
        // and get the new current time from outside.
        tmTimeToWait = 0;
    }
}

IClock::ClockType cTimerScheduler::GetClockType() const
{
    return _clock->getType();
}

timestamp_t cTimerScheduler::GetTime() const
{
    return _clock->getTime();
}

fep::Result cTimerScheduler::execute(timestamp_t time_of_execution)
{
    while (!m_bCancelled)
    {
        timestamp_t tmTimeToWait = -1;

        // call the scheduler
        if (GetClockType() == IClock::continuous)
        {
            ProcessSchedulerQueueAsynchron(GetTime(), tmTimeToWait);
        }

        if (tmTimeToWait < 0)
        {
            // wait for the event. The loop within ProcessSchedulerQueue
            // was not executed. Otherwise tmTimeToWait is greater 0.
            while (!m_bCancelled)
            {
                std::unique_lock<std::mutex> oLock(m_oProcessingTriggerMutex);
                auto timeout = m_oProcessingTriggerEvent.wait_for(oLock, std::chrono::milliseconds(300));
                if (timeout == std::cv_status::no_timeout)
                {
                    break;
                }
            }
        }
        else
        {
            if (tmTimeToWait / 1000 == 0)
            {
                // timespan is to short for wait. so just yield the execution.
                std::this_thread::yield();
            }
            else
            {
                // the next execution is more or equal 1ms. Just wait that time.
                std::unique_lock<std::mutex> oLock(m_oProcessingTriggerMutex);
                m_oProcessingTriggerEvent.wait_for(oLock, std::chrono::microseconds(tmTimeToWait));
            }
        }
    }

    return fep::Result();
}

void cTimerScheduler::timeResetBegin(timestamp_t nOldTime, timestamp_t nNewTime)
{
    m_oTimerProcessingLock.lock();
    
    if (!m_bStarted)
    {
        m_StartUpResetTime = nNewTime;
    }

    bool bForward = nOldTime < nNewTime;
    timestamp_t nDiff;
    if (bForward)
    {
        nDiff = nNewTime - nOldTime;
    }
    else
    {
        nDiff = nOldTime - nNewTime;
    }

    {
        std::lock_guard<std::mutex> oLock(m_oTimerLock);
        for (auto oIt = m_oTimers.begin();
             oIt != m_oTimers.end(); ++oIt)
        {
            if (bForward)
            {
                oIt->tmNextInstant += nDiff;
            }
            else
            {
                oIt->pTimer->Reset();
                oIt->tmNextInstant -= nDiff;
            }
        }
    }
    // make sure any ongoing waiting is cancelled
    m_oProcessingTriggerEvent.notify_all();
}

void cTimerScheduler::timeResetEnd(timestamp_t nNewTime)
{
    m_oTimerProcessingLock.unlock();

    if (m_bStarted && GetClockType() == IClock::discrete)
    {
        timestamp_t tmTimeToWait = -1;
        // process all scheduler items synchronously NOW
        ProcessSchedulerQueueSynchron(nNewTime, tmTimeToWait);
    }
    
#ifdef __QNX__
    sched_yield();      // without this test cTimingLegacy::interfaceSystemTime fails on QNX
#endif
}

void cTimerScheduler::timeUpdateBegin(timestamp_t nOldTime, timestamp_t nNewTime)
{
    //Nothing to do here
}

void cTimerScheduler::timeUpdating(timestamp_t nNewTime)
{
    if (m_bStarted)
    {
        timestamp_t tmTimeToWait = -1;
        // process all scheduler items synchronously
        ProcessSchedulerQueueSynchron(nNewTime, tmTimeToWait);
    }
}

void cTimerScheduler::timeUpdateEnd(timestamp_t nNewTime)
{
    //Nothing to do here
}

} //namespace detail
} //namespace fep
