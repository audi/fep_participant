/**
* Declaration of the Class TimingMaster.
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

#ifndef __FEP_TIMING_TIMING_MASTER_H
#define __FEP_TIMING_TIMING_MASTER_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <set>
#include <thread>
#include <a_util/base/types.h>
#include <a_util/concurrency/semaphore.h>
#include <a_util/system/timer_decl.h>

#include "schedule_map.h"
#include "step_trigger_strategy_intf.h"
#include "fep3/components/legacy/timing/common_timing.h"
#include "fep3/components/legacy/timing/timing_master_intf.h"
#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "messages/fep_notification_listener.h"
#include "transmission_adapter/fep_data_sample.h"
#include "transmission_adapter/fep_user_data_listener_intf.h"

namespace fep
{
class IIncidentHandler;
class IPropertyTreeBase;
class IScheduleNotification;
class IStateMachine;
class IStepTrigger;
class IUserDataSample;
class ITransmissionAdapterPrivate;
class IUserDataAccessPrivate;

namespace timing
{
    class ITimingClientPrivate;

    /**
    * The timing master class
    * The timing master is responsible to control the execution of a FEP simulation
    */
	class FEP_PARTICIPANT_EXPORT TimingMaster : public ITimingMaster, public IUserDataListener, public cNotificationListener, public IStepTriggerStrategyListener
	{
	public:
		///CTOR
		TimingMaster();

		///DTOR
		~TimingMaster();
	
	public:
        /**
        * Initialize and set up internal data.
        * Initialize is called on module creation (\ref cModule::Create)
        *
        * @param [in] data_access_private pointer to private data access interface
        * @param [in] transmission_adapter_private pointer to private transmission adapter interace
        * @param [in] incident_handler pointer to incident handler interface 
        * @param [in] property_tree_base pointer to incident handler interface 
        * @param [in] timing_client_private pointer to the own timing client
        * @param [in] state_machine pointer to state machine interface
        * @returns  Standard result code.
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_INVALID_ARG One of the arguments was invalid (e.g. a NULL pointer)
        * @retval ERR_UNEXPECTED Something unexpected happend
        */
        fep::Result initialize(fep::IUserDataAccessPrivate* data_access_private,
            ITransmissionAdapterPrivate* transmission_adapter_private,
            IIncidentHandler* incident_handler,
            IPropertyTreeBase* property_tree_base,
            ITimingClientPrivate* timing_client_private,
            IStateMachine* state_machine);

        /**
        * Finalize (revert initialization)
        * Finalize is called on module destruction (\ref cModule::Destroy)
        *
        * @returns  Standard result code.
        */
        fep::Result finalize();

        /**
        * Configures according the the current system state 
        * Configure is called when the initializing state of the module is done (\ref IStateExitListener::ProcessInitializingExit)
        *
        * @returns  Standard result code.
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_INVALID_ARG One of the arguments was invalid (e.g. a NULL pointer)
        * @retval ERR_UNEXPECTED Something unexpected happend
        */
        fep::Result configure(); 

        /**
        * Reset (revert configuration)
        * Reset is called when the idle state of the module is entered (\ref IStateEntryListener::ProcessIdleEntry)
        *
        * @returns  Standard result code.
        */
        fep::Result reset();

        /**
        * Start working
        * Start is called on entering the runnning state (\ref IStateEntryListener::ProcessRunningEntry)
        *
        * @returns  Standard result code.
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_INVALID_ARG One of the arguments was invalid (e.g. a NULL pointer)
        * @retval ERR_UNEXPECTED Something unexpected happend
        */
        fep::Result start();

        /**
        * Stop working
        * Stop is called on leaving the runnning state 
        *
        * @returns  Standard result code.
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_INVALID_ARG One of the arguments was invalid (e.g. a NULL pointer)
        * @retval ERR_UNEXPECTED Something unexpected happend
        */
        fep::Result stop();

	public: // implements ITimingMaster
        /// @copydoc ITimingMaster::SetStepTrigger
        fep::Result SetStepTrigger(IStepTrigger* pStepTrigger);

    public: // implements IUserDataListener
        /// @copydoc IUserDataListener::Update
        fep::Result Update(const IUserDataSample* poSample);

    public: // overrides cNotificationListener
        /// @copydoc INotificationListener::Update
        fep::Result Update(IScheduleNotification const * pStateNotification);

    public: // Support unit tests
        ///@cond nodoc
        handle_t GetAckInputSignalHandle() const { return _ack_input_signal_handle; }
        ///@cond nodoc

    private:
        ///@cond nodoc
        bool DoNextSchedule();
        void ThreadFunc();
        fep::Result sendTickSignalToClients();
        void CheckAckTimeout();
        ///@endcond nodoc
	
    private:  // implements IStepTriggerStrategyListener
        /// @copydoc IStepTriggerStrategyListener::Trigger
        fep::Result InternalTrigger(const timestamp_t nCurrentSimulationTime);

	private:
        /// Data access pointer
        fep::IUserDataAccessPrivate* _data_access_private;
        /// pointer to the transmission adapter used by the component
        ITransmissionAdapterPrivate* _transmission_adapter_private;
        /// the incident handler used
        IIncidentHandler* _incident_handler;
        /// the property tree adapter
        IPropertyTreeBase* _property_tree_base;
        /// the own timing client
        ITimingClientPrivate* _timing_client_private;
        /// the state machine
        IStateMachine* _state_machine;
        /// is master mode enabled
        bool _is_timing_master_flag;
        /// initial schedule
        bool _is_initial_schedule_flag;
        /// ScheduleMap used
        ScheduleMap _schedule_map;
        /// Current Time
        timestamp_t _current_time;
        /// Duration since last tick
        timestamp_t _duration_since_last_tick;
        /// Collected notifications
        std::set<fep::timing::ScheduleConfig> _schedule_configs;
        /// Output: Trigger Signal
        handle_t _trigger_output_signal_handle;
        /// Trigger Sample
        cDataSample _trigger_data_sample;
        /// Input: Ack Signal
        handle_t _ack_input_signal_handle;
        /// Pointer to step trigger interface 
        IStepTriggerStrategy* _step_trigger_strategy;
        /// Pointer to step trigger interface 
        IStepTrigger* _user_step_trigger;
        /// Timing master thread
        std::unique_ptr<std::thread> _worker_thread;
        /// Thread shutdown signal
        a_util::concurrency::semaphore _shutdown_worker_semaphore;
        /// Wait for barrier
        a_util::concurrency::semaphore _completition_barrier;
        /// Wait for barrier
        a_util::concurrency::semaphore _tick_barrier;
        /// Timestamp of last received acknowledgement
#ifndef __QNX__
        std::atomic<int64_t> _last_acknowledge_received_timestamp;
#else
        std::atomic_int_fast64_t _last_acknowledge_received_timestamp;
#endif
        /// Acknowledgement wait timeout
        int64_t _acknowledge_wait_timeout;
        /// Timer checking wait timeout
        a_util::system::Timer _acknowledge_wait_timer;
    };
}
}

#endif
