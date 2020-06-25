/**
 * Implementation of the Class cStateMachine.
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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <a_util/concurrency/chrono.h>
#include <a_util/concurrency/semaphore.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>
#include <a_util/strings/strings_functions.h>
#include <a_util/system/system.h>

#include "_common/fep_timestamp.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep_errors.h"
#include "fep_statemachine_common.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "messages/fep_command_access_intf.h"
#include "messages/fep_command_control.h"
#include "messages/fep_command_control_intf.h"
#include "module/fep_module.h"
#include "statemachine/fep_state_change_notifier.h"
#include "statemachine/fep_state_helper.h"
#include "statemachine/fep_statemachine.h"

#ifdef _MSC_VER
#pragma warning(disable : 4355)
#endif

namespace fep
{
class INotificationAccess;

cStateMachine::StateChangeEvent::StateChangeEvent() : _last_state_reached(fep::FS_STARTUP)
{
}

void cStateMachine::StateChangeEvent::setStateReached(tState state_reached)
{
    if (fep::FS_UNKNOWN == state_reached)
    {
        return;
    }
    std::lock_guard<std::mutex> lock(_in_use);
    _last_state_reached = state_reached;
    _event.notify();
}

tState cStateMachine::StateChangeEvent::getLastFullyStateReached()
{
    std::lock_guard<std::mutex> lock(_in_use);
    return _last_state_reached;
}

static const int32_t s_nDefaultQueueWaitTime = 1000;

cStateMachine::cStateMachine()
    : m_bIsInitialized(false),
      m_poCommandAccess(NULL),
      m_poTiming(NULL),
      m_poIncidentHandler(NULL),
      m_poPropertyTree(NULL),
      m_poChangeNotifier(),
      m_bStandAloneModeEnabled(false),
      m_eFEPState(fep::FS_STARTUP),
      m_oStateRequestListenerRegistry(),
      m_oStateEntryListenerRegistry(),
      m_oStateExitListenerRegistry(),
      m_mtxCurrentState(),
      m_bIgnoreEvents(false),
      m_bConsiderRequestResult(true),
      m_bInitalStateChangeDone(false),
      m_oPendingEventQueue(),
      m_oEventWorkerThread()
{
}

cStateMachine::~cStateMachine()
{
    Finalize();
}

fep::Result cStateMachine::ErrorEvent()
{
    return EnqueueStateTransition(ST_Error);
}

fep::Result cStateMachine::ErrorFixedEvent()
{
    return EnqueueStateTransition(ST_ErrorFixed);
}

fep::Result cStateMachine::InitDoneEvent()
{
    return EnqueueStateTransition(ST_InitDone);
}

fep::Result cStateMachine::InitializeEvent()
{
    return EnqueueStateTransition(ST_Initialize);
}

fep::Result cStateMachine::RestartEvent()
{
    return EnqueueStateTransition(ST_Restart);
}

fep::Result cStateMachine::ShutdownEvent()
{
    return EnqueueStateTransition(ST_Shutdown);
}

fep::Result cStateMachine::StartEvent()
{
    return EnqueueStateTransition(ST_Start);
}

fep::Result cStateMachine::StartupDoneEvent()
{
    return EnqueueStateTransition(ST_StartupDone);
}

fep::Result cStateMachine::StopEvent()
{
    return EnqueueStateTransition(ST_Stop);
}

tState cStateMachine::GetState() const
{
    return m_eFEPState;
}

fep::Result
    cStateMachine::GetRemoteState(const char* strElementName, tState& eState, timestamp_t tmTimeout)
{
    fep::Result nResult;
    if (NULL == strElementName)
    {
        nResult = ERR_POINTER;
    }

    IProperty* poProp = NULL;
    if (fep::isOk(nResult))
    {
        nResult = m_poPropertyTree->GetRemoteProperty(
            strElementName, "Header.CurrentState", &poProp, tmTimeout);
        if (fep::isFailed(nResult))
        {
            eState = fep::FS_UNKNOWN;
        }
    }
    if (fep::isOk(nResult))
    {
        const char* strVal = NULL;
        if (fep::isFailed(poProp->GetValue(strVal)))
        {
            nResult = ERR_FAILED;
        }
        else
        {
            nResult = cState::FromString(strVal, eState);
            if (fep::isFailed(nResult))
            {
                nResult = ERR_FAILED;
            }
        }

        delete poProp;
    }

    return nResult;
}

fep::Result cStateMachine::TriggerRemoteEvent(const fep::tControlEvent eEvent,
                                              const char* strElementName)
{
    if (!strElementName)
    {
        return ERR_POINTER;
    }
    fep::Result nResult;
    if (NULL == strElementName)
    {
        nResult = ERR_POINTER;
    }

    if (a_util::strings::isEmpty(strElementName))
    {
        return ERR_INVALID_ARG;
    }

    if (fep::isOk(nResult))
    {
        // assemble control command
        const char* strName;
        if (fep::isOk(m_poPropertyTree->GetPropertyValue(fep::g_strElementHeaderPath_strElementName,
                                                         strName)))
        {
            cControlCommand oCmd(eEvent,
                                 strName,
                                 strElementName,
                                 GetTimeStampMicrosecondsUTC(),
                                 m_poTiming->GetTime());
            // transmit control command
            if (fep::isFailed(m_poCommandAccess->TransmitCommand(&oCmd)))
            {
                nResult = ERR_FAILED;
            }
        }
        else
        {
            nResult = ERR_FAILED;
        }
    }

    return nResult;
}

fep::Result cStateMachine::RegisterStateEntryListener(IStateEntryListener* const poListener)
{
    return m_oStateEntryListenerRegistry.RegisterListener(poListener);
}

fep::Result cStateMachine::GetCallbackFcnPointer(tState eNewState,
                                                 tState eOldState,
                                                 tStateRequestCallback& pRequestCallback,
                                                 tStateEntryCallback& pEntryCallback,
                                                 tStateExitCallback& pExitCallback)
{
    fep::Result nResult = ERR_NOERROR;
    switch (eNewState)
    {
        case FS_ERROR:
        {
            pRequestCallback = &IStateRequestListener::ProcessErrorRequest;
            pEntryCallback = &IStateEntryListener::ProcessErrorEntry;
            break;
        }
        case FS_IDLE:
        {
            pRequestCallback = &IStateRequestListener::ProcessIdleRequest;
            pEntryCallback = &IStateEntryListener::ProcessIdleEntry;
            break;
        }
        case FS_INITIALIZING:
        {
            pRequestCallback = &IStateRequestListener::ProcessInitializingRequest;
            pEntryCallback = &IStateEntryListener::ProcessInitializingEntry;
            break;
        }
        case FS_READY:
        {
            pRequestCallback = &IStateRequestListener::ProcessReadyRequest;
            pEntryCallback = &IStateEntryListener::ProcessReadyEntry;
            break;
        }
        case FS_RUNNING:
        {
            pRequestCallback = &IStateRequestListener::ProcessRunningRequest;
            pEntryCallback = &IStateEntryListener::ProcessRunningEntry;
            break;
        }
        case FS_STARTUP:
        {
            // process startup request is removed, because it isn't needed
            // see also FEPSDK-802
            pEntryCallback = &IStateEntryListener::ProcessStartupEntry;
            break;
        }
        case FS_SHUTDOWN:
        {
            pRequestCallback = &IStateRequestListener::ProcessShutdownRequest;
            pEntryCallback = &IStateEntryListener::ProcessShutdownEntry;
            break;
        }
        default:
        {
            nResult = ERR_INVALID_STATE;
            break;
        }
    } /* switch (eNewState) */
    switch (eOldState)
    {
        case FS_ERROR:
        {
            pExitCallback = &IStateExitListener::ProcessErrorExit;
            break;
        }
        case FS_IDLE:
        {
            pExitCallback = &IStateExitListener::ProcessIdleExit;
            break;
        }
        case FS_INITIALIZING:
        {
            pExitCallback = &IStateExitListener::ProcessInitializingExit;
            break;
        }
        case FS_READY:
        {
            pExitCallback = &IStateExitListener::ProcessReadyExit;
            break;
        }
        case FS_RUNNING:
        {
            pExitCallback = &IStateExitListener::ProcessRunningExit;
            break;
        }
        case FS_STARTUP:
        {
            pExitCallback = &IStateExitListener::ProcessStartupExit;
            break;
        }
        case FS_SHUTDOWN:
        {
            pExitCallback = NULL;
            /* as FS_SHUTDOWN is a valid state we do handle it here and not by
             * returning an error (in default case); but as there is no State
             * Exit Callback, we return NULL (to avoid an unintended "call") */
            break;
        }
        default:
        {
            nResult = ERR_INVALID_STATE;
            break;
        }
    } /* switch (eOldState) */

    if (fep::isFailed(nResult))
    {
        /* to ensure a "default check to NULL" will recognize the invalid
         * pointers, even if return code is not checked, we set all callbacks
         * to NULL if an error occurred */
        pRequestCallback = NULL;
        pEntryCallback = NULL;
        pExitCallback = NULL;
    }

    return nResult;
}

