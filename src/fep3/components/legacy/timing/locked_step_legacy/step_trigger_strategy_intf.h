/**
* Declaration of the Class IStepTrigger
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

#ifndef __FEP_TIMING_STEP_TRIGGER_STRATEGY_INTF_H
#define __FEP_TIMING_STEP_TRIGGER_STRATEGY_INTF_H

#include <a_util/base/types.h>
#include "fep_participant_export.h"
#include "fep_result_decl.h"

namespace fep
{   
    namespace timing
    {
        /**
    * Listener interface used to register at \ref IStepTrigger
    */
        class FEP_PARTICIPANT_EXPORT IStepTriggerStrategyListener
        {
        public:
            /// DTOR
            virtual ~IStepTriggerStrategyListener() = default;

        public:
            /**
            * InternalTrigger next step
            *
            * @param[in] nCurrentSimulationTime current simualtion time for this step
            * @returns Standard result code.
            */
            virtual fep::Result InternalTrigger(const timestamp_t nCurrentSimulationTime) = 0;
        };

        /**
        * Step trigger interface
        * The step trigger is responsible to progress the simulation.
        * The step trigger calls the trigger method \ref IStepTriggerListener::Trigger
        */
        class FEP_PARTICIPANT_EXPORT IStepTriggerStrategy
        {
        public:
            /// DTOR
            virtual ~IStepTriggerStrategy() = default;

        public:
            /**
            * Register a trigger listener
            * Only one step trigger listener must be supported. If more step triggers listeners are
            * registered the method would return an error.
            *
            * @param[in] nCycleTime requested cycle time
            * @param[in] step_trigger_strategy_listener pointer to the listener
            * @returns Standard result code.
            * @retval ERR_NOERROR Everything went fine
            * @retval ERR_UNEXPECTED Registration failed. Usually there is already a listener registered.
            */
            virtual fep::Result RegisterStrategyTrigger(const timestamp_t nCycleTime, IStepTriggerStrategyListener* step_trigger_strategy_listener) = 0;

            /**
            * Unregister a trigger listener
            *
            * @param[in] step_trigger_strategy_listener pointer to the listener
            * @returns Standard result code.
            * @retval ERR_NOERROR Everything went fine
            * @retval ERR_UNEXPECTED Registration failed. Usually the step trigger does not match the registered one.
            */
            virtual fep::Result UnregisterStrategyTrigger(IStepTriggerStrategyListener* step_trigger_strategy_listener) = 0;

        public:
            /**
            * Start triggering.
            * The trigger implementation can use this call to do internal startup.
            *
            * @returns Standard result code.
            */
            virtual fep::Result Start() = 0;

            /**
            * Stop triggering
            * The trigger implementation can use this call to do internal shutdown.
            *
            * @returns Standard result code.
            */
            virtual fep::Result Stop() = 0;

        public:
            /**
            * Do not wait for triggers
            *
            * @returns true, if waiting for triggers ticks is used
            */
            virtual bool NeedToWaitForTrigger() const = 0;

            /**
            * Does this step trigger wait for step completition
            *
            * @returns true, if wait for completition is used
            */
            virtual bool NeedToWaitForCompletition() const = 0;

            /**
            * Does this step trigger support dummy time triggers
            *
            * @returns true, if wait support is present
            */
            virtual bool HasSupportDummyTimeTrigger() const = 0;
        };
    }
}

#endif // __FEP_TIMING_STEP_TRIGGER_STRATEGY_INTF_H
