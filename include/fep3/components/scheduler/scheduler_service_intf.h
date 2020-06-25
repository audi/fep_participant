/**
* Declaration of the Class ISchedulerService.
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

#ifndef __FEP_SCHEDULER_SERVICE_INTF_H
#define __FEP_SCHEDULER_SERVICE_INTF_H

#include "fep3/components/scheduler/scheduler_job_config.h"
#include "fep3/components/clock/clock_service_intf.h"

/**
 * Main property entry of scheduler service 
 */
#define FEP_SCHEDULERSERVICE "Scheduling"
/**
* @brief The scheduler configuration node
* @see @ref page_fep_timing_3
*
*/
#define FEP_SCHEDULERSERVICE_SCHEDULER FEP_SCHEDULERSERVICE".Scheduler"
/**
* @brief Name of the FEP 3 Timing native scheduler implementation.
* @see @ref fep_timing_3
*
*/
#define FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED                 "clock_based_scheduler"
/**
* @brief Name of the FEP Timing 2 legacy scheduler.
* @see @ref fep_timing_2
* @see @ref fep_timing_3
*
*/
#define FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME  "locked_step_simtime_scheduler" 
/**
 * @brief Compatibility mode to be compatible to FEP SDK versions < 2.3.0 regarding transmission and reception of signals.
 *
 * Enabling the compatibility mode for a FEP participant influences the way signals are sent to and received from the FEP system.
 * @see @ref fep_compatibility
 *
 */
#define FEP_SCHEDULERSERVICE_FEP22_COMPATIBILITY_MODE_ENABLED FEP_SCHEDULERSERVICE".bFEP22CompatibilityModeEnabled"

namespace fep
{
/**
* @brief Interface for a Scheduler
* @see @ref page_fep_timing_3
*
*/
class FEP_PARTICIPANT_EXPORT IScheduler
{
    public:
        /**
        * @brief JobInfo contains the name of a job and the corresponding job configuration!
        * Usually JobInfos are used to retrieve information regarding jobs registered at the local scheduler service.
        */
        class FEP_PARTICIPANT_EXPORT JobInfo
        {
            public:
                /**
                * @brief JobInfo Constructor taking the minimum configuration parameters
                *
                * @param name name of the job
                * @param cycle_time_us cycle time of the job
                */
                JobInfo(std::string name, timestamp_t cycle_time_us);

                /**
                * @brief JobInfo Constructor taking the job name and a JobConfiguration
                *
                * @param name name of the job
                * @param configuration configuration of the job
                */
                JobInfo(std::string name, JobConfiguration configuration);

                /**
                * @brief Destroy the JobInfo object
                *
                */
                ~JobInfo() = default;

                /**
                * @brief Get the Name of the JobInfo
                *
                * @return const char* the pointer to the null terminated string with the name
                */
                const char* getName() const;

                /**
                * @brief Get the JobConfiguration of the JobInfo
                *
                * @return fep::JobConfiguration
                */
                const JobConfiguration& getConfig() const;

            private:
                std::string _name;
                JobConfiguration _configuration;
        };

        /**
        * @brief IJob Interface providing three methods for data reception, processing and transmission
        */
        class FEP_PARTICIPANT_EXPORT IJob
        {
            protected:
                /**
                * @brief Destroy the IJob object
                *
                */
                virtual ~IJob() = default;
            public:
                /**
                * @brief Method for data reception
                *
                * @param time_of_execution timestamp when the method is triggered
                */
                virtual fep::Result executeDataIn(timestamp_t time_of_execution) = 0;
                /**
                * @brief Method for data processing
                *
                * @param time_of_execution timestamp when the method is triggered
                */
                virtual fep::Result execute(timestamp_t time_of_execution) = 0;
                /**
                * @brief Method for data transmission.
                *
                * @param time_of_execution timestamp when the method is triggered
                */
                virtual fep::Result executeDataOut(timestamp_t time_of_execution) = 0;
        };

        class FEP_PARTICIPANT_EXPORT IJobConfiguration
        {
            protected:
                ~IJobConfiguration() = default;
            public:
                /**
                * @brief Get the jobs and corresponding JobInfos which are registered at the local scheduler service
                *
                * @return std::list<std::pair<IJob*, JobInfo>>      List of jobs and JobInfos registered at the local scheduler service
                */
                virtual std::list<std::pair<IJob*, JobInfo>> getJobConfig() const = 0;
        };

        /**
        * @brief Destroy the IScheduler object
        *
        */
        virtual ~IScheduler() = default;

        /**
        * @brief Get the Name of the scheduler
        *
        * @return const char* the pointer to the null terminated string with the name
        */
        virtual const char* getName() const = 0;

        /**
        * The method \ref initialize initializes the given \p scheduler.
        * The native implementation of the local_clock_based_scheduler uses the local clock service
        * and configurations of every registered job to create a timer thread for every job which
        * triggers the job at the configured cycle times.
        * @param clock                      The local clock service
        * @param configuration              The configurations of all jobs registered at the local scheduler service
        * @returns                          Standard result code
        * @retval ERR_NOERROR               Everything went fine
        */
        virtual fep::Result initialize(IClockService& clock,
                                        IJobConfiguration& configuration) = 0;

