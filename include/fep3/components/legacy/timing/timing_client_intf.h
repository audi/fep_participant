/**
* Declaration of the Class ITiming.
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

#ifndef __FEP_TIMING_CLIENT_INTF_H
#define __FEP_TIMING_CLIENT_INTF_H

#include <map>
#include <string>
#include "fep_types.h"

namespace fep
{
    /// Strategy enum configuring behaviour in case of an operational time violation
    enum TimeViolationStrategy
    {
        /// dummy value
        TS_UNKNOWN = 0,
        /// Time violations are ignored
        TS_IGNORE_RUNTIME_VIOLATION,
        /// A warning incident will be published when an operational time violation is detected
        TS_WARN_ABOUT_RUNTIME_VIOLATION,
        /// Configured outputs will not be published when an operational time violation is detected
        TS_SKIP_OUTPUT_PUBLISH,
        /// The step listener will abort and set the state machine to error state
        TS_SET_STM_TO_ERROR
    };

    /// Strategy enum configuring behaviour in case of a detected causality error (i.e. no valid samples exist
    /// for a configured input
    enum InputViolationStrategy
    {
        /// dummy value
        IS_UNKNOWN = 0,
        /// Causality violations will be ignored
        IS_IGNORE_INPUT_VALIDITY_VIOLATION,
        /// A warning incident will be published in case of a causality violation
        IS_WARN_ABOUT_INPUT_VALIDITY_VIOLATION,
        /// Configured outputs will not be published when a causality violation is detected
        IS_SKIP_OUTPUT_PUBLISH,
        /// Causality violation is fatal and the step listener will set the state machine to error state
        IS_SET_STM_TO_ERROR
    };

    /// The configuration of an input for the step data access
    struct InputConfig
    {
        /// CTOR
        InputConfig()
            : m_handle(nullptr)
            , m_validAge_sim_us(0)
            , m_delay_sim_us(0)
            , m_inputViolationStrategy(IS_UNKNOWN)
        {}
        InputConfig(handle_t handle, timestamp_t age, timestamp_t delay, fep::InputViolationStrategy strat)
            : m_handle(handle)
            , m_validAge_sim_us(age)
            , m_delay_sim_us(delay)
            , m_inputViolationStrategy(strat)
        {}
        /// The handle of the input (is filled automatically during configuration)
        handle_t m_handle;
        /// The maximum valid age for a sample of the input (simulation time)
        timestamp_t m_validAge_sim_us;
        /// The virtual delay that is applied to samples for this input (simulation time)
        timestamp_t m_delay_sim_us;
        /// The strategy to be used when no valid samples exist in the sample backlog when a trigger is received
        InputViolationStrategy m_inputViolationStrategy;
    };

    /// The configuration of an output for the step data access
    struct OutputConfig
    {
        /// The handle of the output signal (is filled automatically during configuration)
        handle_t m_handle;
    };

    /// Typedef for the map used for configuring inputs
    typedef std::map<std::string, InputConfig> InputMap;
    /// Typedef for the map used for configuring outputs
    typedef std::map<std::string, OutputConfig> OutputMap;

    /// The class used to configure a step listener.
    class StepConfig
    {
    public:
        /**
         * CTOR
         * @param [in] cycle_time_us                The step cycle time
         * @param [in] max_operational_real_us      The max. runtime
         * @param [in] max_waiting_time_real_us     The max. waiting time for inputs (This is a legacy parameter which will not be considered anymore via programming API since 2.3! 
        *                                                                             If you want to use that please contact the support for your usecase or use the "old timing configuration file"!) 
         * @param [in] runtimeViolationStrategy     The violation strategy
         */
        StepConfig(timestamp_t cycle_time_us, 
            timestamp_t max_operational_real_us = 0,
            timestamp_t max_waiting_time_real_us = 0,
            TimeViolationStrategy runtimeViolationStrategy = TS_IGNORE_RUNTIME_VIOLATION)
            : m_cycleTime_sim_us(cycle_time_us)
            , m_maxRuntime_us(max_operational_real_us)
            , m_maxInputWaittime_us(max_waiting_time_real_us)
            , m_runtimeViolationStrategy(runtimeViolationStrategy)
            , m_inputs()
        { }

    public:
        /// The cycle time to be used for the step listener (simulation time)
        timestamp_t                         m_cycleTime_sim_us;
        /// The maximum duration that the step listener is expected to need for computation (real time)
        timestamp_t                         m_maxRuntime_us;
        /// The maximum time the step listener will wait for a valid input sample after receiving a trigger (real time)
        /// This is a legacy memaber which will not be considered anymore via programming API since 2.3 ! If you want to use that please contact the support for your usecase or use the "old timing configuration file"!)
        timestamp_t                         m_maxInputWaittime_us;
        /// The strategy that will be applied in case of a longer computation time than expected
        TimeViolationStrategy	            m_runtimeViolationStrategy;
        /// Map of all configured inputs for the step listener (is forwarded to the step data access)
        InputMap    m_inputs;
        /// Map of all configured outputs for the step listener (is forwarded to the step data access)
        OutputMap   m_outputs;
    };

    // forward decl
    class IStepDataAccess;

    /// Interface for accessing the Timing Client component
	class FEP_PARTICIPANT_EXPORT ITiming
	{
	public:
        /**
         * Typedefinition of the signature to be used by the callback method that will be called when a step listener is triggered
         * @param [in] pCallee           The user data pointer which is forwarded when registering a step listener
         * @param [in] timestamp_t      The simulation time of the current step
         * @param [in] IStepDataAccess  The pointer to the data access to be used inside of the step listener
         */
        typedef void (*ScheduleFunc) (void* pCallee, timestamp_t, IStepDataAccess*);

     public:
        // DTOR
       virtual ~ITiming() = default;
        
	    /**
         * Returns the current simulation time
         * @retval Current simulation time
         */
        virtual timestamp_t GetTime() const = 0;

        /**
         * Sets the timeout value to wait for trigger reception from the Timing Master before entering \ref FS_ERROR state.
         * A timeout value of zero disables the system timeout.
         *
         * @param[in] tmSystemTimeout The timeout value in seconds
         * @deprecated This function is obsolete and might be removed in future versions,
         *             use the respective property inside the property tree
         * @retval Standard result code
         */
        virtual fep::Result SetSystemTimeout(const timestamp_t tmSystemTimeout) = 0;


        /**
         * The method \ref RegisterStepListener registers a simulation step listener. The given ScheduleFunc will be called cyclically 
         * in accordance to the simulation time cycle given in the configuration.
         * @param [in] strStepListenerName The name of the step listener
         * @param [in] tConfiguration      The configuration of the step listener
         * @param [in] pCallback           The callback method to be used when the step listener is triggered
         * @param [in] pCallee             The user data pointer which will be forwarded to the callback method
         * @returns                        Standard result code
         * @retval ERR_NOERROR             Everything went fine
         * @retval ERR_INVALID_STATE       The Timing Client is already running
         * @retval ERR_INVALID_ARG         Invalid configuration arguments were given
         * @retval ERR_NOT_FOUND           Required input signal is not registered or timing configuration does not contain the participant
         * @retval ERR_INVALID_ADDRESS     A step listener with this name already exists
         */
        virtual fep::Result RegisterStepListener(const char* strStepListenerName, const StepConfig& tConfiguration, ScheduleFunc pCallback, void* pCallee) = 0;

        /**
         * The method \ref UnregisterStepListener unregisters a simulation step listener.
         * @param [in] strStepListenerName The name of the registered step listener.
         * @retval Standard result code
         */
        virtual fep::Result UnregisterStepListener(const char* strStepListenerName) = 0;
    };

}

#endif // __FEP_TIMING_CLIENT_INTF_H
