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
#ifndef __FEP_SCHEDULER_SERVICE_H
#define __FEP_SCHEDULER_SERVICE_H

#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <utility>

#include "fep_result_decl.h"
#include "fep3/components/base/component_base.h"
#include "fep3/components/scheduler/scheduler_service_intf.h"

namespace fep
{
class IIncidentHandler;
class JobConfiguration;

namespace detail
{
class LocalClockBasedScheduler;
class RPCSchedulerService;

class LocalSchedulerService : public ISchedulerService,
                              public IScheduler::IJobConfiguration,
                              public ComponentBase
{
public:
    explicit LocalSchedulerService(IIncidentHandler& incident_handler, std::function<fep::Result()> set_participant_to_error_state);
    ~LocalSchedulerService();

public:
    fep::Result registerScheduler(IScheduler& scheduler) override;
    fep::Result unregisterScheduler(const char* scheduler_name) override;
    std::list<std::string> getSchedulerList() const override;

    fep::Result addJob(const char* name,
                       IScheduler::IJob& job,
                       const fep::JobConfiguration& job_config) override;
    fep::Result removeJob(const char* name) override;
    std::list<IScheduler::JobInfo> getJobs() const override;

    fep::Result setScheduler(const char* scheduler_name) override;
    const IScheduler* getScheduler(const char* scheduler_name = "") const override;

    std::list<IScheduler::JobInfo> getTasks(const char* scheduler_name) const override;

public:
    std::list<std::pair<IScheduler::IJob*, IScheduler::JobInfo>> getJobConfig() const override;

public:
    fep::Result create() override;
    fep::Result destroy() override;
    fep::Result ready() override;
    fep::Result start() override;
    fep::Result stop() override;
    fep::Result initializing() override;
    fep::Result deinitializing() override;
    void* getInterface(const char* iid) override;

private:
    fep::Result reconfigureJobsForCompatibilityMode();

private:
    std::unique_ptr<LocalClockBasedScheduler> _local_clock_based_scheduler;
    IIncidentHandler& _incident_handler;
    std::function<fep::Result()> _set_participant_to_error_state;
    RPCSchedulerService* _rpc_impl;
    IScheduler* _current_scheduler;
    std::list<IScheduler*> _schedulers;
    const IScheduler* findScheduler(const char* scheduler_name) const;
    std::atomic_bool _started{false};
    std::list<std::pair<IScheduler::IJob*, IScheduler::JobInfo>> _jobs;
    const IScheduler::JobInfo* findJob(const char* job_name) const;
};

} // namespace detail
} // namespace fep
#endif //__FEP_SCHEDULER_SERVICE_H
