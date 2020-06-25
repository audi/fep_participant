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

#ifndef __FEP_TIMING_STEP_TRIGGER_INTF_H
#define __FEP_TIMING_STEP_TRIGGER_INTF_H

#include <a_util/base/types.h>
#include "fep_result_decl.h"
#include "fep_participant_export.h"

namespace fep
{   
    /**
     * Listener interface used to register at \ref IStepTrigger
     */
    class FEP_PARTICIPANT_EXPORT IStepTriggerListener
    {
    public:    
        /// DTOR
        virtual ~IStepTriggerListener() = default;

    public:
        /**
         * Trigger next step 
         * 
         * @returns Current Simulation Time
         */
        virtual timestamp_t Trigger() = 0;

        /**
        * Set initial simulation time
        * Call is only valid if Trigger was not called
        *
        * @param[in] nInitalSimulationTime initial simulation time
        * @returns Standard result code.
        * @retval ERR_UNEXPECTED Already triggering. Too late to set initial time.
        */
        virtual fep::Result SetInitialSimulationTime(const timestamp_t nInitalSimulationTime) = 0;
    };

    /**
    * Step trigger interface
    * The step trigger is responsible to progress the simulation. 
    * The step trigger calls the trigger method \ref IStepTriggerListener::Trigger
    */
    class FEP_PARTICIPANT_EXPORT IStepTrigger
    {
    public:
        /// DTOR
        virtual ~IStepTrigger() = default;

    public:
        /**
        * Register a trigger listener
        * Only one step trigger listener must be supported. If more step triggers listeners are 
        * registered the method would return an error.
        *
        * @param[in] nCycleTime requested cycle time
        * @param[in] pStepTriggerListener pointer to the listener
        * @returns Standard result code.
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_UNEXPECTED Registration failed. Usually there is already a listener registered.
        */
        virtual fep::Result RegisterTrigger(const timestamp_t nCycleTime, IStepTriggerListener* pStepTriggerListener) = 0;

        /**
        * Unregister a trigger listener
        *
        * @param[in] pStepTriggerListener pointer to the listener
        * @returns Standard result code.
        * @retval ERR_NOERROR Everything went fine
        * @retval ERR_UNEXPECTED Registration failed. Usually the step trigger does not match the registered one.
        */
        virtual fep::Result UnregisterTrigger(IStepTriggerListener* pStepTriggerListener) = 0;
    };
}

#endif // __FEP_TIMING_STEP_TRIGGER_INTF_H
