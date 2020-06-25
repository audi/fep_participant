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
#ifndef __FEP_LOCAL_CLOCK_BASED_SCHEDULER_H
#define __FEP_LOCAL_CLOCK_BASED_SCHEDULER_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <string>
#include <thread>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep3/components/scheduler/job_runtime_check.h"
#include "fep3/components/scheduler/scheduler_service_intf.h"
#include "timer_scheduler_impl.h"

namespace fep
{
class IClockService;
class IIncidentHandler;

namespace detail
{

class cServiceThread
{
private:
    std::thread m_oSystemThread;
    std::promise<void> m_oExited;
    std::future<void> m_oExitedFuture;
    uint32_t m_ui32Flags;
    std::mutex m_oThreadMutex;
    bool m_bThreadCanceled = false;
    //    object_ptr<cServiceThread> m_pThis;

protected:
    std::string m_strName;
    fep::IScheduler::IJob& m_pRunnable;
    IClockService& m_pClock;

public:
    cServiceThread(const char* strName,
                   fep::IScheduler::IJob& job,
                   fep::IClockService& clock,
                   uint32_t ui32Flags);

    virtual ~cServiceThread();
    fep::Result Start();

    cServiceThread(cServiceThread&&) = delete;
    cServiceThread(const cServiceThread&) = delete;

    void operator()();
    virtual fep::Result execute(timestamp_t first_wakeup_time);
    fep::Result Join(timestamp_t nTimeout = -1);
    fep::Result Detach();
    bool IsCurrent() const;
    std::string GetName() const;

protected:
    bool Joinable() const;
};

class cTimerThread : public cServiceThread, public ITimer
{
private:
    timestamp_t m_nPeriod;
    timestamp_t m_nInitialDelay;
    std::mutex m_oManualEventLock;
    std::condition_variable m_oManualEvent;
    volatile bool m_oManualEventOccured = false;
#ifndef __QNX__
    std::atomic<bool> m_bCanceled;
#else
    std::atomic_bool m_bCanceled;
#endif
    std::promise<void>* m_pFinished = nullptr;
    volatile timestamp_t m_tmWakeupTime = -1;
    volatile timestamp_t m_tmLastCallTime = -1;
    std::list<std::string> m_times;
    cTimerScheduler& m_oADTFScheduler;
    JobRuntimeCheck m_oJobRuntimeCheck;

public:
    cTimerThread(const char* strName,
                 fep::IScheduler::IJob& pRunnable,
                 fep::IClockService& clock,
                 timestamp_t nPeriod,
                 timestamp_t nInitialDelay,
                 uint32_t ui32Flags,
                 cTimerScheduler& oScheduler,
                 const JobRuntimeCheck& oJobRuntimeCheck);

    ~cTimerThread();
    fep::Result execute(timestamp_t wakeup_time) override;
    fep::Result WakeUp(timestamp_t wakeup_time, std::promise<void>* pFinished = nullptr) override;
    fep::Result Reset() override;
    fep::Result Stop();
};

class LocalClockBasedScheduler : public IScheduler
{
public:
    LocalClockBasedScheduler(IIncidentHandler& incident_handler, std::function<fep::Result()> set_participant_to_error_state);
    ~LocalClockBasedScheduler() = default;

public:
    const char* getName() const override;

    fep::Result initialize(IClockService& clock, IJobConfiguration& configuration) override;
    fep::Result start() override;
    fep::Result stop() override;
    fep::Result deinitialize() override;

    std::list<IScheduler::JobInfo> getTasks() const override;

private:
    std::unique_ptr<cServiceThread> _scheduler_thread;
    std::unique_ptr<cTimerScheduler> _scheduler_impl;
    std::list<std::shared_ptr<cTimerThread>> _timers;

private:
    IIncidentHandler& _incident_handler;
    std::function<fep::Result()> _set_participant_to_error_state;
};

} // namespace detail
} // namespace fep
#endif //__FEP_LOCAL_CLOCK_BASED_SCHEDULER_H
