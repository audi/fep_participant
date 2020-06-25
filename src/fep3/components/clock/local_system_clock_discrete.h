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
#ifndef __FEP_LOCAL_SYSTEM_CLOCK_DISCRETE_H
#define __FEP_LOCAL_SYSTEM_CLOCK_DISCRETE_H

#include <atomic>
#include <cstdint>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <a_util/base/types.h>

#include "fep3/components/clock/clock_base.h"

namespace fep
{

class DiscreteClockUpdater
{
    protected:
        DiscreteClockUpdater();

        void startWorking();
        void stopWorking();

        virtual void updateTime(const timestamp_t new_time) = 0;

    private:
        void work();

        std::int32_t _cycle_time;                // Duration of a single discrete time step in milliseconds
        double _time_factor;                // Factor to control relation between simulated time and system time

        timestamp_t _simulated_time;        // Current simulation time
        timestamp_t _next_request_gettime;  // System time timestamp of the next discrete time step

        std::unique_ptr<std::thread> _worker;
        std::atomic_bool _stop;

        std::mutex _lock_clock_updater;

        std::condition_variable _cycle_wait_condition;

    public:
        void updateConfiguration(const int32_t cycle_time,
                                 const double time_factor);

};

class LocalSystemSimClock : public DiscreteClockUpdater,
                            public DiscreteClock
{
    public:
        LocalSystemSimClock();
        ~LocalSystemSimClock() = default;

        void start(IEventSink& _sink) override;
        void stop() override;
        void updateTime(const timestamp_t new_time) override;

};

}

#endif //__FEP_LOCAL_SYSTEM_CLOCK_DISCRETE_H
