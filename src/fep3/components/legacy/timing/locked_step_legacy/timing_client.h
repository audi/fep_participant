/**
* Declaration of the Timing Client.
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

#ifndef __FEP_TIMING_TIMING_CLIENT_H
#define __FEP_TIMING_TIMING_CLIENT_H

#include <a_util/base/types.h>
#include <a_util/system/timer_decl.h>
#include <stdint.h>
#include <atomic>
#include <map>
#include <string>
#include "fep3/components/legacy/timing/common_timing.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "messages/fep_command_listener.h"
#include "transmission_adapter/fep_user_data_listener_intf.h"

namespace fep
{
    // forward decl
    class IGetScheduleCommand;
    class IIncidentHandler;
    class IPropertyTreeBase;
    class ISignalRegistryPrivate;
    class IStateMachine;
    class ITransmissionAdapterPrivate;
    class IUserDataAccessPrivate;
    class IUserDataSample;
    class cScheduleList;

namespace timing
{
    // forward decl
	class Task;

    /**
    * The \ref ITimingClientPrivate interface is the internal interface to the
    * functionality of \ref TimingClient.
    * The interface is only used internally in this case only by the timing 
    * master \ref TimingMaster.
    */
    class FEP_PARTICIPANT_EXPORT ITimingClientPrivate
    {
    public:
        /**
        * DTOR
        */
        virtual ~ITimingClientPrivate() = default;

        /**
        * Fill schedule list.
        *
        * @param [in] scheduleList schedule list to fill
        */
        virtual void FillScheduleList(cScheduleList& scheduleList) = 0;
    };

    /**
    * The timing client class
    */
    class FEP_PARTICIPANT_EXPORT TimingClient : public fep::ITiming, public IUserDataListener
		, public cCommandListener, public ITimingClientPrivate
    {
	private:
        /// Internal task map
        typedef std::map<std::string,Task*> TaskMap;

    public:
        ///CTOR
		TimingClient();

        ///DTOR
        ~TimingClient();

    public: // implements ITiming
        /// @copydoc ITiming::RegisterStepListener
        fep::Result RegisterStepListener(const char* strStepListenerName, const StepConfig& tConfiguration, ScheduleFunc pCallback, void* pCallee);

        /// @copydoc ITiming::UnregisterStepListener
        fep::Result UnregisterStepListener(const char* strStepListenerName);
        
        /// @copydoc ITiming::GetTime
        timestamp_t GetTime() const;

        /// @copydoc ITiming::SetSystemTimeout
        fep::Result SetSystemTimeout(const timestamp_t tmSystemTimeout);

	public:
        /**
        * Initialize and set up internal data.
        * Initialize is called on module creation (\ref cModule::Create)
        *
        * @param [in] data_access_private pointer to private data access interface
        * @param [in] signal_registry_private pointer to private signal registry interface
        * @param [in] transmission_adapter_private pointer to private transmission adapter interface
        * @param [in] state_machine pointer to state machine interface
        * @param [in] incident_handler pointer to incident handler interface
        * @param [in] property_tree_base pointer to incident handler interface
        * @returns  Standard result code.
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_INVALID_ARG One of the arguments was invalid (e.g. a NULL pointer)
        * @retval ERR_UNEXPECTED Something unexpected happend
        */
        fep::Result initialize(fep::IUserDataAccessPrivate* data_access_private, fep::ISignalRegistryPrivate* signal_registry_private,
            fep::ITransmissionAdapterPrivate* transmission_adapter_private, fep::IStateMachine* state_machine, fep::IIncidentHandler* incident_handler,
            fep::IPropertyTreeBase* property_tree_base);

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

    public: // unit test support
        ///@cond nodoc
        handle_t GetTriggerInputSignalHandle() const { return m_trigger_input_signal_handle; }
        ///@endcond nodoc

    private:
        ///@cond nodoc
        // Internal support methods
        fep::Result configureTaskMap(std::map<std::string, StepConfig>& taskMap);
        fep::Result configureInputSet(std::map<std::string, GlobalInputConfig>& inputMap);
        fep::Result configureSingleTask(const std::string &name, fep::StepConfig &configuration);
        ///@endcond nodoc

	private: // implements IUserDataListener
        /// @copydoc IUserDataListener::Update
		fep::Result Update(IUserDataSample const* poSample);

    private: // implements/overwrites cCommandListener
        /// @copydoc ICommandListener::Update(IGetScheduleCommand const*)
		fep::Result Update(IGetScheduleCommand const* poCommand);

        // @copydoc ITimingClientPrivate::FillScheduleList
        void FillScheduleList(cScheduleList& scheduleList);

    private: // timer callback function
        /// @cond nodoc
        void CheckSystemTimeout();
        /// @endcond nodoc

    private:
        /// Data access pointer
        fep::IUserDataAccessPrivate* m_data_access_private;
        /// pointer to the signal registry used by the component
        fep::ISignalRegistryPrivate* m_signal_registry_private;
        /// pointer to the transmission adapter used by the component
        fep::ITransmissionAdapterPrivate* m_transmission_adapter_private;
        /// pointer to the state machine used by the component
        fep::IStateMachine* m_state_machine;
        /// the incident handler used
        fep::IIncidentHandler* m_incident_handler;
        /// the property tree adapter
        fep::IPropertyTreeBase* m_property_tree_base;
        /// signal handle for ack output signal
        handle_t m_ack_output_signal_handle;
        /// signal handle for trigger input signal
        handle_t m_trigger_input_signal_handle;
        /// current simulation time
        timestamp_t m_current_simulation_time;
        /// locally computed simulation time
        timestamp_t m_time_progress_sum;
        /// running flag
        bool m_timing_client_running_flag;
        /// the task map
        TaskMap m_task_map;
        /// the name of the configured timing master (read out of property tree during initialization)
        std::string m_master_name;
        /// the time counter since last trigger was received
#ifndef __QNX__
        std::atomic<int64_t> m_last_trigger_received;
#else
        std::atomic_int_fast64_t m_last_trigger_received;
#endif
        /// the trigger wait timeout
        int64_t m_system_timeout;
        /// the trigger timeout timer
        a_util::system::Timer m_trigger_wait_timer;
        /// bool flag indicating whether timeout happened
        bool m_is_timeout;
        /// bool flag indicating whether to use multicast or not
        bool m_use_multicast;
    };
}
}

#endif // __FEP_TIMING_TIMING_CLIENT_H