fep::Result cStateMachine::EnqueueStateTransition(tStateTransition eTransition)
{
    m_oIgnoreMutex.lock();
    if (!m_bIgnoreEvents)
    {
        m_oPendingEventQueue.Enqueue([this, eTransition]() {
            fep::Result result;
            auto transition_valid = PerformStateTransition(eTransition, result);
            if (transition_valid && fep::isFailed(result))
            {
                ErrorEvent();
            }
        });
    }
    m_oIgnoreMutex.unlock();
    return fep::ERR_NOERROR;
}

bool cStateMachine::PerformStateTransition(tStateTransition eTransition,
                                           fep::Result& transition_result)
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_mtxCurrentState);

    fep::tState eNewState = fep::FS_UNKNOWN;

    switch (eTransition)
    {
        case ST_StartupDone:
            if (m_eFEPState == fep::FS_STARTUP)
            {
                eNewState = fep::FS_IDLE;
            }
            break;
        case ST_Error:
            if (m_eFEPState != fep::FS_STARTUP && m_eFEPState != fep::FS_SHUTDOWN)
            {
                eNewState = fep::FS_ERROR;
            }
            break;
        case ST_Restart:
            if (m_eFEPState == fep::FS_IDLE)
            {
                eNewState = fep::FS_STARTUP;
            }
            break;
        case ST_Shutdown:
            if (m_eFEPState == fep::FS_STARTUP || m_eFEPState == fep::FS_IDLE ||
                m_eFEPState == fep::FS_ERROR)
            {
                eNewState = fep::FS_SHUTDOWN;
            }
            break;
        case ST_Initialize:
            if (m_eFEPState == fep::FS_IDLE)
            {
                eNewState = fep::FS_INITIALIZING;
            }
            break;
        case ST_Stop:
            if (m_eFEPState == fep::FS_INITIALIZING || m_eFEPState == fep::FS_READY ||
                m_eFEPState == fep::FS_RUNNING)
            {
                eNewState = fep::FS_IDLE;
            }
            break;
        case ST_InitDone:
            if (m_eFEPState == fep::FS_INITIALIZING)
            {
                eNewState = fep::FS_READY;
            }
            break;
        case ST_Start:
            if (m_eFEPState == fep::FS_READY)
            {
                eNewState = fep::FS_RUNNING;
            }
            break;
        case ST_ErrorFixed:
            if (m_eFEPState == fep::FS_ERROR)
            {
                eNewState = fep::FS_IDLE;
            }
            break;
        default:
            break;
    }

    if (eNewState != fep::FS_UNKNOWN)
    {
        transition_result = SetState(eNewState);
        // transition was valid
        return true;
    }
    // transition is not valid, the event will be ignored
    return false;
}

