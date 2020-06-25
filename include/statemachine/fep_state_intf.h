/**
 * Declaration of the Class IState.
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

#if !defined(EA_C5DBDAFC_7F11_463c_848F_DACC7DE0BBC5__INCLUDED_)
#define EA_C5DBDAFC_7F11_463c_848F_DACC7DE0BBC5__INCLUDED_

#include "fep3/base/states/fep2_state.h"

namespace fep
{
    /**
     * This is the interface that gives access to the state machine.
     */
    class IStateMachineAccess;

    /**
     * This is the base interface of all classes representing a state.
     * It is part of the state pattern.
     */
    class FEP_PARTICIPANT_EXPORT IState
    {

    public:
        /**
         * DTOR
         */
        virtual ~IState(){}

        /**
         * The method \c Error will be called when a corresponding event has been triggered.
         * 
         * @param [in] poStateMachine  The state machine that is supposed to transit.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Error(IStateMachineAccess * const poStateMachine) =0;
        
        /**
         * The method \c ErrorFixed will be called when a corresponding event has been
         * triggered.
         * 
         * @param [in] poStateMachine  The state machine that is supposed to transit.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result ErrorFixed(IStateMachineAccess * const poStateMachine) =0;
        
        /**
         * The method \c Initialize will be called when a corresponding event has been
         * triggered.
         * 
         * @param [in] poStateMachine  The state machine that is supposed to transit.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Initialize(IStateMachineAccess * const poStateMachine) =0;
        
        /**
         * The method \c InitDone will be called when a corresponding event has been
         * triggered.
         * 
         * @param [in] poStateMachine  The state machine that is supposed to transit.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result InitDone(IStateMachineAccess * const poStateMachine) =0;

        /**
         * The method \c Restart will be called when a corresponding event has been
         * triggered.
         * 
         * @param [in] poStateMachine  The state machine that is supposed to transit.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Restart(IStateMachineAccess * const poStateMachine) =0;

        /**
         * The method \c Shutdown will be called when a corresponding event has been
         * triggered.
         * 
         * @param [in] poStateMachine  The state machine that is supposed to transit.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Shutdown(IStateMachineAccess * const poStateMachine) =0;
        
        /**
         * The method \c Start will be called when a corresponding event has been triggered.
         * 
         * @param [in] poStateMachine  The state machine that is supposed to transit.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Start(IStateMachineAccess * const poStateMachine) =0;
        
        /**
         * The method \c StartupDone will be called when a corresponding event has been
         * triggered.
         * 
         * @param [in] poStateMachine  The state machine that is supposed to transit.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result StartupDone(IStateMachineAccess * const poStateMachine) =0;
        
        /**
         * The method \c Stop will be called when a corresponding event has been triggered.
         * 
         * @param [in] poStateMachine  The state machine that is supposed to transit.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result Stop(IStateMachineAccess * const poStateMachine) =0;
        
        /**
         * The method \c GetStateValue returns the \c tState value this state represents.
         * 
         * @returns  The state value.
         */
        virtual tState GetStateValue() const =0;
    };
}
#endif // !defined(EA_C5DBDAFC_7F11_463c_848F_DACC7DE0BBC5__INCLUDED_)
