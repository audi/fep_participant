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

#include "timing_master_external_clock_participant.h"

#include <a_util/system/system.h>
#include <iostream>

//! [DiscreteClockUpdater]
DiscreteClockUpdater::DiscreteClockUpdater()
    : _stop(false), _current_simulation_time(0), _cycle_time(100000)
{
}

void DiscreteClockUpdater::startWorking()
{
    _current_simulation_time = 0;
    _stop = false;
    _next_request_gettime = -1;
    _worker.reset(new std::thread([this] { work(); }));
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
            std::unique_lock<std::mutex> oGuard(_lock_clock_updater);
            // Calculate the remaining time difference until the system time reaches the next
            // discrete time step
            timestamp_t nCurrentDemandTimeDiff =
                _next_request_gettime - a_util::system::getCurrentMicroseconds();

            if (nCurrentDemandTimeDiff > 0)
            {
                // Wait until the next discrete time step is reached
                _cycle_wait_condition.wait_for(oGuard,
                                               std::chrono::microseconds(nCurrentDemandTimeDiff));
            }
        }

        try
        {
            {
                // Update the discrete clock's time
                // An eventSink is used to distribute the simulated time across the system
                std::lock_guard<std::mutex> oLocked(_lock_clock_updater);
                updateTime(_current_simulation_time);
            }

            // Update the simulation time and the system time timestamp of the next discrete time
            // step
            _current_simulation_time += _cycle_time;
            _next_request_gettime = a_util::system::getCurrentMicroseconds() + _cycle_time;
        }
        catch (std::exception& exception)
        {
            std::cout << "Caught an exception during simulation time update: " << exception.what()
                      << std::endl;
        }
    }
}
//! [DiscreteClockUpdater]

//! [CucstomDiscreteClock]
CustomDiscreteClock::CustomDiscreteClock()
    : DiscreteClockUpdater(), DiscreteClock("demo_discrete_clock")
{
}

// Start the discrete clock and the corresponding clock updater
// The EventSink is used by the discrete clock to distribute the simulation time across the FEP
// system
void CustomDiscreteClock::start(IEventSink& oSink)
{
    DiscreteClock::start(oSink);
    DiscreteClockUpdater::startWorking();
}

// Stop the discrete clock and the corresponding clock updater
void CustomDiscreteClock::stop()
{
    DiscreteClockUpdater::stopWorking();
    DiscreteClock::stop();
}

// Distribute the simulation time across the FEP system
// Send an event before starting and after finishing the update process
void CustomDiscreteClock::updateTime(const timestamp_t nNewTime)
{
    DiscreteClock::setNewTime(nNewTime, true);
}

// Set the discrete clock's cycle time which defines the length of a discrete time step
void CustomDiscreteClock::updateCycleTime(const int32_t cycle_time)
{
    // multyply user cycle time by factor 1000 to convert miliseconds to microseconds
    _cycle_time = cycle_time * 1000;
}
//! [CucstomDiscreteClock]

//! [CucstomContinuousClock]
CustomContinuousClock::CustomContinuousClock()
    : _current_offset(0), ContinuousClock("demo_continuous_clock")
{
}

// Return the current system time while taking into account an optional offset
timestamp_t CustomContinuousClock::getNewTime() const
{
    return (a_util::system::getCurrentMicroseconds() - _current_offset);
}

// Reset the clock time and store the current system time as offset for future use
timestamp_t CustomContinuousClock::resetTime()
{
    _current_offset = a_util::system::getCurrentMicroseconds();
    return (a_util::system::getCurrentMicroseconds() - _current_offset);
}
//! [CucstomContinuousClock]

cTimingMasterElement::cTimingMasterElement(const ClockMode& clock_mode, const int cycle_time)
    : _clock_mode(clock_mode), _cycle_time(cycle_time)
{
}

cTimingMasterElement::~cTimingMasterElement()
{
    Destroy();
}