        /**
        * The method \ref start starts the given \p scheduler.
        * The native implementation of the local_clock_based_scheduler starts every job timer thread
        * and the scheduler service thread.
        * @returns                          Standard result code
        * @retval ERR_NOERROR               Everything went fine
        */
        virtual fep::Result start() = 0;

        /**
        * The method \ref stop stops the given \p scheduler.
        * The native implementation of the local_clock_based_scheduler stops the scheduler service thread
        * and every job timer thread.
        * @returns                          Standard result code
        * @retval ERR_NOERROR               Everything went fine
        */
        virtual fep::Result stop() = 0;

        /**
        * The method \ref deinitialize deinitialize the given \p scheduler.
        * The native implementation of the local_clock_based_scheduler stops the scheduler service thread,
        * every job timer thread and clears all job timer threads.
        * @returns                          Standard result code
        * @retval ERR_NOERROR               Everything went fine
        */
        virtual fep::Result deinitialize() = 0;
        virtual std::list<IScheduler::JobInfo> getTasks() const = 0;
};

/**
* @brief Interface of the scheduler service of the participant
* The scheduler service may be used to register jobs and schedulers of a participant. A registered scheduler may be set as active scheduler for the participant.
* Registered jobs may be triggered by the active scheduler.
*
*/
class FEP_PARTICIPANT_EXPORT ISchedulerService
{
    public:
        /**
        * @brief Defintion of the scheduler service component ID
        * @ref page_components
        */
        FEP_COMPONENT_IID("ISchedulerService");

    protected:
        /**
        * @brief Destroy the ISchedulerService object
        *
        */
        virtual ~ISchedulerService() = default;

    public:
        /**
        * The method \ref registerScheduler registers the given \p scheduler at the local scheduler service.
        * Registered schedulers may be set as active scheduler and trigger registered jobs.
        * @param scheduler                  The scheduler to be registered at the scheduler service
        * @returns                          Standard result code
        * @retval ERR_NOERROR               Everything went fine
        * @retval ERR_INVALID_STATE         The Participant is already running
        * @retval ERR_INVALID_ARG           Scheduler with the given name is already registered at the local scheduler service
        */
        virtual fep::Result registerScheduler(IScheduler& scheduler) = 0;

        /**
        * The method \ref unregisterScheduler unregisters the scheduler with the given \p scheduler_name from the local scheduler service.
        * @param scheduler_name             The name of the scheduler to be unregistered from the scheduler service
        * @returns                          Standard result code
        * @retval ERR_NOERROR               Everything went fine
        * @retval ERR_INVALID_STATE         The Participant is already running
        * @retval ERR_NOT_FOUND             Scheduler with the given is not registered at the local scheduler service
        */
        virtual fep::Result unregisterScheduler(const char* scheduler_name) = 0;

        /**
        * The method \ref getSchedulerList returns the names of all registered schedulers.
        * @return std::list<std::string>    List containing the names of all registered schedulers                        
        */
        virtual std::list<std::string> getSchedulerList() const = 0;

        /**
        * The method \ref addJob registers the given \p job at the local scheduler service.
        * @param name                       The name of the job to be registered at the scheduler service
        * @param job                        The job to be registered at the scheduler service
        * @param job_config                 The job configuration of the job to be registered at the scheduler service
        * @returns                          Standard result code
        * @retval ERR_NOERROR               Everything went fine
        * @retval ERR_INVALID_ARG           A job with the given name is already registered at the scheduler service
        */
        virtual fep::Result addJob(const char* name, IScheduler::IJob& job, const fep::JobConfiguration& job_config) = 0;

        /**
        * The method \ref removeJob unregisters the job with the given \p name from the local scheduler service.
        * @param name                       The name of the job to be unregistered from the scheduler service
        * @returns                          Standard result code
        * @retval ERR_NOERROR               Everything went fine
        * @retval ERR_INVALID_ARG           A job with the given name is not registered at the scheduler service
        */
        virtual fep::Result removeJob(const char* name) = 0;

        /**
        * The method \ref getJobs returns the job infos of all registered jobs.
        * @return std::list<IScheduler::JobInfo>    List containing job infos of all registered jobs
        */
        virtual std::list<IScheduler::JobInfo> getJobs() const = 0;

        /**
        * The method \ref setScheduler sets the scheduler with the given \p scheduler_name as active scheduler.
        * @param scheduler_name             The name of the scheduler to be set as active scheduler
        * @returns                          Standard result code
        * @retval ERR_NOERROR               Everything went fine
        * @retval ERR_NOT_FOUND             A scheduler with the given name is not registered at the scheduler service
        */
        virtual fep::Result setScheduler(const char* scheduler_name) = 0;

        /**
        * The method \ref getScheduler returns the scheduler with the given \p scheduler_name.
        * Returns the current active scheduler if \p scheduler_name is empty.
        * @param scheduler_name             The name of the scheduler to be returned
        * @return IScheduler*               The scheduler with the given name
        */
        virtual const IScheduler* getScheduler(const char* scheduler_name) const = 0;

        virtual std::list<IScheduler::JobInfo> getTasks(const char* scheduler_name) const = 0;
};
}

#endif // __FEP_SCHEDULER_SERVICE_INTF_H