fep::Result cStateMachine::SetState(tState eNewState)
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_mtxCurrentState);
    fep::Result nResult;

    // safeguard if this is called during performshutdown
    m_oIgnoreMutex.lock();
    if (m_bIgnoreEvents)
    {
        nResult = ERR_CANCELLED;
    }
    m_oIgnoreMutex.unlock();

    if (fep::isOk(nResult))
    {
        fep::Result nResultRequestBackup = ERR_NOERROR;
        tStateRequestCallback pRequestCallback = NULL;
        tStateEntryCallback pEntryCallback = NULL;
        tStateExitCallback pExitCallback = NULL;

        nResult = GetCallbackFcnPointer(
            eNewState, m_eFEPState, pRequestCallback, pEntryCallback, pExitCallback);

        tState eOldState = m_eFEPState;

        /* Ugly hack, but the states itself can not invoke incidents */
        if (eOldState == fep::FS_STARTUP && eNewState == fep::FS_SHUTDOWN)
        {
            INVOKE_INCIDENT(m_poIncidentHandler,
                            FSI_GENERAL_CRITICAL_FAILURE,
                            SL_Critical_Local,
                            "Due to an error in FS_STARTUP a shutdown was requested. See other FEP "
                            "Incidents for details.");
        }

        if (fep::isOk(nResult) && m_bInitalStateChangeDone && pRequestCallback)
        { /* State Request Listeners */
            nResult = m_oStateRequestListenerRegistry.NotifyListener(pRequestCallback, eOldState);
            nResultRequestBackup = nResult;
        }

        if (fep::isOk(nResult) || !m_bConsiderRequestResult)
        { /* all other listeners: State Exit, CleanUp, State Entry */
            fep::Result nResultExit = ERR_NOERROR, nResultCleanUp = ERR_NOERROR,
                        nResultEntry = ERR_NOERROR;

            /* State Exit Listeners */
            if (m_bInitalStateChangeDone)
            {
                nResultExit = m_oStateExitListenerRegistry.NotifyListener(pExitCallback, eNewState);
                if (m_poInternalStateExitListener)
                {
                    nResultExit |= (m_poInternalStateExitListener->*pExitCallback)(eNewState);
                }
            }

            bool bCleanupCall = false;
            if (eOldState == FS_IDLE || eOldState == FS_ERROR)
            {
                bCleanupCall = eNewState == FS_STARTUP || eNewState == FS_SHUTDOWN;
            }

            // exit listener results are not considered
            // all other results are OK => change state
            m_eFEPState = eNewState;

            if (bCleanupCall)
            {
                /* CleanUp Listeners */
                nResultCleanUp = m_oStateEntryListenerRegistry.NotifyListener(
                    &IStateEntryListener::CleanUp, eOldState);
                /*Internal clean up listener */
                if (m_poInternalStateEntryListener)
                {
                    nResultCleanUp |= m_poInternalStateEntryListener->CleanUp(eOldState);
                }
            }

            /* Internal State Entry Listener */
            if (m_poInternalStateEntryListener)
            {
                (m_poInternalStateEntryListener->*pEntryCallback)(eOldState);
            }
            /* State Entry Listeners */
            nResultEntry = m_oStateEntryListenerRegistry.NotifyListener(pEntryCallback, eOldState);

            /* select return value:
             * - exit callback errors are dominant over CleanUp callback errors
             * - CleanUp callback errors are dominant over entry callback errors
             * - entry callback errors have the lowest priority*/
            if (fep::isFailed(nResultExit))
            {
                nResult = nResultExit;
            }
            else if (fep::isFailed(nResultCleanUp))
            {
                nResult = nResultCleanUp;
            }
            else
            {
                nResult = nResultEntry;
            }
        }

        // Clean up listeners deleted during the execution of this method
        // m_lstStateRequestListeners.remove(NULL);
        // m_lstStateEntryListeners.remove(NULL);
        // m_lstStateExitListeners.remove(NULL);

        if (fep::isOk(nResult) && fep::isFailed(nResultRequestBackup))
        {
            /* state change was not performed, i.e. at least one request
             * callback failed => discard all pending events */
            // log an incident
            INVOKE_INCIDENT(m_poIncidentHandler,
                            FSI_STM_STATE_RQ_FAILED,
                            SL_Info,
                            a_util::strings::format(
                                "pending state transition was "
                                "stopped as at least one state request callback returned an error "
                                "(error code: %d)",
                                nResult.getErrorCode())
                                .c_str());
        }
    }

    if (fep::isFailed(nResult))
    {
        INVOKE_INCIDENT(
            m_poIncidentHandler,
            FSI_GENERAL_CRITICAL_FAILURE,
            SL_Critical,
            a_util::strings::format(
                "An error occurred during state transition from old state: %s to new state: %s "
                "(error code: %d) "
                "(error description: %s)",
                cState::ToString(m_eFEPState),
                cState::ToString(eNewState),
                nResult.getErrorCode(),
                nResult.getDescription())
                .c_str());
    }
    // any SetState call MUST call this event set
    _state_change_event.setStateReached(m_eFEPState);

    return nResult;
}

