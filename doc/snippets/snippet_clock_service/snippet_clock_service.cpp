/**
* Implementation of an exemplary FEP Timing Client Participant which utilizes the local clock service
* to register a custom continous and discrete clock and set various main clocks
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
*
* @file
*
*/

#include <fep_participant_sdk.h>
#include <fep3/participant/default_participant.h>

#include <iostream>


using namespace fep;

/// Dummy continuous clock
class MyCustomContinuousClock : public ContinuousClock
{
public:
    MyCustomContinuousClock() :
        _current_offset(0),
        ContinuousClock("my_custom_continuous_time")
    {
    }

    timestamp_t getNewTime() const
    {
        return (a_util::system::getCurrentMilliseconds() - _current_offset);
    }

    timestamp_t resetTime()
    {
        _current_offset = a_util::system::getCurrentMilliseconds();
        return (a_util::system::getCurrentMilliseconds() - _current_offset);
    }
private:
    timestamp_t _current_offset;
};

/// Dummy discrete clock
class MyCustomDiscreteClock : public DiscreteClock
{
public:
    MyCustomDiscreteClock() :
        DiscreteClock("my_custom_discrete_time")
    {
    }

    void start(IEventSink& _sink)
    {
        DiscreteClock::start(_sink);
    }

    void stop()
    {
        DiscreteClock::stop();
    }
};

