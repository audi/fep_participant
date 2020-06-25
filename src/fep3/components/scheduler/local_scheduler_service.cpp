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

#include <string>
#include <json/value.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>
#include <a_util/strings/strings_functions.h>
#include <rpc_pkg/rpc_server.h>

#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "clock_based/local_clock_based_scheduler.h"
#include "fep3/components/base/component_intf.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/components/rpc/fep_rpc_intf.h"
#include "fep3/components/rpc/fep_rpc_stubs.h"
#include "fep3/components/clock/clock_service_intf.h"
#include "fep3/rpc_components/scheduler/scheduler_service.h"
#include "fep3/rpc_components/scheduler/scheduler_service_rpc_intf_def.h"
#include "fep_errors.h"
#include "local_scheduler_service.h"
#include "fep3/components/scheduler/jobs/job.h"

namespace fep {
class JobConfiguration;
}  // namespace fep

#define INVOKE_INCIDENT(Handler, Error, Serv)                                                    \
    Handler.InvokeIncident(static_cast<std::int16_t>(Error.getErrorCode()), Serv,                \
                           Error.getDescription(), "LocalSchedulerService", __LINE__, __FILE__)

namespace fep
{
namespace detail
{

class RPCSchedulerService
    : public rpc_object_server<rpc_stubs::RPCSchedulerService, fep::rpc::IRPCSchedulerServiceDef>
{
public:
    explicit RPCSchedulerService(ISchedulerService& service) : _service(&service)
    {
    }

protected:
    std::string getSchedulers() override
    {
        std::string ret;
        std::list<std::string> scheduler_names = _service->getSchedulerList();
        bool first = true;
        for (const auto& schedulername : scheduler_names)
        {
            if (first)
            {
                if (!first)
                {
                    ret += ",";
                }
                else
                {
                    first = false;
                }
                ret += schedulername;
            }
            ret += schedulername;
        }
        return ret;
    }
    std::string getCurrentScheduler() override
    {
        return _service->getScheduler("")->getName();
    }
    std::string getJobs() override
    {
        return "";
    }

    Json::Value getJobInfo(const std::string& job_name) override
    {
        return Json::Value();
    }

    std::string getTasks(const std::string& scheduler_name) override
    {
        return "";
    }

    Json::Value getTaskInfo(const std::string& scheduler_name, const std::string& task_name) override
    {
        return Json::Value();
    }

private:
    ISchedulerService* _service;
};

LocalSchedulerService::LocalSchedulerService(IIncidentHandler& incident_handler, std::function<fep::Result()> set_participant_to_error_state)
    : _incident_handler(incident_handler),
      _set_participant_to_error_state(set_participant_to_error_state),
      _rpc_impl(nullptr),
      _current_scheduler(nullptr)
{
}

fep::Result LocalSchedulerService::create()
{
    IPropertyTree* property_tree = _components->getComponent<IPropertyTree>();
    _local_clock_based_scheduler.reset(new LocalClockBasedScheduler(_incident_handler, _set_participant_to_error_state));
    registerScheduler(*_local_clock_based_scheduler);
    setScheduler(_local_clock_based_scheduler->getName());

    std::string res = getProperty(*property_tree, FEP_SCHEDULERSERVICE_SCHEDULER);
    if (res.empty())
    {
        // set default scheduler
        setProperty(
            *property_tree, FEP_SCHEDULERSERVICE_SCHEDULER, _local_clock_based_scheduler->getName());
    }

    auto rpc = _components->getComponent<IRPC>();
    _rpc_impl = new RPCSchedulerService(*this);
    rpc->GetRegistry()->RegisterObjectServer(rpc::IRPCSchedulerServiceDef::DEFAULT_NAME, *_rpc_impl);
    return fep::Result();
}

fep::Result LocalSchedulerService::destroy()
{
    _current_scheduler = nullptr;
    if (_rpc_impl)
    {
        delete _rpc_impl;
        _rpc_impl = nullptr;
    }
    return fep::Result();
}

LocalSchedulerService::~LocalSchedulerService()
{
}

fep::Result LocalSchedulerService::initializing()
{
    return fep::Result();
}

fep::Result LocalSchedulerService::ready()
{
    IPropertyTree* property_tree = _components->getComponent<IPropertyTree>();
    if (nullptr == property_tree)
    {
        fep::Result result = fep::Result(
            ERR_POINTER,
            a_util::strings::format(
                "Initializing scheduler service failed. Property tree is not available.")
            .c_str(),
            __LINE__,
            __FILE__,
            "initializing");
        INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
        return result;
    }

    bool fep22_compatibility{ false };
    property_tree->GetPropertyValue(FEP_SCHEDULERSERVICE_FEP22_COMPATIBILITY_MODE_ENABLED, fep22_compatibility);

    if (fep22_compatibility)
    {
        reconfigureJobsForCompatibilityMode();
    }

    std::string scheduler_mode = getProperty(*property_tree, FEP_SCHEDULERSERVICE_SCHEDULER);

    RETURN_IF_FAILED(setScheduler(scheduler_mode.c_str()));
    RETURN_IF_FAILED(
        _current_scheduler->initialize(*_components->getComponent<IClockService>(), *this));
    return fep::Result();
}

fep::Result LocalSchedulerService::deinitializing()
{
    stop();
    if (_current_scheduler)
    {
        _current_scheduler->deinitialize();
    }

    return fep::Result();
}

fep::Result LocalSchedulerService::start()
{
    _started = true;
    return _current_scheduler->start();
}

fep::Result LocalSchedulerService::stop()
{
    auto res = fep::Result();
    if (_current_scheduler)
    {
        res = _current_scheduler->stop();
    }
    _started = false;
    return res;
}

void* LocalSchedulerService::getInterface(const char* iid)
{
    if (fep::getComponentIID<ISchedulerService>() == iid)
    {
        return static_cast<ISchedulerService*>(this);
    }
    else
    {
        return nullptr;
    }
}

fep::Result LocalSchedulerService::setScheduler(const char* scheduler_name)
{
    auto sched = findScheduler(scheduler_name);
    if (sched)
    {
        for (auto& current_scheduler : _schedulers)
        {
            if (a_util::strings::isEqual(current_scheduler->getName(), scheduler_name))
            {
                _current_scheduler = current_scheduler;
                return fep::Result();
            }
        }
    }
    fep::Result result =
        fep::Result(ERR_NOT_FOUND,
                    a_util::strings::format(
                        "Setting scheduler failed. A scheduler with the name %s is not registered.",
                        scheduler_name)
                        .c_str(),
                    __LINE__,
                    __FILE__,
                    "setScheduler");
    INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
    return result;
}

const IScheduler* LocalSchedulerService::getScheduler(const char* scheduler_name) const
{
    if (std::string(scheduler_name).empty())
    {
        return _current_scheduler;
    }
    else
    {
        return findScheduler(scheduler_name);
    }
}

const IScheduler* LocalSchedulerService::findScheduler(const char* scheduler_name) const
{
    for (const auto& current_scheduler : _schedulers)
    {
        if (a_util::strings::isEqual(current_scheduler->getName(), scheduler_name))
        {
            return current_scheduler;
        }
    }
    return nullptr;
}

fep::Result LocalSchedulerService::registerScheduler(IScheduler& scheduler)
{
    if (_started)
    {
        fep::Result result = fep::Result(
            ERR_INVALID_STATE,
            a_util::strings::format("Registering a scheduler while running is not possible")
                .c_str(),
            __LINE__,
            __FILE__,
            "registerScheduler");
        INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
        return result;
    }
    auto val = findScheduler(scheduler.getName());
    if (val == nullptr)
    {
        _schedulers.push_back(&scheduler);
        return fep::Result();
    }
    else
    {
        fep::Result result = fep::Result(
            ERR_INVALID_ARG,
            a_util::strings::format(
                "Registering scheduler failed. A scheduler with the name %s is already registered.",
                scheduler.getName())
                .c_str(),
            __LINE__,
            __FILE__,
            "registerScheduler");
        INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
        return result;
    }
}

fep::Result LocalSchedulerService::unregisterScheduler(const char* scheduler_name)
{
    if (_started)
    {
        fep::Result result = fep::Result(
            ERR_INVALID_STATE,
            a_util::strings::format("Unregistering a scheduler while running is not possible")
                .c_str(),
            __LINE__,
            __FILE__,
            "unregisterScheduler");
        INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
        return result;
    }

    if (a_util::strings::isEqual(_local_clock_based_scheduler->getName(), scheduler_name))
    {
        fep::Result result = fep::Result(
            ERR_INVALID_ARG,
            a_util::strings::format("Unregistering the default scheduler is not possible")
            .c_str(),
            __LINE__,
            __FILE__,
            "unregisterScheduler");
        INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
        return result;
    }

    for (decltype(_schedulers)::iterator current_scheduler = _schedulers.begin();
         current_scheduler != _schedulers.end();
         current_scheduler++)
    {
        if (a_util::strings::isEqual((*current_scheduler)->getName(), scheduler_name))
        {
            _schedulers.erase(current_scheduler);
            if (a_util::strings::isEqual(_current_scheduler->getName(), scheduler_name))
            {
                _current_scheduler = _local_clock_based_scheduler.get();
            }
            return fep::Result();
        }
    }
    fep::Result result = fep::Result(
        ERR_NOT_FOUND,
        a_util::strings::format(
            "Unregisterung scheduler failed. A scheduler with the name %s is not registered.",
            scheduler_name)
            .c_str(),
        __LINE__,
        __FILE__,
        "unregisterScheduler");
    INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
    return result;
}

std::list<std::string> LocalSchedulerService::getSchedulerList() const
{
    std::list<std::string> ret;
    for (const auto& current_scheduler : _schedulers)
    {
        ret.push_back(current_scheduler->getName());
    }
    return ret;
}

const IScheduler::JobInfo* LocalSchedulerService::findJob(const char* job_name) const
{
    for (const auto& current_job : _jobs)
    {
        if (std::string(current_job.second.getName()) == std::string(job_name))
        {
            return &current_job.second;
        }
    }
    return nullptr;
}

fep::Result LocalSchedulerService::addJob(const char* name,
                                          IScheduler::IJob& job,
                                          const fep::JobConfiguration& job_config)
{
    auto job_info = findJob(name);
    if (job_info)
    {
        fep::Result result = fep::Result(
            ERR_INVALID_ARG,
            a_util::strings::format(
                "Adding job to scheduler service failed. A job with the name %s already exists.",
                name)
                .c_str(),
            __LINE__,
            __FILE__,
            "addJob");
        INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
        return result;
    }
    else
    {
        _jobs.push_back(std::pair<IScheduler::IJob*, IScheduler::JobInfo>(
            &job, IScheduler::JobInfo(name, job_config)));
        return fep::Result();
    }
}

fep::Result LocalSchedulerService::removeJob(const char* name)
{
    auto job_info = findJob(name);
    if (job_info)
    {
        for (decltype(_jobs)::iterator it = _jobs.begin(); it != _jobs.end(); it++)
        {
            if (it->second.getName() == std::string(name))
            {
                _jobs.erase(it);
                return fep::Result();
            }
        }
    }
    fep::Result result = fep::Result(
        ERR_NOT_FOUND,
        a_util::strings::format(
            "Removing job from scheduler service failed. A job with the name %s does not exist.",
            name)
            .c_str(),
        __LINE__,
        __FILE__,
        "removeJob");
    INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
    return result;
}

std::list<IScheduler::JobInfo> LocalSchedulerService::getJobs() const
{
    std::list<IScheduler::JobInfo> jobs;
    for (const auto& job : _jobs)
    {
        jobs.push_back(job.second);
    }
    return jobs;
}

std::list<std::pair<IScheduler::IJob*, IScheduler::JobInfo>>
    LocalSchedulerService::getJobConfig() const
{
    return _jobs;
}

std::list<IScheduler::JobInfo> LocalSchedulerService::getTasks(const char* scheduler_name) const
{
    return _current_scheduler->getTasks();
}

fep::Result LocalSchedulerService::reconfigureJobsForCompatibilityMode()
{
    for (auto& job_entry : _jobs)
    {
        fep::Job* current_job = dynamic_cast<fep::Job*>(job_entry.first);

        if (current_job)
        {
            current_job->setFEP22Compatibility(true);
        }
        else
        {
            fep::Result result = fep::Result(
                ERR_POINTER,
                a_util::strings::format(
                    "Initializing of scheduler failed because property '%s' was set but job '%s' does not support the feature (fep3::Job or specialisation has to be used)",
                    FEP_SCHEDULERSERVICE_FEP22_COMPATIBILITY_MODE_ENABLED,
                    job_entry.second.getName())
                .c_str(),
                __LINE__,
                __FILE__,
                "reconfigureJobsForCompatibilityMode");
            INVOKE_INCIDENT(_incident_handler, result, fep::SL_Critical);
            return result;
        }
    }
    
    return {};
}

} // namespace detail
} // namespace fep
