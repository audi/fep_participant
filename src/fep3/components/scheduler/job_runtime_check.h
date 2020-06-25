/**
 *
 * Job runtime violation check
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

#ifndef __FEP_JOB_RUNTIME_CHECK_H
#define __FEP_JOB_RUNTIME_CHECK_H

#include <functional>
#include <string>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep3/components/scheduler/scheduler_job_config.h"
#include "fep3/components/scheduler/scheduler_service_intf.h"

namespace fep
{
class IIncidentHandler;

class JobRuntimeCheck
{
public:
    JobRuntimeCheck(const std::string& name,
                    const fep::JobConfiguration::TimeViolationStrategy& time_violation_strategy,
                    const timestamp_t max_runtime,
                    fep::IIncidentHandler& incident_handler,
                    std::function<fep::Result()> set_participant_to_error_state);

    fep::Result runJob(const timestamp_t trigger_time, fep::IScheduler::IJob& job);

private:
    fep::Result applyTimeViolationStrategy(const timestamp_t process_duration);

private:
    const std::string _name;
    fep::JobConfiguration::TimeViolationStrategy _time_violation_strategy;
    timestamp_t _max_runtime;
    fep::IIncidentHandler& _incident_handler;
    std::function<fep::Result()> _set_participant_to_error_state;
    bool _cancelled;
    bool _skip_output;
};
} // namespace fep

#endif //__FEP_JOB_RUNTIME_CHECK_H
