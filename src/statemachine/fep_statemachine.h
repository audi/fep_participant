/**
 * Declaration of the Class cStateMachine.
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

#if !defined(EA_8D902920_4D26_4d45_BDD3_4134E9B64F18__INCLUDED_)
#define EA_8D902920_4D26_4d45_BDD3_4134E9B64F18__INCLUDED_

#include <functional>
#include <memory>
#include <mutex>
#include <a_util/base/types.h>
#include <a_util/concurrency/semaphore.h>

#include "_common/fep_observer_pattern.h"
#include "_common/fep_waitable_queue.h"
#include "fep3/components/legacy/property_tree/property_listener_intf.h"
#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "messages/fep_command_listener.h"
#include "messages/fep_control_event.h"
#include "fep3/base/states/fep2_state.h"
#include "statemachine/fep_state_entry_listener_intf.h"
#include "statemachine/fep_state_event_worker.h"
#include "statemachine/fep_state_exit_listener_intf.h"
#include "statemachine/fep_state_request_listener_intf.h"
#include "statemachine/fep_statemachine_intf.h"

namespace fep
{
// stm forward declarations from public includes.
class cStateChangeNotifier;
class ICommandAccess;
class IControlCommand;
class IIncidentHandler;
class INotificationAccess;
class IProperty;
class IPropertyTree;
class ITiming;

/**
 * This class implements the state machine of FEP.
 */
