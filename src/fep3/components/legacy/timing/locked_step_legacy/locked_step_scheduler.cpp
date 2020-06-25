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

#include "locked_step_scheduler.h"
#include <string>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>

#include "fep3/components/legacy/timing/locked_step_legacy/timing_client.h"
#include "fep3/components/legacy/timing/locked_step_legacy/timing_master.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep3/components/scheduler/scheduler_job_config.h"
#include "fep_errors.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_severity_level.h"

namespace fep
{
class IStepDataAccess;

namespace detail
{

LockedStepSchedulerClock::LockedStepSchedulerClock(ITiming& timing_client) 
    : _timing_client(timing_client),
      _event_sink(nullptr)
{
    //_current_time = 0;
}
const char* LockedStepSchedulerClock::getName() const
{
    return FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_MASTER_LOCKED_STEP_SIMTIME;
}

timestamp_t LockedStepSchedulerClock::getTime() const
{
    //return _current_time;
    return _timing_client.GetTime();
}
IClock::ClockType LockedStepSchedulerClock::getType() const
{
    return ClockType::discrete;
}
void LockedStepSchedulerClock::reset()
{
    _current_time = 0;
}

void LockedStepSchedulerClock::start(IEventSink& event_sink)
{
    _event_sink = &event_sink;
}

void LockedStepSchedulerClock::stop()
{
    _event_sink = nullptr;
}

fep::StepConfig convertToStepConfig(const fep::JobConfiguration& job_config)
{
    StepConfig config_step(job_config._cycle_sim_time_us,
        job_config._max_runtime_real_time_us,
        0,
        static_cast<fep::TimeViolationStrategy>(job_config._runtime_violation_strategy));
   
    return config_step;
}

//*****************************************
//*****************************************
    static void forwardCallToJob(void* pCallee, timestamp_t time, IStepDataAccess* pAccess)
    {
        IScheduler::IJob* job = static_cast<IScheduler::IJob*>(pCallee);
        job->executeDataIn(time);
        job->execute(time);
        job->executeDataOut(time);
    }

    LockedStepScheduler::LockedStepScheduler(timing::TimingClient& timing_client,
                                             timing::TimingMaster& timing_master,
                                             IIncidentHandler& incident_handler)
        : _timing_client(timing_client), _timing_master(timing_master), _incident_handler(incident_handler)
    {
    
    }

    const char* LockedStepScheduler::getName() const
    {
        return FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME;
    }

    void rollback(ITiming& client,
        std::list<std::pair<IScheduler::IJob*, IScheduler::JobInfo>>& jobs)
    {
        for (auto& job : jobs)
        {
            client.UnregisterStepListener(job.second.getName());
        }
        jobs.clear();
    }

    fep::Result LockedStepScheduler::initialize(IClockService& clock,
                                                IJobConfiguration& configuration)
    {
        //here we need to create all step listener!! 
        //IJob to IStepListener conversion!!
        auto jobs = configuration.getJobConfig();
        for (auto& job : jobs)
        {
            fep::StepConfig step_config = convertToStepConfig(job.second.getConfig());
            auto res = _timing_client.RegisterStepListener(job.second.getName(),
                                                           step_config,
                                                           &forwardCallToJob,
                                                           job.first);
            if (fep::isOk(res))
            {
                //we copy to our local job management
                _jobs.push_back(job);
            }
            else
            {
                //otherwise we clear the list and unregister already successfully registered
                rollback(_timing_client, _jobs);
                return res;
            }
        }

        //now we do the old legacy things! 
        auto result = _timing_client.configure();
        if (isFailed(result))
        {
            _incident_handler.InvokeIncident(FSI_TIMING_CLIENT_CONFIGURATION_FAIL,
                                              SL_Critical_Global,
                                              a_util::strings::format("The configuration of the FEP Timing Client failed - reason: %s",
                                                                       result.getDescription()).c_str(),
                                              result.getFunction(), result.getLine(), result.getFile());
            _timing_client.reset();
            return result;
        }
        result = _timing_master.configure();

        if (fep::isFailed(result))
        {
            _incident_handler.InvokeIncident(FSI_TIMING_MASTER_CONFIGURATION_FAIL,
                                              SL_Critical_Global,
                                              a_util::strings::format("The configuration of the FEP Timing Master failed - reason: %s",
                                                                      result.getDescription()).c_str(),
                                              result.getFunction(), result.getLine(), result.getFile());

            _timing_master.reset();
            _timing_client.reset();
        }

        return result;
    }

    fep::Result LockedStepScheduler::start()
    {
        auto res = _timing_client.start();
        
        if (fep::isOk(res))
        {
            res = _timing_master.start();
        }
        return res;
    }

    fep::Result LockedStepScheduler::stop()
    {
        _timing_master.stop();
        _timing_client.stop();

        return fep::Result();
    }

    fep::Result LockedStepScheduler::deinitialize()
    {
        _timing_master.reset();
        _timing_client.reset();

        rollback(_timing_client, _jobs);
        return fep::Result();
    }

    std::list<IScheduler::JobInfo> LockedStepScheduler::getTasks() const
    {
        return std::list<IScheduler::JobInfo>();
    }
}
}
