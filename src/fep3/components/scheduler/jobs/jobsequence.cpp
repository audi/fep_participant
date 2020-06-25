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
#include "fep3/components/scheduler/jobs/jobsequence.h"

namespace fep
{

JobSequence::JobSequence(std::string name, timestamp_t cycle_time) :
    Job(std::move(name), cycle_time)
{
}

JobSequence::JobSequence(std::string name, timestamp_t cycle_time, std::vector<std::pair<std::string, IJob*>> jobs_to_add) :
    Job(std::move(name), cycle_time),
    _sequence(std::move(jobs_to_add))
{
}

fep::Result JobSequence::addJob(std::string name, IJob & job_reference)
{
    _sequence.push_back({ name, &job_reference });
    return ERR_NOERROR;
}

void JobSequence::clearJobs()
{
    _sequence.clear();
}

fep::Result JobSequence::execute(timestamp_t time_of_execution)
{
    for (const auto& job : _sequence)
    {
        auto err_code = job.second->execute(time_of_execution);
        //TODO: incident if error // break execution
        //TODO: error event ... to health state
        if (fep::isFailed(err_code))
        {
            return err_code;
        }
    }
    return fep::Result();
}

} // ns fep