int main(int nArgc, const char* pArgv[])
{
    ParticipantFEP2 example_clock_participant;
    cModuleOptions module_options("ExampleClockParticipant", eTimingSupportDefault::timing_FEP_30);

    // Configure the participant
    // Set participant header information
    setProperty(example_clock_participant, FEP_PARTICIPANT_HEADER_VERSION, 1.0);
    setProperty(
        example_clock_participant, FEP_PARTICIPANT_HEADER_DESCRIPTION, "Example Clock Participant");
    double fFepVersion =
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) + static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    setProperty(example_clock_participant, FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
    setProperty(example_clock_participant, FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    setProperty(example_clock_participant, FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
    setProperty(example_clock_participant, FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    setProperty(
        example_clock_participant, FEP_PARTICIPANT_HEADER_TYPE_ID, "b33a2517-e7b4-4ae5-825c-c064ba40b462");

    MyCustomContinuousClock my_custom_continuous_clock;
    //! [RegisterContinuousClock]
    // Retrieve the local clock service component
    IClockService* clock_service = fep::getComponent<IClockService>(example_clock_participant);

    // Register a custom continuous clock my_custom_continuous_clock at the clock service
    clock_service->registerClock(my_custom_continuous_clock);
    //! [RegisterContinuousClock]

    //! [SetContinuousClock]
    // Set a custom clock my_custom_continuous_clock as clock by settinge the corresponding FEP property
    setProperty(example_clock_participant, FEP_CLOCKSERVICE_MAIN_CLOCK, my_custom_continuous_clock.getName());
    //! [SetContinuousClock]

    MyCustomDiscreteClock my_custom_discrete_clock;
    //! [RegisterDiscreteClock]

    // Register a custom discrete clock my_custom_discrete_clock at the clock service
    clock_service->registerClock(my_custom_discrete_clock);
    //! [RegisterDiscreteClock]

    //! [SetDiscreteClock]
    // Set a custom clock my_custom_discrete_clock as main clock using the local clock service
    clock_service->setMainClock(my_custom_discrete_clock.getName());
    //! [SetDiscreteClock]

    //! [SetupFEPTiming25]
    // Use module options to configure a FEP Participant to use the FEP 3 timing by default
    cModuleOptions module_options_30_timing("TimingMaster30Timing");
    module_options_30_timing.SetDefaultTimingSupport(eTimingSupportDefault::timing_FEP_30);
    //! [SetupFEPTiming25]
    //! [SetupFEPTiming20]
    // Use module options to configure a FEP Participant to use the FEP 2 timing by default
    cModuleOptions module_options_legacy_timing("TimingMasterLegacyTiming");
    module_options_legacy_timing.SetDefaultTimingSupport(eTimingSupportDefault::timing_FEP_20);
    //! [SetupFEPTiming20]

    //! [SetupNoTimeSynchronization]
    // Set the built-in clock *local_system_realtime* as main clock
    setProperty(example_clock_participant, FEP_CLOCKSERVICE_MAIN_CLOCK, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME);
    // Set the scheduler *clock_based_scheduler* as scheduler
    setProperty(example_clock_participant, FEP_SCHEDULERSERVICE_SCHEDULER, FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
    //! [SetupNoTimeSynchronization]

    //! [ClientSetupLocalRealTimeSynchronization]
    // Set the built-in clock *slave_master_on_demand* as main clock
    setProperty(example_clock_participant, FEP_CLOCKSERVICE_MAIN_CLOCK, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND);
    // Set the scheduler *clock_based_scheduler* as scheduler
    setProperty(example_clock_participant, FEP_SCHEDULERSERVICE_SCHEDULER, FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
    // Set *Participant 1* as timing master
    setProperty(example_clock_participant, FEP_TIMING_MASTER_PARTICIPANT, "Participant 1");
    // Optionally set the update time which defaults to 1000 ms
    setProperty(example_clock_participant, FEP_CLOCKSERVICE_SLAVE_SYNC_CYCLE_TIME, 100);
    //! [ClientSetupLocalRealTimeSynchronization]

    //! [MasterSetupExternalRealTimeSynchronization]
    // The external clock which is used by the timing master
    MyCustomContinuousClock my_custom_continuous_time;
    // Retrieve the local clock service component
    IClockService* local_clock_service = fep::getComponent<IClockService>(example_clock_participant);
    // Register the external clock *my_fancy_realtime* at the clock service
    local_clock_service->registerClock(my_custom_continuous_time);
    // Set the external clock as main clock
    // Either by setting the corresponding property
    setProperty(example_clock_participant, FEP_CLOCKSERVICE_MAIN_CLOCK, "my_custom_continuous_time");
    // Or by using the clock_service
    local_clock_service->setMainClock("my_custom_continuous_time");
    // Set the scheduler *clock_based_scheduler* as scheduler
    setProperty(example_clock_participant, FEP_SCHEDULERSERVICE_SCHEDULER, FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
    //! [MasterSetupExternalRealTimeSynchronization]

    //! [MasterSetupLocalDiscreteTimeSynchronization]
    // Set the built-in clock *local_system_simtime* as main clock
    setProperty(example_clock_participant, FEP_CLOCKSERVICE_MAIN_CLOCK, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME);
    // Set the scheduler *clock_based_scheduler* as scheduler
    setProperty(example_clock_participant, FEP_SCHEDULERSERVICE_SCHEDULER, FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
    // Optionally set the *cycle_time* property which defaults to 100 ms
    setProperty(example_clock_participant, FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME, 200);
    // Optionally set the *time_factor* property which defaults to 1,0
    setProperty(example_clock_participant, FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR, 0.5);
    //! [MasterSetupLocalDiscreteTimeSynchronization]

    //! [ClientSetupLocalDiscreteTimeSynchronization]
    // Set the built-in clock *slave_master_on_demand_discrete* as main clock
    setProperty(example_clock_participant, FEP_CLOCKSERVICE_MAIN_CLOCK, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE);
    // Set the scheduler *clock_based_scheduler* as scheduler
    setProperty(example_clock_participant, FEP_SCHEDULERSERVICE_SCHEDULER, FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
    // Set *Participant 1* as timing master
    setProperty(example_clock_participant, FEP_TIMING_MASTER_PARTICIPANT, "Participant 1");
    //! [ClientSetupLocalDiscreteTimeSynchronization]

    //! [MasterSetupExternalDiscreteTimeSynchronization]
    // The external clock which is used by the timing master
    MyCustomDiscreteClock external_player_clock;
    // Register the external clock *external_player* at the clock service
    local_clock_service->registerClock(external_player_clock);
    // Set the external clock as main clock
    // Either by setting the corresponding property
    setProperty(example_clock_participant, FEP_CLOCKSERVICE_MAIN_CLOCK, "my_custom_discrete_time");
    // Or by using the clock_service
    local_clock_service->setMainClock("my_custom_discrete_time");
    // Set the scheduler *clock_based_scheduler* as scheduler
    setProperty(example_clock_participant, FEP_SCHEDULERSERVICE_SCHEDULER, FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
    //! [MasterSetupExternalDiscreteTimeSynchronization]

    //! [ClientSetupExternalDiscreteTimeSynchronization]
    // Set the built-in clock *slave_master_on_demand_discrete* as main clock
    setProperty(example_clock_participant, FEP_CLOCKSERVICE_MAIN_CLOCK, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_SLAVE_MASTER_ONDEMAND_DISCRETE);
    // Set the scheduler *clock_based_scheduler* as scheduler
    setProperty(example_clock_participant, FEP_SCHEDULERSERVICE_SCHEDULER, FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
    // Set *Participant 1* as timing master
    setProperty(example_clock_participant, FEP_TIMING_MASTER_PARTICIPANT, "Participant 1");
    //! [ClientSetupExternalDiscreteTimeSynchronization]

    //! [SetupLegacyTiming]
    // Set the built-in clock *locked_step_simtime* as main clock
    setProperty(example_clock_participant, FEP_CLOCKSERVICE_MAIN_CLOCK, FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_MASTER_LOCKED_STEP_SIMTIME);
    // Set the legacy scheduler *locked_step_simtime_scheduler* as scheduler
    setProperty(example_clock_participant, FEP_SCHEDULERSERVICE_SCHEDULER, FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME);
    // Set *Participant 1* as timing master
    setProperty(example_clock_participant, FEP_TIMING_MASTER_PARTICIPANT, "Participant 1");
    //! [SetupLegacyTiming]

    // Create the FEP Participant
    Result result = example_clock_participant.Create(module_options);
    if (isFailed(result))
    {
        std::cerr << "Internal Error: Failed to create Element." << std::endl;
        std::cerr << result.getErrorLabel() << ": " << result.getDescription() << std::endl;
        return 1;
    }

    return isFailed(example_clock_participant.waitForShutdown()) ? 1 : 0;
}
