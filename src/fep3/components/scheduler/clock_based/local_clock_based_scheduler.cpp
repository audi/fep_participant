/**
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

#include <chrono>
#include <string>
#include <utility>
#include <a_util/result/result_type.h>
#include <a_util/result/error_def.h>

#include "fep3/components/clock/clock_service_intf.h"
#include "fep3/components/scheduler/scheduler_job_config.h"
#include "fep_errors.h"
#include "local_clock_based_scheduler.h"
#include "timer_scheduler_impl.h"

namespace fep
{
class IIncidentHandler;

namespace detail
{

cServiceThread::cServiceThread(const char* strName,
                               fep::IScheduler::IJob& job,
                               fep::IClockService& clock,
                               uint32_t ui32Flags)
    : m_ui32Flags(ui32Flags), m_strName(strName ? strName : ""), m_pRunnable(job), m_pClock(clock)
{
    m_oExitedFuture = m_oExited.get_future();
}

cServiceThread::~cServiceThread()
{
    Join();
}

fep::Result cServiceThread::Start()
{
    fep::Result nResult = ERR_NOERROR;
    {
        std::lock_guard<std::mutex> oLocker(m_oThreadMutex);
        m_oSystemThread = std::thread(std::ref(*this));
        //  nResult = SetScheduling(m_sScheduling);
        if (fep::isFailed(nResult))
        {
            m_bThreadCanceled = true;
        }
    }

    if (fep::isFailed(nResult))
    {
        if (m_oSystemThread.joinable())
        {
            m_oSystemThread.join();
        }
        return nResult;
    }

    return fep::Result();
}

void cServiceThread::operator()()
{
    {
        std::lock_guard<std::mutex> oGuard(m_oThreadMutex);
        if (m_bThreadCanceled)
        {
            m_oExited.set_value();
            return;
        }
    }

    /*
    auto pErrorHandler = create_error_handler(cString::Format("kernel_thread.%s",
    m_strName.GetPtr()), "kernel_thread_error", IErrorHandling::tAction::Log);
    */
    fep::Result nResult = execute(m_pClock.getTime());
    /*
    if (IS_FAILED(nResult))
    {
        pErrorHandler->Handle(ADTF_BASE_COMPOSED_RESULT(nResult,
            "Thread '%s' exited abnormally.",
            m_strName.GetPtr()));
    } */

    {
        std::lock_guard<std::mutex> oGuard(m_oThreadMutex);
        m_oExited.set_value();
    }

    /*   if (m_pThis)
    {
        // we have been detached, so release ourselfs
        m_pThis = nullptr;
    } */
}

fep::Result cServiceThread::execute(timestamp_t wakeup_time)
{
    m_pRunnable.executeDataIn(wakeup_time);
    m_pRunnable.execute(wakeup_time);
    m_pRunnable.executeDataOut(wakeup_time);
    return fep::Result();
}

fep::Result cServiceThread::Join(timestamp_t nTimeout)
{
    if (!m_oSystemThread.joinable())
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "thread not joinable");
    }

    if (nTimeout == -1)
    {
        m_oSystemThread.join();
    }
    else
    {
        if (m_oExitedFuture.wait_for(std::chrono::microseconds(nTimeout)) ==
            std::future_status::ready)
        {
            m_oSystemThread.join();
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_TIMEOUT, "timeout joining thread");
        }
    }

    return fep::Result();
}

fep::Result cServiceThread::Detach()
{
    if (!m_oSystemThread.joinable())
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "thread not joinable");
    }

    std::lock_guard<std::mutex> oLocker(m_oThreadMutex);
    if (m_oExitedFuture.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)
    {
        m_oSystemThread.join();
    }
    else
    {
        // make sure we live long enough
        //    m_pThis = object_ptr_from_this();
        m_oSystemThread.detach();
    }

    return fep::Result();
}

bool cServiceThread::IsCurrent() const
{
    return std::this_thread::get_id() == m_oSystemThread.get_id();
}

std::string cServiceThread::GetName() const
{
    return m_strName;
}

bool cServiceThread::Joinable() const
{
    return m_oSystemThread.joinable();
}

cTimerThread::cTimerThread(const char* strName,
                           fep::IScheduler::IJob& pRunnable,
                           fep::IClockService& clock,
                           timestamp_t nPeriod,
                           timestamp_t nInitialDelay,
                           uint32_t ui32Flags,
                           cTimerScheduler& oScheduler,
                           const JobRuntimeCheck& oJobRuntimeCheck)
    : cServiceThread(strName, pRunnable, clock, 0),
      m_nPeriod(nPeriod),
      m_nInitialDelay(nInitialDelay),
      m_bCanceled(false),
      m_oADTFScheduler(oScheduler),
      m_oJobRuntimeCheck(oJobRuntimeCheck)
{
}

