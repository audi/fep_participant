/**
 * Declaration of the Class IStateMachine.
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

#if !defined(EA_DA75C34C_2035_418c_9586_B2B338DB3F7A__INCLUDED_)
#define EA_DA75C34C_2035_418c_9586_B2B338DB3F7A__INCLUDED_

#include "fep_types.h"
#include "fep3/base/states/fep2_state.h"

// needed explicitly, include order not reversible
#include <messages/fep_control_event.h>

namespace fep
{
    class IStateEntryListener;
    class IStateExitListener;
    class IStateRequestListener;
    class IStateCyclicListener;

    /**
     * The IStateMachine interface is the main gateway for the user to access
     * the state machine. See \ref fep_state_machine for more information about the state
     * machine.
     */
    class FEP_PARTICIPANT_EXPORT IStateMachine
    {

    public:
        /// DTOR
        virtual ~IStateMachine() {}

        /**
         * @brief triggers the \c StartupDoneEvent
         *
         * This method enqueues an event to the state machine - see \ref fep_state_machine 
         * and its subsections for more information about the state machine, available 
         * states and events.
         * 
         * @retval ERR_NOERROR   Everything went fine, i.e. the state change was enqueued
         *                       into the event queue.
         */
        virtual fep::Result StartupDoneEvent() =0;
        
        /**
         * @brief triggers the \c InitializeEvent
         * @copydetails fep::IStateMachine::StartupDoneEvent
         */
        virtual fep::Result InitializeEvent() =0;

        /**
         * @brief triggers the \c InitDoneEvent
         * @copydetails fep::IStateMachine::StartupDoneEvent
         */
        virtual fep::Result InitDoneEvent() =0;
        
        /**
         * @brief triggers the \c StartEvent
         * @copydetails fep::IStateMachine::StartupDoneEvent
         */
        virtual fep::Result StartEvent() =0;
        
        /**
         * @brief triggers the \c StopEvent
         * @copydetails fep::IStateMachine::StartupDoneEvent
         */
        virtual fep::Result StopEvent() =0;

        /**
         * @brief triggers the \c ErrorEvent
         * @copydetails fep::IStateMachine::StartupDoneEvent
         */
        virtual fep::Result ErrorEvent() =0;

        /**
         * @brief triggers the \c ErrorFixedEvent
         * @copydetails fep::IStateMachine::StartupDoneEvent
         */
        virtual fep::Result ErrorFixedEvent() =0;

        /**
         * @brief triggers the \c RestartEvent
         * @copydetails fep::IStateMachine::StartupDoneEvent
         */
        virtual fep::Result RestartEvent() =0;

        /**
         * @brief triggers the \c ShutdownEvent
         * @copydetails fep::IStateMachine::StartupDoneEvent
         */
        virtual fep::Result ShutdownEvent() =0;

        /**
         * The method \c RegisterStateRequestListener will register a state request listener at
         * the state machine that will be called before every state transition.
         * 
         * @param [in] poListener  [...]
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_POINTER  Passed listener is a NULL pointer.
         */
        virtual fep::Result RegisterStateRequestListener(IStateRequestListener* const poListener) =0;
        
        /**
         * The method \c UnregisterStateRequestListener will unregister a previously
         * registered state request listener.
         * 
         * @param [in] poListener  [...]
         * @returns  Standard result code.
         * @retval ERR_NOERROR   Everything went fine
         * @retval ERR_POINTER   Passed listener is a NULL pointer.
         * @retval ERR_NOT_FOUND The given IStateRequestListener is not registered as listener
         */
        virtual fep::Result UnregisterStateRequestListener(IStateRequestListener* const poListener) =0;

        /**
         * The method \c RegisterStateEntryListener will register a state entry listener at
         * the state machine that will be called when a state was entered.
         * 
         * @param [in] poListener  [...]
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_POINTER  Passed listener is a NULL pointer.
         */
        virtual fep::Result RegisterStateEntryListener(IStateEntryListener * const poListener) =0;

        /**
         * The method \c UnregisterStateEntryListener will unregister a previously
         * registered state entry listener.
         * 
         * @param [in] poListener  The listener.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_POINTER  Passed listener is a NULL pointer.
         * @retval ERR_NOT_FOUND The given IStateRequestListener is not registered as listener
         */
        virtual fep::Result UnregisterStateEntryListener(IStateEntryListener * const poListener) =0;

        /**
         * The method \c RegisterStateExitListener will register a state exit listener at
         * the state machine that will be called when a state is left.
         * 
         * @param [in] poListener  [...]
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_POINTER  Passed listener is a NULL pointer.
         */
        virtual fep::Result RegisterStateExitListener(IStateExitListener * const poListener) =0;

        /**
         * The method \c UnregisterStateExitListener will unregister a previously
         * registered state exit listener.
         * 
         * @param [in] poListener  The listener.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_POINTER  Passed listener is a NULL pointer.
         * @retval ERR_NOT_FOUND The given IStateRequestListener is not registered as listener
         */
        virtual fep::Result UnregisterStateExitListener(IStateExitListener * const poListener) =0;

        /**
         * The method \c GetState returns the current state the state machine is in.
         * 
         * @returns  The current state.
         */
        virtual fep::tState GetState() const =0;

        /**
         * The method \c GetRemoteState queries another element for its current state.
         *
         * \note The element name may not contain any wildcards!
         *
         * @param [in] strElementName The name of the element to query
         * @param [out] eState Destination variable for the remote state
         * @param [in] tmTimeout The maximum duration for the query
         *
         * @retval ERR_NOERROR Everything went fine, the remote state is stored in eState
         * @retval ERR_TIMEOUT The request timed out
         * @retval ERR_POINTER The element name string is NULL
         * @retval ERR_INVALID_ARG The timeout is negative or the element name contains wildcards
         * @retval ERR_FAILED Communication failure
         */
        virtual fep::Result GetRemoteState(const char * strElementName,
            fep::tState & eState, timestamp_t tmTimeout) =0;

        /**
         * The method \c TriggerRemoteEvent triggers a state machine event
         * in a remote module/element, provided the receiving element has not engaged
         * standalone mode.
         *
         * @param [in] eEvent The event type to trigger
         * @param [in] strElementName The name of the element in which to
         *      trigger the event. Wildcards are supported.
         * 
         * @retval ERR_NOERROR Everything went fine, the event was sent.
         * @retval ERR_POINTER strElementName is NULL
         * @retval ERR_FAILED Communication failure
         */
        virtual fep::Result TriggerRemoteEvent(const fep::tControlEvent eEvent,
            const char * strElementName) =0;
        /**
         * @brief Wait for a (fully) state change ... exit, request and entry listener 
         * 
         * @param eState state to wait for
         * @param tmTimeOut_ms -1 wait forever (or error is failure detected)
         * @param error_is_failure if the error state was detected it will break and return an error
         * @return fep::Result 
         */
        virtual fep::Result WaitForState(const fep::tState eState,
                                         const timestamp_t tmTimeOut_ms,
                                         bool error_is_failure) const = 0;
    };
}
#endif // !defined(EA_DA75C34C_2035_418c_9586_B2B338DB3F7A__INCLUDED_)
