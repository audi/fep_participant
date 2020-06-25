/************************************************************************
* Implementation of a timing master which utilizes an external continuous or discrete clock
*

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

#ifndef _EXAMPLE_TIMING_MASTER_EXTERNAL_CLOCK_H_
#define _EXAMPLE_TIMING_MASTER_EXTERNAL_CLOCK_H_

#include <thread>
#include <mutex>
#include <condition_variable>

#include <fep_participant_sdk.h>

/// Available timing master clock modes
/// Depending on the configured clock mode, either the custom continuous or
/// the custom discrete clock is set as main clock for the timing master
enum ClockMode
{
    UNDEFINED_MODE = -1,
    CONTINUOUS_MODE = 0,
    DISCRETE_MODE = 1
};

/// Helper class for the custom discrete clock
class DiscreteClockUpdater
{
protected:
    DiscreteClockUpdater();

    void startWorking();
    void stopWorking();

    virtual void updateTime(const timestamp_t new_time) = 0;

    /// Duration of a single discrete time step
    int32_t _cycle_time;

private:
    void work();

    /// Simulated time which is distributed by the EventSink of the discrete clock
    timestamp_t _current_simulation_time;
    /// Timestamp of the next discrete time step
    timestamp_t _next_request_gettime;
    
    std::unique_ptr<std::thread> _worker;
    std::atomic_bool _stop;

    std::mutex _lock_clock_updater;

    std::condition_variable _cycle_wait_condition;
};

/// Custom discrete clock
class CustomDiscreteClock : public DiscreteClockUpdater,
                            public fep::DiscreteClock
{
public:
    CustomDiscreteClock();
    ~CustomDiscreteClock() = default;

public:
    void start(IEventSink& sink) override;
    void stop();
    void updateTime(const timestamp_t new_time) override;
    void updateCycleTime(const int32_t nCycleTime);
};

/// Custom continuous clock
class CustomContinuousClock : public fep::ContinuousClock
{
public:
    CustomContinuousClock();
    ~CustomContinuousClock() = default;

public:
    timestamp_t getNewTime() const override;
    timestamp_t resetTime() override;

private:
    /// Offset if the continuous clock is reset at some point
    mutable timestamp_t _current_offset;
};

/// FEP Timing Master
class cTimingMasterElement : public fep::cModule
{
public:
    cTimingMasterElement(const ClockMode& eClockMode, const int nCycleTime);
    ~cTimingMasterElement();

public:
    fep::Result ProcessStartupEntry(const fep::tState eOldState) override;
    fep::Result ProcessInitializingEntry(const fep::tState eOldState) override;

private:
    /// Clock mode
    ClockMode _clock_mode;
    /// Cycle time of the discrete clock
    int32_t _cycle_time;

    /// External continuous clock (used if m_eClockMode == CONTINUOUS_MODE)
    CustomContinuousClock _custom_continuous_clock;
    /// External discrete clock (used if m_eClockMode == DISCRETE_MODE)
    CustomDiscreteClock _custom_discrete_clock;
};

#endif // _EXAMPLE_TIMING_MASTER_EXTERNAL_CLOCK_H