cTimerThread::~cTimerThread()
{
    Stop();
}

fep::Result cTimerThread::execute(timestamp_t first_wakeup_time)
{
    while (!m_bCanceled)
    {
        std::unique_lock<std::mutex> oLock(m_oManualEventLock, std::defer_lock);

        {
            oLock.lock();
            m_oManualEvent.wait(oLock, [this] { return m_oManualEventOccured; });
            m_oManualEventOccured = false;
        }

        if (m_bCanceled)
        {
            break;
        }
        if (m_tmLastCallTime == -1
            || m_tmWakeupTime > m_tmLastCallTime)
        {
            RETURN_IF_FAILED(m_oJobRuntimeCheck.runJob(m_tmWakeupTime, m_pRunnable));
            m_tmLastCallTime = m_tmWakeupTime;
        }

        if (m_pFinished)
        {
            m_pFinished->set_value();
            m_pFinished = nullptr;
        }
    }
    return fep::Result();
}

fep::Result cTimerThread::WakeUp(timestamp_t wakeup_time, std::promise<void>* pFinished)
{
    std::lock_guard<std::mutex> oLock(m_oManualEventLock);
    m_pFinished = pFinished;
    m_tmWakeupTime = wakeup_time;
    m_oManualEventOccured = true;
    m_oManualEvent.notify_all();
    return fep::Result();
}

fep::Result cTimerThread::Reset()
{
    m_tmWakeupTime = -1;
    m_tmLastCallTime = -1;
    return {};
}

fep::Result cTimerThread::Stop()
{
    if (Joinable())
    {
        {
            m_oADTFScheduler.RemoveTimer(*this);
        }

        m_bCanceled = true;
        if (!IsCurrent())
        {
            WakeUp(m_pClock.getTime());
            RETURN_IF_FAILED(Join());
        }
        else
        {
            return Detach();
        }
    }

    return fep::Result();
}

LocalClockBasedScheduler::LocalClockBasedScheduler(IIncidentHandler& incident_handler, std::function<fep::Result()> set_participant_to_error_state)
    : _incident_handler(incident_handler),
      _set_participant_to_error_state(set_participant_to_error_state)
{
}

const char* LocalClockBasedScheduler::getName() const
{
    return FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED;
}

fep::Result LocalClockBasedScheduler::initialize(IClockService& clock,
                                                 IJobConfiguration& configuration)
{
    auto job_configurations = configuration.getJobConfig();
    _scheduler_impl.reset(new cTimerScheduler(clock));
    _scheduler_thread.reset(new cServiceThread("__scheduler", *_scheduler_impl.get(), clock, 0));

    for (auto& job : job_configurations)
    {
        IScheduler::JobInfo& job_config = job.second;

        JobRuntimeCheck job_runtime_check(std::string(job_config.getName()),
                job_config.getConfig()._runtime_violation_strategy,
                                              job_config.getConfig()._max_runtime_real_time_us,
                                              _incident_handler,
                                              _set_participant_to_error_state);

        auto new_timer = std::make_shared<cTimerThread>(job_config.getName(),
                                                        *job.first,
                                                        clock,
                                                        job_config.getConfig()._cycle_sim_time_us,
                                                        job_config.getConfig()._delay_sim_time_us,
                                                        0,
                                                        *_scheduler_impl.get(),
                                                        job_runtime_check);

        RETURN_IF_FAILED(_scheduler_impl->AddTimer(*new_timer.get(),
                                                   job_config.getConfig()._cycle_sim_time_us,
                                                   job_config.getConfig()._delay_sim_time_us));
        _timers.push_back(new_timer);
    }

    return fep::Result();
}

fep::Result LocalClockBasedScheduler::start()
{
    for (auto& timer : _timers)
    {
        timer->Start();
    }
    RETURN_IF_FAILED(_scheduler_impl->Start());
    RETURN_IF_FAILED(_scheduler_thread->Start());
    return fep::Result();
}

fep::Result LocalClockBasedScheduler::stop()
{
    if (_scheduler_impl)
    {
        _scheduler_impl->Stop();
    }

    for (auto& timer : _timers)
    {
        timer->Stop();
    }

    if (_scheduler_thread)
    {
        _scheduler_thread->Join();
    }
    return fep::Result();
}

fep::Result LocalClockBasedScheduler::deinitialize()
{
    stop();
    _scheduler_impl.reset();
    _scheduler_thread.reset();
    _timers.clear();
    return fep::Result();
}

std::list<IScheduler::JobInfo> LocalClockBasedScheduler::getTasks() const
{
    return std::list<IScheduler::JobInfo>();
}
} // namespace detail
} // namespace fep
