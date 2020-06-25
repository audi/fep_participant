/**
* Implementation of an exemplary FEP Timing Client Participant which utilizes the local scheduler service
* to register a custom scheduler, a datajob and set an active scheduler
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
#include <fep3/components/scheduler/jobs/datajob.h>
#include <fep3/participant/default_participant.h>

#include <iostream>


/// Dummy scheduler
class MyFancyScheduler : public fep::IScheduler
{
public:
    const char* getName() const
    {
        return "my_fancy_scheduler";
    };
    fep::Result initialize(fep::IClockService& clock, fep::IScheduler::IJobConfiguration& configuration)
    {
        return fep::Result();
    };
    fep::Result start()
    {
        return fep::Result();
    };
    fep::Result stop()
    {
        return fep::Result();
    };
    fep::Result deinitialize()
    {
        return fep::Result();
    };
    std::list<IScheduler::JobInfo> getTasks() const
    {
        return std::list<IScheduler::JobInfo>();
    };
};

int main(int nArgc, const char* pArgv[])
{
    fep::ParticipantFEP2 example_scheduler_participant;
    fep::cModuleOptions module_options("ExampleSchedulerParticipant", fep::eTimingSupportDefault::timing_FEP_30);

    // Configure the participant
    // Set participant header information
    setProperty(example_scheduler_participant, FEP_PARTICIPANT_HEADER_VERSION, 1.0);
    setProperty(
        example_scheduler_participant, FEP_PARTICIPANT_HEADER_DESCRIPTION, "Example Scheduler Participant");
    double fFepVersion =
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) + static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    setProperty(example_scheduler_participant, FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
    setProperty(example_scheduler_participant, FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    setProperty(example_scheduler_participant, FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
    setProperty(example_scheduler_participant, FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    setProperty(
        example_scheduler_participant, FEP_PARTICIPANT_HEADER_TYPE_ID, "b33a2517-e7b4-4ae5-825c-c064ba40b462");

    MyFancyScheduler my_fancy_scheduler;
    //! [RegisterScheduler]
    // Retrieve the local scheduler service component
    fep::ISchedulerService* scheduler_service = fep::getComponent<fep::ISchedulerService>(example_scheduler_participant);

    // Register a custom scheduler my_fancy_scheduler at the scheduler service
    scheduler_service->registerScheduler(my_fancy_scheduler);
    //! [RegisterScheduler]

    //! [SetScheduler]
    // Set a custom scheduler my_fancy_scheduler as active scheduler by settinge the corresponding FEP property
    setProperty(example_scheduler_participant, FEP_SCHEDULERSERVICE_SCHEDULER, my_fancy_scheduler.getName());
    //! [SetScheduler]

    //! [RegisterJob]
    // Create a datajob
    timestamp_t cycle_sim_time_us = 100;        //ms
    timestamp_t first_delay_sim_time_us = 0;    //ms
    timestamp_t max_runtime_real_time_us = 0;   //ms
    timestamp_t max_waiting_time_real_us = 0;   //ms
    fep::JobConfiguration my_job_configuration(cycle_sim_time_us,
        first_delay_sim_time_us,
        max_runtime_real_time_us,
        max_waiting_time_real_us,
        fep::JobConfiguration::TS_IGNORE_RUNTIME_VIOLATION);
    fep::DataJob my_data_job("my_data_job", my_job_configuration);

    // Register datajob 'my_data_job' at the local scheduler service
    my_data_job.addToComponents(example_scheduler_participant.getComponents());

    // Create a second datajob
    fep::DataJob my_data_job_2("my_data_job_2", my_job_configuration);
    std::shared_ptr<fep::DataJob> my_element(&my_data_job_2);

    // Register element 'my_element' which contains 'my_data_job_2' at the local scheduler service
    // when creating the FEP Participant
    fep::Result result = example_scheduler_participant.Create(module_options, my_element);
    //! [RegisterJob]

    // Create the FEP Participant
    if (fep::isFailed(result))
    {
        std::cerr << "Internal Error: Failed to create Element." << std::endl;
        std::cerr << result.getErrorLabel() << ": " << result.getDescription() << std::endl;
        return 1;
    }

    return fep::isFailed(example_scheduler_participant.waitForShutdown()) ? 1 : 0;
}