fep::Result cStateMachine::Initialize(fep::ICommandAccess* poCommandAccess,
                                      IIncidentHandler* poIncidentHandler,
                                      ITiming* poTiming,
                                      IPropertyTree* poPropertyTree,
                                      INotificationAccess* poNotificationAccess,
                                      IStateEntryListener* poInternalStateEntryListener,
                                      IStateExitListener* poInternalStateExitListener)
{
    fep::Result nRes = ERR_NOERROR;

    if (m_bIsInitialized)
    {
        nRes = ERR_UNEXPECTED;
    }

    if (fep::isOk(nRes))
    {
        if (!poCommandAccess || !poIncidentHandler || !poTiming || !poPropertyTree ||
            !poNotificationAccess || !poInternalStateEntryListener || !poInternalStateExitListener)
        {
            nRes = ERR_INVALID_ARG;
        }
    }

    if (fep::isOk(nRes))
    {
        m_poCommandAccess = poCommandAccess;
        m_poTiming = poTiming;
        m_poIncidentHandler = poIncidentHandler;
        m_poPropertyTree = poPropertyTree;
        m_poInternalStateEntryListener = poInternalStateEntryListener;
        m_poInternalStateExitListener = poInternalStateExitListener;
    }

    if (fep::isOk(nRes))
    {
        // Init worker thread
        nRes = m_oEventWorkerThread.SetEventQueue(&m_oPendingEventQueue);

        nRes |= RegisterStandAloneModeListener();
        nRes |= m_poCommandAccess->RegisterCommandListener(this);

        m_poChangeNotifier.reset(new cStateChangeNotifier());
        nRes |=
            m_poChangeNotifier->Initialize(this, poTiming, poPropertyTree, poNotificationAccess);

        // reset to initial value
        m_eFEPState = fep::FS_STARTUP;
        _state_change_event.setStateReached(fep::FS_STARTUP);
    }

    if (fep::isOk(nRes))
    {
        m_bIsInitialized = true;
    }
    else
    {
        m_poChangeNotifier.reset();
    }

    return nRes;
}

