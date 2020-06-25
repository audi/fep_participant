/**
 * Implementation of the Class cEventHistoryStrategy.
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

#include <list>
#include <string>
#include <a_util/concurrency/fast_mutex.h>
#include <a_util/datetime/datetime.h>
#include <a_util/memory/memory.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_functions.h>

#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep_errors.h"
#include "module/fep_module_intf.h"
#include "incident_handler/fep_log_history_strat.h"

using namespace fep;

#define LOG_DEBUG(msg)
//#define LOG_DEBUG(msg) LOG_INFO(msg)

/// Enum to beautify the array access
enum eHistoryBuffEntry
{
    BE_HIST_BUFF_1 = 0,
    BE_HIST_BUFF_2 = 1
};

cIncidentHistoryStrategy::sHistoryInternal::sHistoryInternal() :
    m_bLocked(false), m_nNumEntries(0)
{
    // nothing to do here really.
}

cIncidentHistoryStrategy::sHistoryInternal::~sHistoryInternal()
{
    m_dequeIncidents.clear();
}

fep::Result cIncidentHistoryStrategy::sHistoryInternal::PushBack(tIncidentEntry& sEntry)
{
    fep::Result nResult = ERR_NOERROR;

    // the internal memory is being taken care of by the ring buffer.
    // to remain clean, we have to assure that the .size() is consistent.
    if (m_dequeIncidents.size() == m_nNumEntries)
    {
        m_dequeIncidents.pop_front();
    }

    m_dequeIncidents.push_back(sEntry);

    return nResult;
}

fep::Result cIncidentHistoryStrategy::sHistoryInternal::Lock()
{
    std::unique_lock<a_util::concurrency::fast_mutex> oLock(m_oLockGuard);
    if (!m_bLocked)
    {
        m_bLocked = true;
        return ERR_NOERROR;
    }

    return ERR_ACCESS_DENIED;
}

fep::Result cIncidentHistoryStrategy::sHistoryInternal::Unlock()
{
    std::unique_lock<a_util::concurrency::fast_mutex> oLock(m_oLockGuard);
    if (m_bLocked)
    {
        m_bLocked = false;
        return ERR_NOERROR;
    }

    return ERR_ACCESS_DENIED;
}

bool cIncidentHistoryStrategy::sHistoryInternal::IsLocked()
{
    std::unique_lock<a_util::concurrency::fast_mutex> oLock(m_oLockGuard);
    return m_bLocked;
}

void cIncidentHistoryStrategy::sHistoryInternal::Resize(size_t nNumEntries)
{
#ifdef USE_EXPERIMENTAL_RT_ALLOCATOR
    m_oHistoryMemory.Resize(nNumEntries * sizeof(tIncidentEntry));
#endif
    m_nNumEntries = nNumEntries;
}

void cIncidentHistoryStrategy::sHistoryInternal::Clear()
{
    m_dequeIncidents.clear();
#ifdef USE_EXPERIMENTAL_RT_ALLOCATOR
    m_oHistoryMemory.Reset();
#endif
}

cIncidentHistoryStrategy::cIncidentHistoryStrategy()
{
    m_pWriteHistory = &m_sHistoryBuffer[BE_HIST_BUFF_1];
    m_pReadHistory = &m_sHistoryBuffer[BE_HIST_BUFF_2];

    // use as less memory as possible as long as the strategy might be disabled.
    m_sHistoryBuffer[BE_HIST_BUFF_1].Resize(1);
    m_sHistoryBuffer[BE_HIST_BUFF_2].Resize(1);
}

cIncidentHistoryStrategy::~cIncidentHistoryStrategy()
{
    a_util::memory::zero(&m_sIncidentEntry, sizeof(m_sIncidentEntry), sizeof(m_sIncidentEntry));
    a_util::memory::zero(&m_sLastIncidentEntry, sizeof(m_sLastIncidentEntry), sizeof(m_sLastIncidentEntry));
}

fep::Result cIncidentHistoryStrategy::HandleLocalIncident(fep::IModule* pModuleContext,
                                              const int16_t nIncident,
                                              const tSeverityLevel severity,
                                              const char* strOrigin,
                                              int nLine,
                                              const char* File,
                                              const timestamp_t tmSimTime,
                                              const char* strDescription)
{
    fep::Result nResult = ERR_NOERROR;

    if (m_bEnabled && pModuleContext)
    {
        std::unique_lock<std::recursive_mutex> oSync(m_oConfigGuard);
        std::unique_lock<std::recursive_mutex> oSync2(m_oSwitchGuard);
        std::unique_lock<std::recursive_mutex> oSync3(m_oHandleGuard);
        m_sIncidentEntry.nTimeStamp = a_util::datetime::getCurrentLocalDateTime().toTimestamp();
        m_sIncidentEntry.nIncident = nIncident;
        m_sIncidentEntry.eSeverity = severity;
        m_sIncidentEntry.tmSimTime = tmSimTime;

        // this is not really deterministic but it'll do fine
        const char *strSource = pModuleContext->GetName();
        a_util::strings::copy(m_sIncidentEntry.strSource, SOURCE_NAME_LENGTH, strSource);
        if(NULL != strOrigin)
        {
            a_util::strings::copy(m_sIncidentEntry.strOrigin, ENTRY_ORIGIN_LENGTH, strOrigin);
        }
        a_util::strings::copy(m_sIncidentEntry.strMessage, ENTRY_MESSAGE_LENGTH, strDescription);

        nResult = m_pWriteHistory->PushBack(m_sIncidentEntry);
    }

    return nResult;
}

fep::Result cIncidentHistoryStrategy::HandleGlobalIncident(const char *strSource,
                                                    const int16_t nIncident,
                                                    const tSeverityLevel severity,
                                                    const timestamp_t tmSimTime,
                                                    const char *strDescription)
{
    std::unique_lock<std::recursive_mutex> oSync(m_oConfigGuard);
    std::unique_lock<std::recursive_mutex> oSync2(m_oSwitchGuard);
    std::unique_lock<std::recursive_mutex> oSync3(m_oHandleGuard);

    fep::Result nResult = ERR_NOERROR;

    if (m_bEnabled)
    {
        m_sIncidentEntry.nTimeStamp = a_util::datetime::getCurrentLocalDateTime().toTimestamp();
        m_sIncidentEntry.nIncident = nIncident;
        m_sIncidentEntry.eSeverity = severity;
        m_sIncidentEntry.tmSimTime = tmSimTime;

        // this is not really deterministic but it'll do fine
        a_util::strings::copy(m_sIncidentEntry.strSource, SOURCE_NAME_LENGTH, strSource);
        a_util::strings::copy(m_sIncidentEntry.strMessage, ENTRY_MESSAGE_LENGTH, strDescription);

        nResult = m_pWriteHistory->PushBack(m_sIncidentEntry);
    }

    return nResult;
}


fep::Result cIncidentHistoryStrategy::PurgeHistory()
{
    std::unique_lock<std::recursive_mutex> oSync(m_oSwitchGuard);

    m_sHistoryBuffer[BE_HIST_BUFF_1].Clear();
    m_sHistoryBuffer[BE_HIST_BUFF_2].Clear();

    return ERR_NOERROR;
}

fep::Result cIncidentHistoryStrategy::LockHistory(tIncidentListConstIter& io_iterHistBegin,
                                              tIncidentListConstIter& io_iterHistEnd)
{
    fep::Result nResult = ERR_NOERROR;

    if (!m_bEnabled)
    {
        nResult = ERR_NOT_READY;
    }

    std::unique_lock<std::recursive_mutex> oSync(m_oSwitchGuard);

    if (fep::isOk(nResult))
    {
        // first, check whether the other side of the buffer is currently in use or not!
        if (m_pReadHistory->IsLocked())
        {
            LOG_DEBUG("Unable to lock the incident history; A lock is still in place.");
            nResult = ERR_DEVICE_IN_USE;
        }
    }

    if (fep::isOk(nResult) &&
        m_pWriteHistory->m_dequeIncidents.begin() == m_pWriteHistory->m_dequeIncidents.end())
    {
        nResult = ERR_EMPTY;
    }

    if (fep::isOk(nResult) && fep::isFailed(m_pWriteHistory->Lock()))
    {
        LOG_DEBUG("Unable to lock the incident history! This is fatal!");
        nResult = ERR_ACCESS_DENIED;
    }

    if (fep::isOk(nResult))
    {
        io_iterHistBegin = m_pWriteHistory->m_dequeIncidents.begin();
        io_iterHistEnd = m_pWriteHistory->m_dequeIncidents.end();

        sHistoryInternal* pTmpHistory = m_pWriteHistory;
        m_pWriteHistory = m_pReadHistory;
        m_pReadHistory = pTmpHistory;

        m_pWriteHistory->Clear();
    }

    return nResult;
}

fep::Result cIncidentHistoryStrategy::UnlockHistory()
{
    std::unique_lock<std::recursive_mutex> oSync(m_oSwitchGuard);

    fep::Result nResult = m_pReadHistory->Unlock();
    if (fep::isOk(nResult))
    {
        m_pReadHistory->Clear();
    }

    return nResult;
}

fep::Result cIncidentHistoryStrategy::GetLastIncident(const tIncidentEntry** pIncidentEntry)
{
    fep::Result nResult = ERR_NOERROR;

    if (!pIncidentEntry)
    {
        nResult = ERR_POINTER;
    }

    if (fep::isOk(nResult) && !m_bEnabled)
    {
        nResult = ERR_NOT_READY;
    }

    std::unique_lock<std::recursive_mutex> oSync(m_oSwitchGuard);
    std::unique_lock<std::recursive_mutex> oSync2(m_oHandleGuard);

    if (fep::isOk(nResult))
    {
        // m_pReadHistory is either invalid or already requested by the user
        // hence, the only part of interest is the latest entry in the m_pWriteHistory.
        // -> The entry is NOT invalidated or removed (would make no sense if there is
        // more in the deque).
        if (0 < m_pWriteHistory->m_dequeIncidents.size())
        {
            m_sLastIncidentEntry = m_pWriteHistory->m_dequeIncidents.back();
            *pIncidentEntry = &m_sLastIncidentEntry;
        }
        else
        {
            nResult = ERR_EMPTY;
        }
    }

    return nResult;
}

fep::Result cIncidentHistoryStrategy::RefreshConfiguration(
        const fep::IProperty* pStrategyProperty,
        const fep::IProperty* pAffectedProperty)
{
    fep::Result nResult = ERR_NOERROR;

    if (!pStrategyProperty || !pAffectedProperty)
    {
        nResult = ERR_POINTER;
    }

    std::unique_lock<std::recursive_mutex> oSync(m_oConfigGuard);

    if (fep::isOk(nResult))
    {
        // distinguish two calls to this method:
        // 1) a specific property changed (and the full path is known)
        // 2) the "root" path for this strategy is being given after a global
        // configuration or an association with the incident handler.

        if (std::string(pAffectedProperty->GetPath()) == component_config::g_strIncidentHistoryLogBase)
        {
            // we need a full rewind here; only the first level is required, really.
            IProperty::tPropertyList::const_iterator itSubProperty =
                    pStrategyProperty->GetBeginIterator();
            for (; itSubProperty != pStrategyProperty->GetEndIterator() && fep::isOk(nResult);
                 itSubProperty++)
            {
                nResult = RefreshConfiguration(pStrategyProperty, (*itSubProperty));
            }
        }

        else if (std::string(pAffectedProperty->GetPath()) == component_config::g_strIncidentHistoryLogPath_bEnable)
        {
            nResult = pAffectedProperty->GetValue(m_bEnabled);
        }

        else if (std::string(pAffectedProperty->GetPath()) == component_config::g_strIncidentHistoryLogPath_nQueueSize)
        {
            if (m_pWriteHistory->IsLocked() || m_pReadHistory->IsLocked())
            {
                LOG_DEBUG("The history is currently locked; resizing not possible!");
                nResult = ERR_ACCESS_DENIED;
            }
            if(fep::isOk(nResult))
            {
                int32_t nValue = 0;
                nResult = pAffectedProperty->GetValue(nValue);
                if (fep::isOk(nResult))
                {
                    m_pWriteHistory->Resize(nValue);
                    m_pReadHistory->Resize(nValue);
                    LOG_DEBUG(a_util::strings::format("Incident History resized to length of %d entries",
                                              nValue));
                }
            }
        }
    }

    return nResult;
}
