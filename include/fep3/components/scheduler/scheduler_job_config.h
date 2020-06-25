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

#ifndef __FEP_SCHEDULER_JOB_CONFIGURATION_H
#define __FEP_SCHEDULER_JOB_CONFIGURATION_H

#include <map>
#include <string>
#include "fep_types.h"

namespace fep
{
    class FEP_PARTICIPANT_EXPORT JobConfiguration
    {
        public:
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

    public:
        /**
        * CTOR
        * @param [in] cycle_sim_time_us            The cycle time to be used for the step listener (simulation time)
        * @param [in] delay_sim_time_us            The cycle delay time to the 0 point of the time base (simulation time)
        * @param [in] max_runtime_real_time_us     The maximum duration that the one job execution is expected to need for computation (real time)
        * @param [in] max_waiting_time_real_us     The max. waiting time for inputs (This is a legacy parameter which will not be considered anymore via programming API since 2.3! 
        *                                                                            If you want to use that please contact the support for your usecase or use the "old timing configuration file"!) 
        * @param [in] runtimeViolationStrategy     The violation strategy
        */
        JobConfiguration(timestamp_t cycle_sim_time_us,
                         timestamp_t first_delay_sim_time_us = 0, 
                         timestamp_t max_runtime_real_time_us = 0,
                         timestamp_t max_waiting_time_real_us = 0,
                         TimeViolationStrategy runtime_violation_strategy = TS_IGNORE_RUNTIME_VIOLATION,
                         const char* dependencies = "")
            : _cycle_sim_time_us(cycle_sim_time_us),
              _delay_sim_time_us(first_delay_sim_time_us),
              _max_runtime_real_time_us(max_runtime_real_time_us),
              _runtime_violation_strategy(runtime_violation_strategy),
              _dependencies(dependencies)
        {
        }

    public:
        /// The cycle time to be used for the step listener (simulation time)
        timestamp_t                         _cycle_sim_time_us;
        /// The cycle delay time to the 0 point of the time base (simulation time)
        timestamp_t                         _delay_sim_time_us;
        /// The maximum duration that the one job execution is expected to need for computation (real time)
        timestamp_t                         _max_runtime_real_time_us;
        /// The strategy that will be applied in case of a longer computation time than expected
        TimeViolationStrategy	            _runtime_violation_strategy;
        /// comma separated list of jobs this job depends on
        std::string                         _dependencies;
    };

    
}

#endif // __FEP_SCHEDULER_JOB_CONFIGURATION_H
