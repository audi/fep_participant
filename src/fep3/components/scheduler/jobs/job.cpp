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
#include <a_util/system.h>
#include <a_util/result/error_def.h>
#include "fep3/components/scheduler/jobs/job.h"

namespace fep
{

Job::Job(std::string name, timestamp_t cycle_time) :
    _job_info(std::move(name), cycle_time),
    _call([](timestamp_t) -> Result {return Result(); }),
    _scheduler(nullptr)
{
}

Job::Job(std::string name, JobConfiguration config) :
    _job_info(std::move(name), std::move(config)),
    _call([](timestamp_t) -> Result {return Result(); }),
    _scheduler(nullptr)
{
}

Job::Job(std::string name, timestamp_t cycle_time, Func fc) :
    _job_info(std::move(name), cycle_time),
    _call(fc),
    _scheduler(nullptr)
{
}

Job::Job(std::string name, JobConfiguration config, Func fc) :
    _job_info(std::move(name), std::move(config)),
    _call(fc),
    _scheduler(nullptr)
{
}

Result Job::executeDataIn(timestamp_t time_of_execution)
{
    return fep::Result();
}

Result Job::execute(timestamp_t time_of_execution)
{
    return _call(time_of_execution);
}

fep::Result Job::executeDataOut(timestamp_t time_of_execution)
{
    return fep::Result();
}

fep::Result Job::addToComponents(const fep::IComponents & components)
{
    //do not lock here... this is task of the scheduler service
    auto scheduler = getComponent<ISchedulerService>(components);
    if (!scheduler)
    {
        RETURN_ERROR_DESCRIPTION(ERR_BAD_DEVICE, "can not find schduler service in components");
    }

    auto res = scheduler->addJob(_job_info.getName(),
        *this,
        _job_info.getConfig());
    if (isOk(res))
    {
        _scheduler = scheduler;
    }
    else
    {
        _scheduler = nullptr;
    }
    return res;
}

Result Job::removeFromComponents(const fep::IComponents & components)
{
    //do not lock here... this is task of the scheduler service
    auto scheduler = getComponent<ISchedulerService>(components);
    if (!scheduler)
    {
        RETURN_ERROR_DESCRIPTION(ERR_BAD_DEVICE, "can not find schduler service in components");
    }

    return scheduler->removeJob(_job_info.getName());
}

const IScheduler::JobInfo & Job::getJobConfig() const
{
    return _job_info;
}

Result Job::reconfigure(const fep::JobConfiguration & configuration)
{
    _job_info = IScheduler::JobInfo(getJobConfig().getName(), configuration);
    return fep::Result();
}

void Job::setFEP22Compatibility(const bool fep22_compatibility)
{
    _fep22Compatibility = fep22_compatibility;
}


Result Job::reset()
{
    return fep::Result();
}

}
