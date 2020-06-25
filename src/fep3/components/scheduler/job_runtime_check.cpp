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

#include <cstddef>
#include <a_util/result/result_type.h>
#include <a_util/result/error_def.h>
#include <a_util/strings/strings_format.h>
#include <a_util/system/system.h>

#include "incident_handler/fep_incident_handler_intf.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep_errors.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_severity_level.h"
#include "job_runtime_check.h"

#include <chrono>

#define INVOKE_INCIDENT(Handler, Code, Serv, Descr)                                                \
    Handler.InvokeIncident(Code, Serv, Descr, "JobRuntimeCheck", 0, NULL)

namespace fep
{

JobRuntimeCheck::JobRuntimeCheck(
    const std::string& name,
    const fep::JobConfiguration::TimeViolationStrategy& time_violation_strategy,
    const timestamp_t max_runtime,
    fep::IIncidentHandler& incident_handler,
    std::function<fep::Result()> set_participant_to_error_state)
    : _name(name),
      _time_violation_strategy(time_violation_strategy),
      _max_runtime(max_runtime),
      _incident_handler(incident_handler),
      _set_participant_to_error_state(set_participant_to_error_state),
      _cancelled(false),
      _skip_output(false)
{
}

fep::Result JobRuntimeCheck::runJob(const timestamp_t trigger_time, fep::IScheduler::IJob& job)
{
    if (_cancelled)
    {
        return ERR_CANCELLED;
    }

    _skip_output = false;

    if (fep::isFailed(job.executeDataIn(trigger_time)))
    {
        INVOKE_INCIDENT(
            _incident_handler,
            FSI_GENERAL_WARNING,
            SL_Warning,
            a_util::strings::format("Job %s: Execution of data input step failed for this processing cycle.",
                                    _name.c_str())
                .c_str());
    }

    auto start = std::chrono::high_resolution_clock::now();
    fep::Result result = job.execute(trigger_time);
    auto end = std::chrono::high_resolution_clock::now();

    auto execution_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    if (isFailed(result))
    {
        INVOKE_INCIDENT(
            _incident_handler,
            FSI_GENERAL_WARNING,
            SL_Warning,
            a_util::strings::format("Job %s: Execution of data processing step failed for this processing cycle.",
                                    _name.c_str())
                .c_str());
    }

    if (0 < _max_runtime && execution_time > _max_runtime)
    {
        RETURN_IF_FAILED(applyTimeViolationStrategy(execution_time));
    }

    if (!_skip_output)
    {
        if (fep::isFailed(job.executeDataOut(trigger_time)))
        {
            INVOKE_INCIDENT(
                _incident_handler,
                FSI_GENERAL_WARNING,
                SL_Warning,
                a_util::strings::format(
                    "Job %s: Execution of data output step failed for this processing cycle.", _name.c_str())
                    .c_str());
        }
    }

    return result;
}

fep::Result JobRuntimeCheck::applyTimeViolationStrategy(const timestamp_t process_duration)
{
    fep::Result result = fep::Result();
    switch (_time_violation_strategy)
    {
        case fep::JobConfiguration::TS_IGNORE_RUNTIME_VIOLATION:
            // ignore
            break;
        case fep::JobConfiguration::TS_WARN_ABOUT_RUNTIME_VIOLATION:
            INVOKE_INCIDENT(
                _incident_handler,
                FSI_GENERAL_WARNING,
                SL_Warning,
                a_util::strings::format(
                    "Job %s: Computation time (%d us) exceeded configured maximum runtime.",
                    _name.c_str(),
                    process_duration)
                    .c_str());
            result = ERR_NOERROR;
            break;
        case fep::JobConfiguration::TS_SKIP_OUTPUT_PUBLISH:
            INVOKE_INCIDENT(
                _incident_handler,
                FSI_GENERAL_CRITICAL,
                SL_Critical,
                a_util::strings::format(
                    "Job %s: Computation time (%d us) exceeded configured maximum runtime. "
                    "CAUTION: "
                    "defined output in data writer queues will not be published during this processing cycle!",
                    _name.c_str(),
                    process_duration)
                    .c_str());
            _skip_output = true;
            result = ERR_NOERROR;
            break;
        case fep::JobConfiguration::TS_SET_STM_TO_ERROR:
            INVOKE_INCIDENT(
                _incident_handler,
                FSI_GENERAL_CRITICAL,
                SL_Critical,
                a_util::strings::format(
                    "Job %s: Computation time (%d us) exceeded configured maximum runtime. FATAL: "
                    "changing "
                    "state to FS_ERROR - continuation of simulation not possible!",
                    _name.c_str(),
                    process_duration)
                    .c_str());
            result = _set_participant_to_error_state();
            if (fep::isFailed(result))
            {
                RETURN_ERROR_DESCRIPTION(result.getErrorCode(), "Failed to set participant to state FS_ERROR. State change was initiated because the configured maximum job runtime was exceeded.");
            }
            _cancelled = true;
            _skip_output = true;
            result = ERR_CANCELLED;
            break;
        case fep::JobConfiguration::TS_UNKNOWN:
            // Should never be the case
            break;
    }
    return result;
}

} // namespace fep