class FEP_PARTICIPANT_EXPORT cStateMachine : public fep::cCommandListener,
                                      public fep::IPropertyListener,
                                      public fep::IStateMachine
{
public:
    enum tStateTransition
    {
        ST_StartupDone,
        ST_Error,
        ST_Restart,
        ST_Shutdown,
        ST_Initialize,
        ST_Stop,
        ST_InitDone,
        ST_Start,
        ST_ErrorFixed
    };

public:
    /**
     * CTOR
     */
    explicit cStateMachine();

    /**
     * DTOR
     */
    virtual ~cStateMachine();

public:
    fep::Result SetState(tState eNewState);

public: // override cCommandListener
    fep::Result Update(IControlCommand const* poCommand);

public: // implements IPropertyListener interface
    fep::Result ProcessPropertyAdd(fep::IProperty const* poProperty,
                                   fep::IProperty const* poAffectedProperty,
                                   char const* strRelativePath);
    fep::Result ProcessPropertyChange(fep::IProperty const* poProperty,
                                      fep::IProperty const* poAffectedProperty,
                                      char const* strRelativePath);
    fep::Result ProcessPropertyDelete(fep::IProperty const* poProperty,
                                      fep::IProperty const* poAffectedProperty,
                                      char const* strRelativePath);

public: // implements IStateMachine
    fep::Result StartupDoneEvent();
    fep::Result InitializeEvent();
    fep::Result InitDoneEvent();
    fep::Result StartEvent();
    fep::Result StopEvent();
    fep::Result ErrorEvent();
    fep::Result ErrorFixedEvent();
    fep::Result RestartEvent();
    fep::Result ShutdownEvent();
    fep::Result RegisterStateRequestListener(IStateRequestListener* const poListener);
    fep::Result UnregisterStateRequestListener(IStateRequestListener* const poListener);
    fep::Result RegisterStateEntryListener(IStateEntryListener* const poListener);
    fep::Result UnregisterStateEntryListener(IStateEntryListener* const poListener);
    fep::Result RegisterStateExitListener(IStateExitListener* const poListener);
    fep::Result UnregisterStateExitListener(IStateExitListener* const poListener);
    fep::tState GetState() const;
    fep::Result
        GetRemoteState(const char* strElementName, fep::tState& eState, timestamp_t tmTimeout);
    fep::Result TriggerRemoteEvent(const fep::tControlEvent eEvent, const char* strElementName);

public:
    /**
     * The method \ref Initialize sets up internal data and puts
     * the component into an valid/working state.
     *
     * @param [in] poCommandAccess pointer to the command access component
     * @param [in] poIncidentHandler pointer to the incident handler component
     * @param [in] poTiming pointer to the timing component
     * @param [in] poPropertyTree pointer to the property tree component
     * @param [in] poNotificationAccess pointer to the to notification access component
     * @param [in] poInternalStateEntryListener Pointer to StateEntryListener that is called
     *                                          before the registered listeners. This is used
     *                                          for the internal components.
     * @param [in] poInternalStateExitListener  Pointer to StateExitListener that is called
     *                                          after the registered listeners. This is used
     *                                          for the internal components.
     * @returns  Standard result code.
     * @retval ERR_NOERROR Everything went fine
     * @retval ERR_INVALID_ARG One of the arguments was invalid (e.g. a NULL pointer)
     * @retval ERR_UNEXPECTED Something unexpected happend
     */
    fep::Result Initialize(fep::ICommandAccess* poCommandAccess,
                           IIncidentHandler* poIncidentHandler,
                           ITiming* poTiming,
                           IPropertyTree* poPropertyTree,
                           INotificationAccess* poNotificationAccess,
                           IStateEntryListener* poInternalStateEntryListener,
                           IStateExitListener* poInternalStateExitListener);

    /**
     * The method \ref Finalize clears internal data and puts
     * the component into an invalid state.
     *
     * @returns  Standard result code.
     * @retval ERR_NOERROR Everything went fine
     * @retval ERR_UNEXPECTED Something unexpected happend (module was not initialized before)
     */
    fep::Result Finalize();

    /**
     * The method \c FireupStateMachine will make the state machine go directly to the
     * first state STARTUP. Call this only once and after you created the state machine.
     *
     * @returns  Standard result code.
     * @retval ERR_NOERROR  Everything went fine
     */
    fep::Result FireupStateMachine();

    /**
     * The Shutdown method performs the statemachine shutdown sequence
     * ending up in the Shutdown state after this call.
     *
     * @retval ERR_NOERROR Everything went fine.
     * @retval ERR_<any> Return code from user-defined StateEntry/StateExit callback.
     *
     */
    fep::Result PerformShutdown();

    fep::Result WaitForState(const fep::tState eState,
                             const timestamp_t tmTimeOut_ms,
                             bool error_is_failure) const override;

public: /// Resets stand alone mode property to false (to be used in clean up)
    void ResetStandAloneProperty();

private: // types
    /// Type for State Request Callbacks (Process<State>Request)
    typedef fep::Result (IStateRequestListener::*tStateRequestCallback)(const tState);
    /// Type for State Entry Callbacks (Process<State>Entry)
    typedef fep::Result (IStateEntryListener::*tStateEntryCallback)(const tState);
    /// Type for State Exit Callbacks (Process<State>Exit)
    typedef fep::Result (IStateExitListener::*tStateExitCallback)(const tState);

    /// Typedef for a function pointer to an event method of the state machine.
    typedef fep::Result (cStateMachine::*tStateMachineEvent)();
    /// Typedef for a function pointer to an event method of a state
    typedef std::function<void()> tStateEventHandler;
    /// Typedef for the event queue type
    typedef fep::cWaitableQueue<tStateEventHandler> tStateEventQueue;

private:
    /**
     * @brief       The method initializes the standalone flag property and registers
                    the state machine as listener on the property path
     *              ComponentConfig.StateMachine.bStandAloneModeEnabled.
     *
     * @returns     Standard result code.
     * @retval      ERR_NOERROR if everything went fine.
     *
     * \note Must be called _after_ \ref cModule::Create!
     */
    fep::Result RegisterStandAloneModeListener();

    /**
     * @brief       The method unregisters the state machine as listener on
     *              the property path ComponentConfig.StateMachine.bStandAloneModeEnabled.
     *
     * @returns     Standard result code.
     * @retval      ERR_NOERROR if everything went fine.
     * @retval      ERR_ERR_NOTFOUND if there is the STM was not registered previously.
     *
     * \note Must be called _after_ \ref cModule::Create!
     */
    fep::Result UnregisterStandAloneModeListener();

    /**
     * This method returns function pointers to the State Callbacks for the given
     * transition (defined by previous and new state). If for any reason there is
     * no callback for the current state (e.g. no State Exit Callback for "final"
     * state FS_SHUTDOWN) the corresponding out argument is set to NULL - so check
     * for NULL before you call it!
     *
     * @param[in] eNewState         The new state of current state change.
     * @param[in] eOldState         The old state of current state change.
     * @param[out] pRequestCallback Pointer to State Request Callback
     * @param[out] pEntryCallback   Pointer to State Entry Callback
     * @param[out] pExitCallback    Pointer to State Exit Callback
     * @retval ERR_NOERROR          Everything went fine
     * @retval ERR_INVALID_STATE    One or both of the given states are not supported
     *                              by the state machine - do NOT try to call any of
     *                              the returned callback pointers!
     */
    static fep::Result GetCallbackFcnPointer(tState eNewState,
                                             tState eOldState,
                                             tStateRequestCallback& pRequestCallback,
                                             tStateEntryCallback& pEntryCallback,
                                             tStateExitCallback& pExitCallback);

    /**
     * \c EnqueueStateTransition enqueues the passed event into the event queue.
     * Ignores the event if event queuing is disabled at the moment.
     *
     * @param [in] eTransition The transition
     * @return Returns standard result
     */
    fep::Result EnqueueStateTransition(tStateTransition eTransition);

    /**
     * \c RequestStateTransition performs the actual state transition
     *
     * @param [in] eTransition The transition
     * @return Returns standard result
     */
    bool PerformStateTransition(tStateTransition eTransition, fep::Result& transition_result);

private: // members
    /// module initialize state value
    bool m_bIsInitialized;
    /// The command access interface
    fep::ICommandAccess* m_poCommandAccess;
    /// the instance of the timing component of the module
    ITiming* m_poTiming;
    /// the incident handler used
    IIncidentHandler* m_poIncidentHandler;
    /// the property tree adapter
    IPropertyTree* m_poPropertyTree;
    /// The instance of the change notifier helper class
    a_util::memory::unique_ptr<cStateChangeNotifier> m_poChangeNotifier;
    /// Holds the flag whether standalone mode is enabled or not
    bool m_bStandAloneModeEnabled;
    /// The current state
    tState m_eFEPState;
    /// The registered State Request Listeners
    fep::ext::cListenerRegistry<IStateRequestListener> m_oStateRequestListenerRegistry;
    /// The registered State Entry Listeners
    fep::ext::cListenerRegistry<IStateEntryListener> m_oStateEntryListenerRegistry;
    /// The registered State Exit Listeners
    fep::ext::cListenerRegistry<IStateExitListener> m_oStateExitListenerRegistry;
    /// Critical section for SetState (important during shutdown)
    a_util::concurrency::recursive_mutex m_mtxCurrentState;
    /// Disables event queuing
    volatile bool m_bIgnoreEvents;
    /// Mutex guarding the IgnoreEvents flag
    a_util::concurrency::mutex m_oIgnoreMutex;
    /// Flag whether request listener results are considered in SetState
    bool m_bConsiderRequestResult;
    /// true if the first state change (FireupStateMachine) is done
    bool m_bInitalStateChangeDone;
    /// Event queue
    tStateEventQueue m_oPendingEventQueue;
    /// Event worker thread
    cStateEventWorker<tStateEventHandler> m_oEventWorkerThread;
    /// Internal StateListener to inform internal compotons of state entry
    IStateEntryListener* m_poInternalStateEntryListener;
    /// Internal StateListener to inform internal compotons of state exit
    IStateExitListener* m_poInternalStateExitListener;

    struct StateChangeEvent
    {
        std::mutex _in_use;
        a_util::concurrency::semaphore _event;
        tState _last_state_reached;
        explicit StateChangeEvent();
        void setStateReached(tState state_reached);
        tState getLastFullyStateReached();
    };
    mutable StateChangeEvent _state_change_event;
};
} // namespace fep
#endif // !defined(EA_8D902920_4D26_4d45_BDD3_4134E9B64F18__INCLUDED_)
