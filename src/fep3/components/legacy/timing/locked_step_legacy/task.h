/**
* Implementation of the Class Task.
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

#if !defined(__FEP_TIMING_TASK_H)
#define __FEP_TIMING_TASK_H

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <a_util/base/types.h>
#include <a_util/concurrency/semaphore.h>

#include "fep3/components/legacy/timing/common_timing.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep_participant_export.h"
#include "fep_result_decl.h"

namespace fep
{
class IIncidentHandler;
class IPreparationDataSample;
class IStateMachine;
class IStepDataAccess;
class IStepDataAccessPrivate;
class ITransmissionAdapterPrivate;

namespace timing
{
    /**
    * Task class
    * A task is the execution unit for steps
    */
    class FEP_PARTICIPANT_EXPORT Task
	{
	public:
		///CTOR
        /// @param[in] name the task name
        /// @param[in] uuid the task uuid
        /// @param[in] data_access_private pointer to data access
        /// @param[in] state_machine pointer to state machine
        /// @param[in] incident_handler pointer to incident handler
        Task(const std::string& name, const std::string& uuid,
            fep::IStepDataAccessPrivate* data_access_private,
            fep::ITransmissionAdapterPrivate* transmission_adapter_private,
            fep::IStateMachine* state_machine,
            fep::IIncidentHandler* incident_handler);

        ///DTOR
        ~Task();

	public:
        /**
        * Configure the task
        *
        * @param [in] step_config step coniguration, part of the timing configuration
        * @returns  Standard result code.
        */
        fep::Result configure(const timing::StepConfig &step_config);

        /**
        * Progress simulation
        *
        * @param [in] time_progressSum syntetic simulation time starting at zero (accumulated simulation time)
        * @param [in] curr_sim_time real simulation time
        */
        void simTimeProgress(const timestamp_t time_progressSum, const timestamp_t curr_sim_time);

        /**
        * Set schedule func
        * Used by \ref ITiming::RegisterStepListener
        *
        * @param [in] callback callback function pointer
        * @param [in] callee callback function pointer (usually called user data)
        * @returns  Standard result code.
        */
        fep::Result setScheduleFunc(ITiming::ScheduleFunc callback, void* callee);

        /**
        * Create the task
        * Start the associated thread 
        *
        * @param [in] ack_handle signal handle for acknowledgement samples
        */
        void create(handle_t ack_handle);

        /**
        * Destroy the task
        * Stop the associated thread
        */
        void destroy();

    public: // getter methods
        /**
        * Get current cycle time
        *
        * @return the current cycle time
        */
        timestamp_t getCycleTime() { return m_cycle; }

        /**
        * Get the task name
        * The task is named by \ref ITiming::RegisterStepListener
        *
        * @return the task name
        */
        const std::string& getName() { return m_name; }

        /**
        * Get the task uuid
        * The task uuid is internally created and used to identify the task.
        *
        * @return the task uuid
        */
        const std::string& getUuid() { return m_uuid; }

        /**
         * Returns bool value whether the task is currently computing.
         *
         * @return Bool flag whether the task is working
         */
        bool isTaskWorking();

	private:
        ///@cond nodoc
        void compute(timestamp_t sim_time_for_step);
        void ThreadFunc();
        void applyTriggerViolation();
        fep::Result applyTimeViolationStrat();
        fep::Result sendAcknowledgement(timestamp_t used_time, timestamp_t sim_time_for_this_step);
        ///@endcond nodoc

    private:
        ///@cond nodoc
        std::unique_ptr<std::thread> m_worker_thread;
		a_util::concurrency::semaphore m_barrier;
        std::mutex m_task_working_mutex;
		a_util::concurrency::semaphore m_thread_shutdown_semaphore;
        timestamp_t m_current_simulation_time;
        ITiming::ScheduleFunc m_callback;
		void* m_instance;
        std::string m_name;
		std::string m_uuid;
		timestamp_t m_cycle;
		timestamp_t m_operational;
		timing::TimeViolationStrategy m_time_violation_strategy;
		fep::IStepDataAccessPrivate* m_data_access_private;
        fep::IStepDataAccess*   m_step_data_for_user;
        fep::ITransmissionAdapterPrivate* m_transmission_adapter_private;
        fep::IStateMachine* m_state_machine;
        fep::IIncidentHandler* m_incident_handler;
		fep::IPreparationDataSample* m_ack_data_sample;
        ///@endcond nodoc
    };
}
}

#endif
