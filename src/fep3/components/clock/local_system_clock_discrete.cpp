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
#include <chrono>
#include <exception>
#include <iostream>
#include <a_util/system/system.h>

#include "fep3/components/clock/clock_service_intf.h"
#include "local_system_clock_discrete.h"

namespace fep
{
DiscreteClockUpdater::DiscreteClockUpdater()
    : _stop(false), _cycle_time(100000), _time_factor(1.0), _simulated_time(0)
{
}

void DiscreteClockUpdater::startWorking()
{
    _simulated_time = 0;
    _stop = false;
    _next_request_gettime = -1;
    _worker.reset(new std::thread([this] { work();  }));
}

void DiscreteClockUpdater::stopWorking()
{
    _stop = true;
    if (_worker)
    {
        if (_worker->joinable())
        {
            _worker->join();
        }
    }
}

void DiscreteClockUpdater::work()
{
    while (!_stop)
    {
        if (_next_request_gettime == -1)
        {
            // no need to wait
        }
        else
        {
            std::unique_lock<std::mutex> guard(_lock_clock_updater);

            // If time factor is configured to be 0,0, we do not wait between time steps
            if (_time_factor != 0.0)
            {
                timestamp_t current_demand_time_diff = static_cast<timestamp_t>(
                    (_next_request_gettime - a_util::system::getCurrentMicroseconds()) /
                    static_cast<float>(_time_factor));

                // If the system timestamp for the next discrete time step is not reached yet, we
                // wait
                if (current_demand_time_diff > 0)
                {
                    _cycle_wait_condition.wait_for(
                        guard, std::chrono::microseconds(current_demand_time_diff));
                }
            }
        }
        _next_request_gettime = a_util::system::getCurrentMicroseconds() + _cycle_time;

        try
        {
            {
                std::lock_guard<std::mutex> locked(_lock_clock_updater);
                updateTime(_simulated_time);
            }

            _simulated_time += _cycle_time;          
        }
        catch (std::exception& exception)
        {
            std::cout << "Caught an exception during update of simulation time: "
                      << exception.what() << std::endl;
        }
    }
}

void DiscreteClockUpdater::updateConfiguration(const int32_t cycle_time, const double time_factor)
{
    // multiply cycle_time by factor 1000 to get microsecond value from miliseconds
    _cycle_time = cycle_time * 1000;
    _time_factor = time_factor;
}

LocalSystemSimClock::LocalSystemSimClock()
    : DiscreteClockUpdater(), DiscreteClock(FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME)
{
}

void LocalSystemSimClock::start(IEventSink& _sink)
{
    DiscreteClock::start(_sink);
    DiscreteClockUpdater::startWorking();
}

void LocalSystemSimClock::stop()
{
    DiscreteClockUpdater::stopWorking();
    DiscreteClock::stop();
}

void LocalSystemSimClock::updateTime(const timestamp_t new_time)
{
    DiscreteClock::setNewTime(new_time, true);
}

} // namespace fep