fep::Result cTimingMasterElement::ProcessStartupEntry(const fep::tState eOldState)
{
    fep::Result result = fep::ERR_NOERROR;

    //! [ConfigureParticipantHeader]
    // Configure the FEP Participant header
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION,
                                        "Timing Demo Master External Clock");
    double fFepVersion =
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) + static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR,
                                        "Audi Electronics Venture GmbH");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID,
                                        "31b21158-dfc4-4ba7-98be-7457328d06ab");

    // This element is the timing master
    GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, GetName());
    //! [ConfigureParticipantHeader]

    //! [RegisterCustomClocks]
    // Register both custom clocks at the clock service
    fep::IClockService* clock_service = GetComponents()->getComponent<fep::IClockService>();
    RETURN_IF_FAILED(clock_service->registerClock(_custom_continuous_clock));
    RETURN_IF_FAILED(clock_service->registerClock(_custom_discrete_clock));
    //! [RegisterCustomClocks]

    //! [SetMainClock]
    // Set the main clock depending on the configured clock mode
    switch (_clock_mode)
    {
        case CONTINUOUS_MODE:
            RETURN_IF_FAILED(clock_service->setMainClock(_custom_continuous_clock.getName()));
            break;
        case DISCRETE_MODE:
            // Set the cycle time for discrete time steps of the discrete clock
            _custom_discrete_clock.updateCycleTime(_cycle_time);
            RETURN_IF_FAILED(clock_service->setMainClock(_custom_discrete_clock.getName()));
            break;
    }
    //! [SetMainClock]

    GetStateMachine()->StartupDoneEvent();

    return result;
}

fep::Result cTimingMasterElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    GetStateMachine()->InitDoneEvent();

    return fep::ERR_NOERROR;
}

int main(int nArgc, const char* pArgv[])
{
    //! [EnableFEPTiming25]
    fep::cModuleOptions module_options("TimingMasterExternalClock",
                                       fep::eTimingSupportDefault::timing_FEP_30);
    //! [EnableFEPTiming25]

    // Extend the FEP Participant executable options by an option to choose the Participant's main
    // clock
    std::string strMode = std::string();
    module_options.SetAdditionalOption(
        strMode,
        "-m",
        "--mode",
        "(Required Argument) set timing master clock mode. Available modes are: \n"
        "     0: Continuous mode.\n"
        "     1: Discrete mode.\n",
        "int");

    // Extend the FEP Participant executable options by an option to configure the cycle time of the
    // custom discrete clock
    std::string strCycle = std::string();
    module_options.SetAdditionalOption(strCycle,
                                       "-c",
                                       "--cycle",
                                       "Set the discrete clock cycle time in ms.\n"
                                       "Cycle time needs to be > 0.\n"
                                       "Default value is 100.\n",
                                       "int");

    if (fep::isFailed(module_options.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }

    // Set the FEP Participant clock mode depending on the corresponding command line option
    ClockMode clock_mode = UNDEFINED_MODE;
    if (!strMode.empty())
    {
        const int mode = atoi(strMode.c_str());
        switch (mode)
        {
            case CONTINUOUS_MODE:
                clock_mode = CONTINUOUS_MODE;
                break;
            case DISCRETE_MODE:
                clock_mode = DISCRETE_MODE;
                break;
            default:
                clock_mode = UNDEFINED_MODE;
        }
    }

    if (clock_mode == UNDEFINED_MODE)
    {
        std::cerr << "Error: Invalid clock mode value. See usage for allowed values." << std::endl;
        module_options.PrintHelp();
        return 1;
    }

    // Set the discrete clock's cycle time depending on the corresponding command line option
    int cycle_time = 100;
    if (!strCycle.empty())
    {
        cycle_time = atoi(strCycle.c_str());
    }

    if (cycle_time <= 0)
    {
        std::cerr << "Error: Invalid cycle time value. See usage for allowed values." << std::endl;
        module_options.PrintHelp();
        return 1;
    }

    // Create the FEP Participant using the corresponding timing configuration
    cTimingMasterElement timing_master_element(clock_mode, cycle_time);
    //! [CreateFEPModule]
    fep::Result result = timing_master_element.Create(module_options);
    //! [CreateFEPModule]
    if (fep::isFailed(result))
    {
        std::cerr << "Internal Error: Failed to create Element." << std::endl;
        std::cerr << result.getErrorLabel() << ": " << result.getDescription() << std::endl;
        return 1;
    }

    return fep::isFailed(timing_master_element.WaitForShutdown()) ? 1 : 0;
}