fep::Result cStateMachine::Finalize()
{
    fep::Result nRes = ERR_NOERROR;

    if (!m_bIsInitialized)
    {
        nRes = ERR_UNEXPECTED;
        m_bIgnoreEvents = true;
    }

    if (fep::isOk(nRes))
    {
        nRes = UnregisterStandAloneModeListener();
        nRes |= m_poCommandAccess->UnregisterCommandListener(this);
        nRes |= PerformShutdown();

        // clear all listeners
        m_poChangeNotifier->Finalize();
        m_poChangeNotifier.reset();

        m_poInternalStateEntryListener = NULL;
        m_oStateEntryListenerRegistry.UnregisterAllListeners();
        m_poInternalStateExitListener = NULL;
        m_oStateExitListenerRegistry.UnregisterAllListeners();
        m_oStateRequestListenerRegistry.UnregisterAllListeners();

        // Deinit worker thread
        m_oEventWorkerThread.SetEventQueue(NULL);
    }

    if (fep::isOk(nRes))
    {
        m_poCommandAccess = NULL;
        m_poTiming = NULL;
        m_poIncidentHandler = NULL;
        m_poPropertyTree = NULL;
        m_bIgnoreEvents = true;
    }

    // Always set to deinitialized
    m_bIsInitialized = false;

    return nRes;
}

fep::Result cStateMachine::UnregisterStateEntryListener(IStateEntryListener* const poListener)
{
    return m_oStateEntryListenerRegistry.UnregisterListener(poListener);
}

fep::Result cStateMachine::RegisterStateRequestListener(IStateRequestListener* const poListener)
{
    return m_oStateRequestListenerRegistry.RegisterListener(poListener);
}

fep::Result cStateMachine::UnregisterStateRequestListener(IStateRequestListener* const poListener)
{
    return m_oStateRequestListenerRegistry.UnregisterListener(poListener);
}

fep::Result cStateMachine::Update(IControlCommand const* poCommand)
{
    fep::Result nResult = ERR_NOERROR;

    // in standalone mode we ignore any remote state change requests
    if (m_bStandAloneModeEnabled)
    {
        return nResult;
    }

    switch (poCommand->GetEvent())
    {
        case CE_Initialize:
        {
            nResult = InitializeEvent();
            break;
        }
        case CE_Start:
        {
            nResult = StartEvent();
            break;
        }
        case CE_Stop:
        {
            nResult = StopEvent();
            break;
        }
        case CE_Shutdown:
        {
            nResult = ShutdownEvent();
            break;
        }
        case CE_ErrorFixed:
        {
            nResult = ErrorFixedEvent();
            break;
        }
        case CE_Restart:
        {
            nResult = RestartEvent();
            break;
        }
        default:
        {
            // unknown control event => ignore
            // FIXME: StateMachine should throw incident (via IncidentHandler) in case an unknown
            // event is given
            break;
        }
    }

    return ERR_NOERROR;
}

