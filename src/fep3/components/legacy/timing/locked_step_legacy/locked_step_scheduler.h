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
#ifndef __FEP_LOCKED_STEP_SCHEDULER_H
#define __FEP_LOCKED_STEP_SCHEDULER_H

#include <a_util/base/types.h>
#include <atomic>
#include <list>
#include <utility>
#include "fep3/components/clock/clock_service_intf.h"
#include "fep3/components/scheduler/scheduler_service_intf.h"
#include "fep_result_decl.h"

namespace fep
{
class IIncidentHandler;
class ITiming;

namespace timing
{
class TimingClient;
class TimingMaster;
}

namespace detail
{

class LockedStepSchedulerClock : public IClock
{
    public:
        LockedStepSchedulerClock(ITiming& timing_client);
        ~LockedStepSchedulerClock() = default;
        const char* getName() const override;
    private:
        timestamp_t getTime() const override;
        ClockType getType() const override;
        void reset() override;

        void start(IEventSink& event_sink) override;
        void stop() override;
    private:
        IEventSink* _event_sink;
#ifndef __QNX__
        std::atomic<timestamp_t> _current_time;
#else
        atomic_timestamp_t _current_time;
#endif
        ITiming& _timing_client;
};

class LockedStepScheduler : public IScheduler
{
    public:
        LockedStepScheduler(timing::TimingClient& timing_client,
                            timing::TimingMaster& timing_master,
                            IIncidentHandler& incident_handler);
        ~LockedStepScheduler() = default;

    public:
        const char* getName() const override;
        fep::Result initialize(IClockService& clock,
                               IJobConfiguration& configuration) override;
        fep::Result start() override;
        fep::Result stop() override;
        fep::Result deinitialize()  override;

        std::list<IScheduler::JobInfo> getTasks() const override;

     private:
         IIncidentHandler& _incident_handler;
         timing::TimingClient& _timing_client;
         timing::TimingMaster& _timing_master;
         std::list<std::pair<IJob*, JobInfo>> _jobs;

};



}
}
#endif //__FEP_LOCKED_STEP_SCHEDULER_H
