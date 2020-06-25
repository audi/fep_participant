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

#ifndef _ADTF_TIMER_SCHEDULER_CLASS_HEADER_
#define _ADTF_TIMER_SCHEDULER_CLASS_HEADER_

#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <future>
#include <a_util/base/types.h>
#include <a_util/result/result_type.h>

#include "fep_result_decl.h"
#include "fep3/components/clock/clock_service_intf.h"
#include "fep3/components/scheduler/scheduler_service_intf.h"

namespace fep
{
namespace detail
{

class ITimer
{
    public:
        virtual fep::Result WakeUp(timestamp_t wakeup_time, std::promise<void>* pFinished = nullptr) = 0;
        virtual fep::Result Reset() = 0;
};

class cTimerScheduler : public IClock::IEventSink,
                        public fep::IScheduler::IJob
{
    private:
        struct tTimerInfo
        {
            ITimer*     pTimer;
            timestamp_t tmNextInstant;
            timestamp_t tmPeriod;

            bool operator<(const tTimerInfo& sOther) const
            {
                return tmNextInstant < sOther.tmNextInstant;
            }
        };

        std::list<tTimerInfo> m_oTimers;
        std::mutex m_oTimerLock;
        std::mutex m_oTimerProcessingLock;

        std::mutex m_oProcessingTriggerMutex;
        std::condition_variable m_oProcessingTriggerEvent;
        timestamp_t             m_StartUpResetTime;
#ifndef __QNX__
        std::atomic<bool> m_bCancelled;
        std::atomic<bool> m_bStarted;
#else
        std::atomic_bool  m_bCancelled;
        std::atomic_bool  m_bStarted;
#endif

        IClockService* _clock;

        cTimerScheduler() {}
    public:
        explicit cTimerScheduler(IClockService& clock);
        virtual ~cTimerScheduler();

        fep::Result AddTimer(ITimer& oTimer, timestamp_t nPeriod, timestamp_t nInitialDelay);
        fep::Result RemoveTimer(ITimer& oTimer);
        fep::Result Start();
        fep::Result Stop();

    private:
        void ProcessSchedulerQueueSynchron(timestamp_t tmCurrent, timestamp_t& tmTimeToWait);
        void ProcessSchedulerQueueAsynchron(timestamp_t tmCurrent, timestamp_t& tmTimeToWait);
        timestamp_t GetTime() const;
        IClock::ClockType GetClockType() const;

    private: //IClock::IEventSink
        void timeUpdateBegin(timestamp_t old_time, timestamp_t new_time) override;
        void timeUpdating(timestamp_t new_time) override;
        void timeUpdateEnd(timestamp_t new_time) override;
        void timeResetBegin(timestamp_t old_time, timestamp_t new_time) override;
        void timeResetEnd(timestamp_t new_time) override;

    private:
        fep::Result executeDataIn(timestamp_t time_of_execution) override { return fep::Result(); }
        fep::Result execute(timestamp_t time_of_execution) override;
        fep::Result executeDataOut(timestamp_t time_of_execution) override { return fep::Result(); }
};

}
}

#endif