fep::Result cStateMachine::ProcessPropertyAdd(fep::IProperty const* poProperty,
                                              fep::IProperty const* poAffectedProperty,
                                              char const* strRelativePath)
{
    fep::Result nResult;
    if (NULL == m_poIncidentHandler)
    {
        nResult = ERR_POINTER;
    }

    // standalone mode monitoring
    const char* strFullPath = poAffectedProperty->GetPath();
    if (fep::isOk(nResult) &&
        a_util::strings::isEqual(strFullPath,
                                 component_config::g_strStateMachineStandAloneModePath_bEnable))
    {
        nResult = poAffectedProperty->GetValue(m_bStandAloneModeEnabled);
        if (m_bStandAloneModeEnabled)
        {
            INVOKE_INCIDENT(m_poIncidentHandler,
                            FSI_STM_STAND_ALONE_MODE,
                            SL_Warning,
                            "STM: stand alone mode just got enabled - remote state events "
                            "will be ignored now");
        }
        else
        {
            INVOKE_INCIDENT(m_poIncidentHandler,
                            FSI_STM_STAND_ALONE_MODE,
                            SL_Info,
                            "STM: stand alone mode just got disabled - remote state events "
                            "will be considered now");
        }
    }
    return nResult;
}

fep::Result cStateMachine::ProcessPropertyChange(fep::IProperty const* poProperty,
                                                 fep::IProperty const* poAffectedProperty,
                                                 char const* strRelativePath)
{
    fep::Result nResult;
    if (NULL == m_poIncidentHandler)
    {
        nResult = ERR_POINTER;
    }

    // standalone mode monitoring
    const char* strFullPath = poAffectedProperty->GetPath();
    if (fep::isOk(nResult) &&
        a_util::strings::isEqual(strFullPath,
                                 component_config::g_strStateMachineStandAloneModePath_bEnable))
    {
        nResult = poAffectedProperty->GetValue(m_bStandAloneModeEnabled);
        if (m_bStandAloneModeEnabled)
        {
            INVOKE_INCIDENT(m_poIncidentHandler,
                            FSI_STM_STAND_ALONE_MODE,
                            SL_Info,
                            "STM: stand alone mode just got enabled - remote state events "
                            "will be ignored now");
        }
        else
        {
            INVOKE_INCIDENT(m_poIncidentHandler,
                            FSI_STM_STAND_ALONE_MODE,
                            SL_Info,
                            "STM: stand alone mode just got disabled - remote state events "
                            "will be considered now");
        }
    }
    return nResult;
}

fep::Result cStateMachine::ProcessPropertyDelete(fep::IProperty const* poProperty,
                                                 fep::IProperty const* poAffectedProperty,
                                                 char const* strRelativePath)
{
    return ERR_NOERROR;
}

fep::Result cStateMachine::FireupStateMachine()
{
    m_bInitalStateChangeDone = false;
    m_bIgnoreEvents = false;    // needed for renewed startup (see cTesterStatemachine.TestChangeNotifier)

    fep::Result nRes = SetState(FS_STARTUP);
    m_bInitalStateChangeDone = true;
    return nRes;
}

void cStateMachine::ResetStandAloneProperty()
{
    m_poPropertyTree->SetPropertyValue(
        component_config::g_strStateMachineStandAloneModePath_bEnable, false);
}

fep::Result cStateMachine::RegisterStandAloneModeListener()
{
    fep::Result nResult;
    if (NULL == m_poPropertyTree)
    {
        nResult = ERR_POINTER;
    }

    if (fep::isOk(nResult))
    {
        nResult = m_poPropertyTree->SetPropertyValue(
            component_config::g_strStateMachineStandAloneModePath_bEnable, false);
    }
    if (fep::isOk(nResult))
    {
        nResult = m_poPropertyTree->RegisterListener(
            component_config::g_strStateMachineStandAloneModePath_bEnable, this);
    }

    return nResult;
}

