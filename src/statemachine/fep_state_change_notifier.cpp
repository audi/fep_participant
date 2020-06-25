/**
 * Implementation of the Class cStateChangeNotifier.
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

#include <cstddef>
#include <a_util/result/result_type.h>

#include "_common/fep_timestamp.h"
#include "fep3/components/legacy/property_tree/propertytreebase_intf.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep_errors.h"
#include "messages/fep_notification_access_intf.h"
#include "messages/fep_notification_state.h"
#include "module/fep_module.h"
#include "statemachine/fep_statemachine_intf.h"
#include "statemachine/fep_state_change_notifier.h"

using namespace fep;

cStateChangeNotifier::cStateChangeNotifier() :
    m_bIsInitialized(false), m_poStateMachine(NULL), m_poTiming(NULL), 
    m_poPropertyTree(NULL), m_poNotificationAccess(NULL),
    m_eCurrentState(FS_STARTUP)
{
}

cStateChangeNotifier::~cStateChangeNotifier()
{
    Finalize();
}

fep::Result cStateChangeNotifier::CleanUp(const tState)
{
    /* no state message to sent */
    return ERR_NOERROR;
}

fep::Result cStateChangeNotifier::ProcessErrorEntry(const tState)
{
    m_eCurrentState = FS_ERROR;
    SendStateChangeNotification(FS_ERROR);
    return ERR_NOERROR;
}

fep::Result cStateChangeNotifier::ProcessIdleEntry(const tState)
{
    m_eCurrentState = FS_IDLE;
    SendStateChangeNotification(FS_IDLE);
    return ERR_NOERROR;
}

fep::Result cStateChangeNotifier::ProcessInitializingEntry(const tState)
{
    m_eCurrentState = FS_INITIALIZING;
    SendStateChangeNotification(FS_INITIALIZING);
    return ERR_NOERROR;
}

fep::Result cStateChangeNotifier::ProcessReadyEntry(const tState)
{
    m_eCurrentState = FS_READY;
    SendStateChangeNotification(FS_READY);
    return ERR_NOERROR;
}

fep::Result cStateChangeNotifier::ProcessRunningEntry(const tState)
{
    m_eCurrentState = FS_RUNNING;
    SendStateChangeNotification(FS_RUNNING);
    return ERR_NOERROR;
}

fep::Result cStateChangeNotifier::ProcessStartupEntry(const tState)
{
    m_eCurrentState = FS_STARTUP;
    SendStateChangeNotification(FS_STARTUP);
    return ERR_NOERROR;
}

fep::Result cStateChangeNotifier::ProcessShutdownEntry(const tState)
{
    m_eCurrentState = FS_SHUTDOWN;
    SendStateChangeNotification(FS_SHUTDOWN);
    return ERR_NOERROR;
}

fep::Result cStateChangeNotifier::SendStateChangeNotification(const tState eState)
{
    fep::Result nResult = ERR_POINTER;
    if (NULL != m_poTiming && NULL != m_poNotificationAccess && NULL != m_poPropertyTree)
    {
        const char* strName;
        if (fep::isOk(
            m_poPropertyTree->GetPropertyValue(fep::g_strElementHeaderPath_strElementName, strName)))
        {
            cStateNotification oNoti(eState, strName, "*", GetTimeStampMicrosecondsUTC(),
                m_poTiming->GetTime());
            nResult = m_poNotificationAccess->TransmitNotification(&oNoti);
        }
        else
        {
            nResult = ERR_FAILED;
        }
    }

    return nResult;
}

fep::Result cStateChangeNotifier::Initialize(IStateMachine* poStateMachine, ITiming* poTiming, 
    IPropertyTreeBase* poPropertyTree, INotificationAccess* poNotificationAccess)
{
    fep::Result nRes = ERR_NOERROR;

    if (m_bIsInitialized)
    {
        nRes = ERR_UNEXPECTED;
    }

    if (fep::isOk(nRes))
    {
        if (!poStateMachine || !poTiming || !poPropertyTree || !poNotificationAccess)
        {
            nRes = ERR_INVALID_ARG;
        }
    }

    if (fep::isOk(nRes))
    {
        m_poStateMachine = poStateMachine;
        m_poTiming = poTiming;
        m_poPropertyTree = poPropertyTree;
        m_poNotificationAccess = poNotificationAccess;
    }

    if (fep::isOk(nRes))
    {
        m_poStateMachine->RegisterStateEntryListener(this);
    }

    if (fep::isOk(nRes))
    {
        m_bIsInitialized = true;
    }

    return nRes;
}

fep::Result cStateChangeNotifier::Finalize()
{
    fep::Result nRes = ERR_NOERROR;

    if (!m_bIsInitialized)
    {
        nRes = ERR_UNEXPECTED;
    }

    if (fep::isOk(nRes))
    {
        m_poStateMachine->UnregisterStateEntryListener(this);
    }
    
    if (fep::isOk(nRes))
    {
        m_poStateMachine = NULL;
        m_poTiming = NULL;
        m_poPropertyTree = NULL;
        m_poNotificationAccess = NULL;
    }

    // Always set to deinitialized
    m_bIsInitialized = false;

    return nRes;
}
