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

#ifndef __FEP_DATAREGISTRY_CONFIGURATION_H
#define __FEP_DATAREGISTRY_CONFIGURATION_H

#include <string>
#include "fep_types.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"

namespace fep
{
    /// Strategy enum configuring behaviour in case of a detected causality error (i.e. no valid samples exist
    /// for a configured input
    //enum DataInputViolationStrategy
    //{
    //    /// dummy value
    //    IS_UNKNOWN = 0,
    //    /// Causality violations will be ignored
    //    IS_IGNORE_INPUT_VALIDITY_VIOLATION,
    //    /// A warning incident will be published in case of a causality violation
    //    IS_WARN_ABOUT_INPUT_VALIDITY_VIOLATION,
    //    /// Configured outputs will not be published when a causality violation is detected
    //    IS_SKIP_OUTPUT_PUBLISH,
    //    /// Causality violation is fatal and the step listener will set the state machine to error state
    //    IS_SET_STM_TO_ERROR
    //};
    using DataInputViolationStrategy = InputViolationStrategy;
 
    struct FEP_PARTICIPANT_EXPORT DataInputConfiguration
    {
        /// CTOR
        DataInputConfiguration() : _name(std::string()),
            _valid_age_sim_us(0),
            _delay_sim_us(0),
            _input_violation_strategy(DataInputViolationStrategy::IS_UNKNOWN)
        {
        }
        DataInputConfiguration(std::string name, timestamp_t valid_age, timestamp_t delay, DataInputViolationStrategy strat) :
            _name(std::move(name)),
            _valid_age_sim_us(valid_age),
            _delay_sim_us(delay),
            _input_violation_strategy(strat)
        {
        }

        std::string _name;
        /// The maximum valid age for a sample of the input (simulation time)
        timestamp_t _valid_age_sim_us;
        /// The virtual delay that is applied to samples for this input (simulation time)
        timestamp_t _delay_sim_us;
        /// The strategy to be used when no valid samples exist in the sample backlog when a trigger is received
        DataInputViolationStrategy _input_violation_strategy;
    };

    /// The configuration of an output for the step data access
    struct FEP_PARTICIPANT_EXPORT DataOutputConfiguration
    {
        std::string _name;
    };
}

#endif // __FEP_DATAREGISTRY_CONFIGURATION_H