fep::Result cStateMachine::UnregisterStandAloneModeListener()
{
    fep::Result nResult;
    if (NULL == m_poPropertyTree)
    {
        nResult = ERR_POINTER;
    }

    if (fep::isOk(nResult))
    {
        nResult = m_poPropertyTree->UnregisterListener(
            component_config::g_strStateMachineStandAloneModePath_bEnable, this);
    }
    return nResult;
}

fep::Result cStateMachine::PerformShutdown()
{
    fep::Result nResult = ERR_NOERROR;

    // disable events
    m_oIgnoreMutex.lock();
    m_bIgnoreEvents = true;
    m_oIgnoreMutex.unlock();

    {
        // stop worker
        m_oEventWorkerThread.SetEventQueue(NULL);
        // clear event queue
        tStateEventHandler oEvent = NULL;
        while (m_oPendingEventQueue.TryDequeue(oEvent, s_nDefaultQueueWaitTime))
        {
            // just clearing
        }
        // start worker
        m_oEventWorkerThread.SetEventQueue(&m_oPendingEventQueue);
    }

    // disable request callback result consideration
    m_bConsiderRequestResult = false;

    // reenable events
    m_oIgnoreMutex.lock();
    m_bIgnoreEvents = false;
    m_oIgnoreMutex.unlock();

    // trigger state machine into shutdown state
    switch (GetState())
    {
        case FS_STARTUP:
        { // -> Shutdown
            ShutdownEvent();
        }
        break;
        case FS_IDLE:
        { // -> Shutdown
            ShutdownEvent();
        }
        break;
        case FS_INITIALIZING:
        { // -> Idle -> Shutdown
            StopEvent();
            ShutdownEvent();
        }
        break;
        case FS_READY:
        { // -> Idle -> Shutdown
            StopEvent();
            ShutdownEvent();
        }
        break;
        case FS_RUNNING:
        { // -> Idle -> Shutdown
            StopEvent();
            ShutdownEvent();
        }
        break;
        case FS_ERROR:
        { // -> Shutdown
            ShutdownEvent();
        }
        break;
        case FS_SHUTDOWN:
            // nothing to do here
            break;
        default:
            assert(false);
            break;
    }

    while (isFailed(WaitForState(FS_SHUTDOWN, 500, true)))
    {
        ShutdownEvent();
    }

    // reenable request callback result consideration (just to be clean)
    m_bConsiderRequestResult = true;

    // all done
    return nResult;
}

fep::Result cStateMachine::RegisterStateExitListener(IStateExitListener* const poListener)
{
    return m_oStateExitListenerRegistry.RegisterListener(poListener);
}

fep::Result cStateMachine::UnregisterStateExitListener(IStateExitListener* const poListener)
{
    return m_oStateExitListenerRegistry.UnregisterListener(poListener);
}

fep::Result cStateMachine::WaitForState(const tState eState,
                                        const timestamp_t tmTimeout,
                                        bool error_is_failure) const
{
    if (tmTimeout < -1 || FS_UNKNOWN == eState)
    {
        return ERR_INVALID_ARG;
    }

    auto current_state = _state_change_event.getLastFullyStateReached();
    if (current_state == eState)
    {
        return fep::Result();
    }

    if (tmTimeout == -1)
    {
        while (true) // wait forever or until error or until state is reached
        {
            _state_change_event._event.wait_for(a_util::chrono::milliseconds(300));
            current_state = _state_change_event.getLastFullyStateReached();
            if (current_state == eState)
            {
                return fep::Result();
            }
            else if (error_is_failure && current_state == FS_ERROR)
            {
                return ERR_FAILED;
            }
        }
    }
    else
    {
        timestamp_t wait_time = tmTimeout;

        while (wait_time > 0) // wait for timeout or until error or until state is reached
        {
            auto begin_time_waiting = a_util::system::getCurrentMilliseconds();
            _state_change_event._event.wait_for(a_util::chrono::milliseconds(wait_time));
            auto end_time_waiting = a_util::system::getCurrentMilliseconds();

            current_state = _state_change_event.getLastFullyStateReached();
            if (current_state == eState)
            {
                return fep::Result();
            }
            else if (error_is_failure && current_state == FS_ERROR)
            {
                return ERR_FAILED;
            }
            else
            {
                wait_time -= end_time_waiting - begin_time_waiting;
            }
        }
    }

    return ERR_TIMEOUT;
}

}  // namespace fep
