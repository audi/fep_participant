/**
 * Implementation of the Class cAIStateAggregator.
 *

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

#include <utility>
#include <a_util/concurrency/chrono.h>
#include <a_util/concurrency/semaphore.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_functions.h>

#include "_common/fep_timestamp.h"
#include "automation_interface/fep_ai_state_aggregator.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep_errors.h"
#include "messages/fep_command_access_intf.h"
#include "messages/fep_command_get_property.h"
#include "messages/fep_notification_access_intf.h"
#include "messages/fep_notification_property_intf.h"
#include "messages/fep_notification_state_intf.h"
#include "module/fep_module.h"
#include "module/fep_module_intf.h"
#include "statemachine/fep_state_helper.h"

using namespace fep;

void cSystemStateCollector::OnStateReceived(tCollectionMap::iterator it, const tState eState)
{
    // i.e no state has been received for this module before
    if (it->second == FS_UNKNOWN)
    {
        m_szResponsesCollected++;
    }

    // save current state for aggregation calculation
    it->second = eState;

    // only consider setting the event when all modules reported their state at least once
    if (m_szResponsesCollected == m_mapModuleStates.size())
    {
        // set event if GetSystemState (!m_bWaitForSystemState) or
        // target state of WaitForSystemState reached successfully
        if (!m_bWaitForSystemState || GetAggregatedState() == m_eTargetSystemState)
        {
            m_poModule->GetNotificationAccess()->UnregisterNotificationListener(this);
            m_oEventFinished.notify();
        }
    }
}

cSystemStateCollector::cSystemStateCollector(IModule * poModule,
    bool bWaitForTargetState, tState eTargetState) :
    m_poModule(poModule), m_mapModuleStates(), m_szResponsesCollected(0),
    m_bWaitForSystemState(bWaitForTargetState), m_eTargetSystemState(eTargetState),
    m_oResponseMutex(), m_oEventFinished()
{
}

cSystemStateCollector::~cSystemStateCollector()
{
    m_poModule->GetNotificationAccess()->UnregisterNotificationListener(this);

    // wait until all running callbacks are finished in case the wait timed out
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oResponseMutex);
}

fep::Result cSystemStateCollector::Init(const std::vector<std::string>& vecElementList)
{
    // reset map
    m_mapModuleStates.clear();

    // empty module list not allowed
    if (vecElementList.size() == 0)
    {
        return ERR_INVALID_ARG;
    }

    fep::Result nResult = ERR_NOERROR;
    for (size_t nIdx = 0; nIdx < vecElementList.size() && fep::isOk(nResult); ++nIdx)
    {
        std::string strElement = vecElementList[nIdx];
        a_util::strings::trim(strElement);
        if (!strElement.empty() &&
            strElement.find('?') == std::string::npos &&
            strElement.find('*') == std::string::npos)
        {
            m_mapModuleStates.insert(std::make_pair(strElement,
                FS_UNKNOWN));
        }
        else
        {
            nResult = ERR_INVALID_ARG;
        }
    }

    if (fep::isFailed(nResult))
    {
        m_mapModuleStates.clear();
    }

    return nResult;
}

fep::Result cSystemStateCollector::SendStateRequests()
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oResponseMutex);
    fep::Result nResult = ERR_NOERROR;

    nResult = m_poModule->GetNotificationAccess()->RegisterNotificationListener(this);
    m_szResponsesCollected = 0;

    for (tCollectionMap::const_iterator it = m_mapModuleStates.begin();
        it != m_mapModuleStates.end() && fep::isOk(nResult); ++it)
    {
        fep::cGetPropertyCommand oCmd(
            g_strElementHeaderPath_strElementCurrentState,
            m_poModule->GetName(), it->first.c_str(), GetTimeStampMicrosecondsUTC(),
            m_poModule->GetTimingInterface()->GetTime());

        nResult = m_poModule->GetCommandAccess()->TransmitCommand(&oCmd);
    }

    return nResult;
}

fep::Result cSystemStateCollector::WaitForEvent(const timestamp_t tmTimeout)
{
    fep::Result nResult = ERR_NOERROR;
    if (tmTimeout == -1)
    {
        m_oEventFinished.wait();
    }
    else
    {
        nResult = m_oEventFinished.wait_for(a_util::chrono::milliseconds(tmTimeout));
        if (fep::isFailed(nResult))
        {
            nResult = ERR_TIMEOUT;
        }
    }

    m_poModule->GetNotificationAccess()->UnregisterNotificationListener(this);

    return nResult;
}

fep::Result fep::cSystemStateCollector::AskForStates(const std::vector<std::string>& vecElementList, const timestamp_t tmTimeoutMs)
{
    fep::Result nResult = Init(vecElementList);

    if (fep::isOk(nResult))
    {
        nResult = SendStateRequests();
    }

    if (fep::isOk(nResult))
    {
        nResult = WaitForEvent(tmTimeoutMs);
    }
    return nResult;
}

tState cSystemStateCollector::GetAggregatedState() const
{
    static const unsigned int nAmountStates = 7;

    // all states, ordered by their "weight"
    static const tState arrStates[nAmountStates] = {
        FS_SHUTDOWN,     // 0
        FS_ERROR,        // 1
        FS_STARTUP,      // 2
        FS_IDLE,         // 3
        FS_INITIALIZING, // 4
        FS_READY,        // 5
        FS_RUNNING       // 6
    };

    // and now build a table to look up the weight of a specific state
    static int arrWeight[nAmountStates];
    for (unsigned int nWeightIdx = 0; nWeightIdx < nAmountStates; ++nWeightIdx)
    {
        arrWeight[arrStates[nWeightIdx]] = nWeightIdx;
    }

    int nMinWeight = arrWeight[FS_RUNNING];
    for (tCollectionMap::const_iterator it = m_mapModuleStates.begin();
        it != m_mapModuleStates.end(); ++it)
    {
        if (arrWeight[it->second] < nMinWeight)
        {
            nMinWeight = arrWeight[it->second];
        }
    }

    return arrStates[nMinWeight];
}



cSystemStateCollector::tCollectionMap cSystemStateCollector::GetStates() const
{
    return m_mapModuleStates;
}

fep::Result cSystemStateCollector::Update(IPropertyNotification const * pPropertyNotification)
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oResponseMutex);

    // only consider property notifications during the initial stage
    if (m_szResponsesCollected < m_mapModuleStates.size())
    {
        const IProperty * poProp = pPropertyNotification->GetProperty();
        if (poProp && a_util::strings::isEqual(poProp->GetPath(), g_strElementHeaderField_strElementCurrentState))
        {
            tCollectionMap::iterator it = m_mapModuleStates.find(pPropertyNotification->GetSender());
            if (it != m_mapModuleStates.end())
            {
                const char * strState = NULL;
                if (poProp && fep::isOk(poProp->GetValue(strState)))
                {
                    tState eState = FS_SHUTDOWN;
                    cState::FromString(strState, eState);
                    OnStateReceived(it, eState);
                }
            }
        }
    }

    return ERR_NOERROR;
}

fep::Result cSystemStateCollector::Update(IStateNotification const * poStateNotification)
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oResponseMutex);

    // we only consider state notifications when we're waiting for a system state
    if (m_bWaitForSystemState)
    {
        tCollectionMap::iterator it = m_mapModuleStates.find(poStateNotification->GetSender());
        if (it != m_mapModuleStates.end())
        {
            OnStateReceived(it, poStateNotification->GetState());
        }
    }

    return ERR_NOERROR;
}
