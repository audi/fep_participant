/**
* Declaration of the Class IRPCSchedulerService. (can be reached from over rpc)
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

#ifndef __FEP_JOB_H
#define __FEP_JOB_H

#include <functional>
#include "fep3/components/base/component_intf.h"
#include "fep3/components/scheduler/scheduler_service_intf.h"

namespace fep
{
    class FEP_PARTICIPANT_EXPORT Job : public IScheduler::IJob
    {
    public:
        typedef std::function<Result(timestamp_t time_of_execution)> Func;
        /**
        * CTOR if you want to override execute by yourself
        */
        Job(std::string name, timestamp_t cycle_time);

        /**
        * CTOR if you want to override execute by yourself
        */
        Job(std::string name, JobConfiguration config);

        /**
        * CTOR if you adding a function by @p fc
        */
        Job(std::string name, timestamp_t cycle_time, Func fc);

        /**
        * CTOR if you adding a function by @p fc
        */
        Job(std::string name, JobConfiguration config, Func fc);

    protected:
        fep::Result executeDataIn(timestamp_t time_of_execution) override;
        fep::Result execute(timestamp_t time_of_execution) override;
        fep::Result executeDataOut(timestamp_t time_of_execution) override;

    public:
        virtual fep::Result addToComponents(const fep::IComponents& components);
        virtual fep::Result removeFromComponents(const fep::IComponents& components);
        const IScheduler::JobInfo& getJobConfig() const;
        fep::Result reconfigure(const fep::JobConfiguration& configuration);

        /**
        * Setter function for the job compatibility mode.
        * This is used by the scheduler service to configure jobs
        * and the compatibility mode depending on the corresponding
        * FEP property @ref FEP_SCHEDULERSERVICE_FEP22_COMPATIBILITY_MODE_ENABLED.
        */
        void setFEP22Compatibility(const bool fep22_compatibility);
        virtual fep::Result reset();

    protected:
        bool                           _fep22Compatibility{ false };

    private:
        IScheduler::JobInfo            _job_info;
        Func                           _call;
        ISchedulerService*             _scheduler;
    };
}

#endif // __FEP_JOB_H