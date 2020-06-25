/**
 * Implementation of the Class cDataAccess.
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
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <a_util/memory/memory.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>

#include "_common/fep_optional.h"
#include "_common/fep_timestamp.h"
#include "data_access/fep_data_sample_buffer.h"
#include "data_access/fep_signal_counter.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/fep_module_header_config.h"
#include "fep3/components/legacy/property_tree/propertytreebase_intf.h"
#include "fep_data_access_common.h"
#include "fep_errors.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "mapping/fep_mapping.h"
#include "messages/fep_command_mute_signal_intf.h"
#include "messages/fep_notification_resultcode.h"
#include "signal_registry/fep_signal_registry.h"
#include "signal_registry/fep_signal_struct.h"
#include "transmission_adapter/fep_data_listener_adapter.h"
#include "transmission_adapter/fep_preparation_data_listener_intf.h"
#include "transmission_adapter/fep_preparation_data_sample_intf.h"
#include "transmission_adapter/fep_signal_direction.h"
#include "transmission_adapter/fep_transmission_adapter_intf.h"
#include "transmission_adapter/fep_user_data_sample_intf.h"
#include "data_access/fep_data_access.h"

using namespace fep;

cDataAccess::cDataAccess() :
    m_bIsInitialized(false), m_poTransmissionAdapter(NULL),
    m_poSignalRegistryPrivate(NULL), m_poSignalMappingPrivate(NULL),
    m_poIncidentHandler(NULL), m_poPropertyTree(NULL)
{
}

cDataAccess::~cDataAccess()
{
    Finalize();
}

fep::Result cDataAccess::Initialize(ITransmissionAdapter* poTransmissionAdapter,
    ISignalRegistryPrivate* poSignalRegistryPrivate,
    ISignalMappingPrivate* poSignalMappingPrivate,
    IIncidentHandler* poIncidentHandler,
    IPropertyTreeBase* poPropertyTree)
{
    fep::Result nRes = ERR_NOERROR; 
    
    if (m_bIsInitialized)
    {
        nRes = ERR_UNEXPECTED;
    }

    if (fep::isOk(nRes))
    {
        if (!poTransmissionAdapter || !poSignalRegistryPrivate || !poSignalMappingPrivate || !poIncidentHandler || !poPropertyTree)
        {
            nRes = ERR_INVALID_ARG;
        }
    }

    if (fep::isOk(nRes))
    {
        m_poTransmissionAdapter = poTransmissionAdapter;
        m_poSignalRegistryPrivate = poSignalRegistryPrivate;
        m_poSignalMappingPrivate = poSignalMappingPrivate;
        m_poIncidentHandler = poIncidentHandler;
        m_poPropertyTree = poPropertyTree;
    }

    if (fep::isOk(nRes))
    {
        nRes = m_poTransmissionAdapter->RegisterCommandListener(this);
    }

    if (fep::isOk(nRes))
    {
        m_bIsInitialized = true;
    }

    return nRes;
}

fep::Result cDataAccess::Finalize()
{
    fep::Result nRes = ERR_NOERROR;

    if (!m_bIsInitialized)
    {
        nRes = ERR_UNEXPECTED;
    }

    if (fep::isOk(nRes))
    {
        nRes = m_poTransmissionAdapter->UnregisterCommandListener(this);
    }

    if (fep::isOk(nRes))
    {
        while (0 < m_listListenerAdapter.size())
        {
            UnregisterDataListener(m_listListenerAdapter.front()->GetUserDataListener(),
                m_listListenerAdapter.front()->GetSignalHandle());
        }

        while (0 < m_mapSampleBuffers.size())
        {
            cDataSampleBuffer* pSampleBuffer= m_mapSampleBuffers.begin()->second;
            m_mapSampleBuffers.erase(m_mapSampleBuffers.begin());
            delete pSampleBuffer;
        }

        m_poTransmissionAdapter = NULL;
        m_poSignalRegistryPrivate = NULL;
        m_poSignalMappingPrivate = NULL;
        m_poIncidentHandler = NULL;
        m_poPropertyTree = NULL;
    }

    // Always set to deinitialized
    m_bIsInitialized = false;

    return nRes;
}

fep::Result cDataAccess::RegisterSignal(tSignal& oSignal,
    handle_t& hSignalHandle)
{
    if (oSignal.strSignalName.empty() || oSignal.eDirection == SD_Undefined) { return ERR_POINTER; }

    fep::Result nResult = ERR_NOERROR;

    if (fep::SD_Input == oSignal.eDirection)
    {
        if (!oSignal.bIsRaw.GetValue())
        {
            nResult = m_poSignalMappingPrivate->
                RegisterSignal(oSignal, hSignalHandle);
            if (nResult == ERR_UNKNOWN_FORMAT)
            {
                INVOKE_INCIDENT(m_poIncidentHandler,
                    FSI_MAPPED_SIGNAL_INCONSISTENCY_FAIL, SL_Critical_Local,
                    a_util::strings::format("Registration of mapped signal %s failed because "
                        "the registered mapping configuration is "
                        "inconsistent with the registered signal description",
                        oSignal.strSignalName.c_str()).c_str());
            }
            else if (nResult == ERR_NOT_FOUND)
            {
                // mapping not found -> relay to transmission adapter
                nResult = m_poTransmissionAdapter->RegisterSignal(oSignal, hSignalHandle);
            }
            else if (fep::isOk(nResult))
            {
                // mapping registration went fine
                oSignal.bIsMapped = true;
            }
        }
        else
        {
            // variable signal size signal
            nResult = m_poTransmissionAdapter->RegisterSignal(oSignal, hSignalHandle);
        }
    }
    else if (fep::SD_Output == oSignal.eDirection)
    {
        /* create cSignalCounter*/
        cSignalCounter* poSignalCounter = new cSignalCounter();

        nResult = poSignalCounter->Create(oSignal,
            m_poPropertyTree, m_poTransmissionAdapter);

        if (fep::isOk(nResult))
        {
            m_listSignalCounters.push_back(poSignalCounter);
            hSignalHandle = poSignalCounter;
        }

        if (fep::isFailed(nResult))
        {
            delete poSignalCounter;
        }
    }

    return nResult;
}

fep::Result cDataAccess::SignalRegistered(handle_t hSignal)
{
    size_t szBacklog = 0;
    RETURN_IF_FAILED(m_poSignalRegistryPrivate->GetSignalSampleBacklog(hSignal, szBacklog));

    // preallocate the sample backlog
    cDataSampleBuffer* pSampleBuffer = NULL;
    tSampleBuffers::iterator it = m_mapSampleBuffers.find(hSignal);
    if (it == m_mapSampleBuffers.end())
    {
        pSampleBuffer = new cDataSampleBuffer;
        m_mapSampleBuffers.insert(std::make_pair(hSignal, pSampleBuffer));
    }
    else
    {
        pSampleBuffer = it->second;
    }
    
    // insert all but the first sample
    for (size_t i = 0; i < szBacklog - 1; ++i)
    {
        IUserDataSample* pSample = NULL;
        CreateUserDataSample(pSample, hSignal);
        pSample->SetTime(-1); // this marks the sample as unused

        pSampleBuffer->m_samples.insert(tSampleSlot(pSample));
    }

    // insert default buffer as sample at t=0
    IUserDataSample* pSample = NULL;
    CreateUserDataSample(pSample, hSignal);

    if (m_poSignalRegistryPrivate->IsMappedSignal(hSignal))
    {
        m_poSignalMappingPrivate->CopyBuffer(hSignal,
            pSample->GetPtr(), pSample->GetSize());
    }
    else
    {
        m_poTransmissionAdapter->GetRecentSample(hSignal,
            dynamic_cast<IPreparationDataSample*>(pSample));
    }

    pSample->SetTime(0);
    pSampleBuffer->m_samples.insert(tSampleSlot(pSample));

    // register myself as data listener
    RegisterDataListener(this, hSignal);

    return ERR_NOERROR;
}

fep::Result cDataAccess::UnregisterSignal(const tSignal& oSignal, handle_t hSignalHandle)
{
    fep::Result nResult = ERR_NOERROR;

    if (oSignal.eDirection == fep::SD_Output)
    {
        cSignalCounter * poSignalChoke = static_cast<cSignalCounter*>(hSignalHandle);
        // destructor of the signal choke calls the unregister of the tranmission adapter
        delete poSignalChoke;
        /* remove signal from internal list */
        m_listSignalCounters.remove(poSignalChoke);
    }
    else if (oSignal.eDirection == fep::SD_Input)
    {
        if(oSignal.bIsMapped)
        {
            nResult = m_poSignalMappingPrivate->UnregisterSignal(hSignalHandle);
            SignalUnregistered(hSignalHandle);
        }
        else
        {
            // we need to inform the data access now, before the signal is gone,
            // so that it has a chance to unregister any data listeners
            SignalUnregistered(hSignalHandle);
            nResult = m_poTransmissionAdapter->UnregisterSignal(hSignalHandle);
        }
    }

    return nResult;
}

fep::Result cDataAccess::SignalUnregistered(handle_t hSignal)
{
    tDataListenerAdapterContainer lstTemp;
    for (tDataListenerAdapterContainer::iterator it = m_listListenerAdapter.begin();
        it != m_listListenerAdapter.end(); ++it)
    {
        if ((*it)->GetSignalHandle() == hSignal)
        {
            lstTemp.push_back(*it);
        }
    }

    for (tDataListenerAdapterContainer::iterator it = lstTemp.begin();
        it != lstTemp.end(); ++it)
    {
        // Multiple UserDataListeners can be registered to a chunk listener (via a data listener adapter)
        // we need to unregister all these listeners
        fep::IUserDataListener * UDListener = (*it)->GetUserDataListener();

        UnregisterDataListener(UDListener,hSignal);
    }

    tSampleBuffers::iterator it = m_mapSampleBuffers.find(hSignal);
    if (it != m_mapSampleBuffers.end())
    {
        cDataSampleBuffer* pSampleBuffer = it->second;
        m_mapSampleBuffers.erase(it);
        delete pSampleBuffer;
    }

    return ERR_NOERROR;
}

fep::Result cDataAccess::MuteSignal(handle_t hSignal)
{
    // muting info
    const char * strSigName = NULL;
    m_poSignalRegistryPrivate->GetSignalNameFromHandle(hSignal, strSigName);
    std::string strSetMuteProp = a_util::strings::format("%s.%s.%s",
        fep::component_config::g_strSignalRegistryPath_RegisteredOutSignals, strSigName,
        fep::component_config::g_strSignalRegistryField_MutedSignal);
    m_poPropertyTree->SetPropertyValue(strSetMuteProp.c_str(), true);
    handle_t hAdapterHandle = GetInternalHandle(hSignal);
    return m_poTransmissionAdapter->MuteSignal(hAdapterHandle);
}

fep::Result cDataAccess::UnmuteSignal(handle_t hSignal)
{
    // muting info
    const char * strSigName = NULL;
    m_poSignalRegistryPrivate->GetSignalNameFromHandle(hSignal, strSigName);
    std::string strSetMuteProp = a_util::strings::format("%s.%s.%s",
        fep::component_config::g_strSignalRegistryPath_RegisteredOutSignals, strSigName,
        fep::component_config::g_strSignalRegistryField_MutedSignal);
    m_poPropertyTree->SetPropertyValue(strSetMuteProp.c_str(), false);
    handle_t hAdapterHandle = GetInternalHandle(hSignal);
    return  m_poTransmissionAdapter->UnmuteSignal(hAdapterHandle);
}

fep::Result cDataAccess::MuteAll()
{
    m_poPropertyTree->SetPropertyValue(fep::g_strElementHeaderPath_bGlobalMute, true);

    for (tSignalCounterContainer::iterator it = m_listSignalCounters.begin(); it != m_listSignalCounters.end(); ++it)
    {
        handle_t hMuteHandle = (*it)->GetInternalSignalHandle();
        m_poTransmissionAdapter->MuteSignal(hMuteHandle);
    }
    return ERR_NOERROR;
}

fep::Result cDataAccess::UnmuteAll()
{
    m_poPropertyTree->SetPropertyValue(fep::g_strElementHeaderPath_bGlobalMute, false);

    for (tSignalCounterContainer::iterator it = m_listSignalCounters.begin(); it != m_listSignalCounters.end(); ++it)
    {
        handle_t hUnmuteHandle = (*it)->GetInternalSignalHandle();
        m_poTransmissionAdapter->UnmuteSignal(hUnmuteHandle);
    }
    return ERR_NOERROR;
}

handle_t cDataAccess::GetInternalHandle(handle_t hSignalHandle) const
{
    cSignalCounter * poSignalCounter = static_cast<cSignalCounter*>(hSignalHandle);
    return poSignalCounter->GetInternalSignalHandle();
}

fep::Result cDataAccess::SignalBacklogChanged(handle_t hSignal, size_t szSampleBacklog)
{
    size_t szSize = 0;
    fep::Result nResult = m_poSignalRegistryPrivate->GetSignalSampleSize(hSignal, szSize);

    if (fep::isOk(nResult))
    {
        // find matching backlog
        tSampleBuffers::iterator it = m_mapSampleBuffers.find(hSignal);
        if (it == m_mapSampleBuffers.end())
        {
            nResult = ERR_INVALID_ARG;
        }

        if (fep::isOk(nResult))
        {
            cDataSampleBuffer* pDataSampleBuffer = it->second;
            nResult = pDataSampleBuffer->SignalBacklogChanged(hSignal, szSampleBacklog, szSize);
        }
    }

    return nResult;
}


fep::Result cDataAccess::LockData(handle_t hSignal, const fep::IUserDataSample*& poSample)
{
    return LockDataAt(hSignal, poSample, 0, SS_LATEST_SAMPLE);
}

fep::Result cDataAccess::LockDataAt(handle_t hSignal, const fep::IUserDataSample*& poSample,
    timestamp_t tmSimulation, uint32_t eSelectionFlags)
{
    if (!m_poSignalMappingPrivate->HandleHasTriggers(hSignal) &&
        m_poSignalRegistryPrivate->IsMappedSignal(hSignal))
    {
        IUserDataSample* pSample = nullptr;
        CreateUserDataSample(pSample, hSignal);
        fep::Result nRes = m_poSignalMappingPrivate->CopyBuffer(hSignal, pSample->GetPtr(),
            pSample->GetSize());
        if (nRes == ERR_NOERROR)
        {
            poSample = pSample;
        }
        else
        {
            delete pSample;
        }
        return nRes;
    }

    cDataSampleBuffer* pDataSampleBuffer;
    if (fep::isFailed(GetSampleBuffer(hSignal, pDataSampleBuffer)))
    {
        return ERR_INVALID_ARG;
    }
    bool bSampleIsvalid;
    return pDataSampleBuffer->LockDataAt(poSample, bSampleIsvalid, tmSimulation, eSelectionFlags);
}

fep::Result cDataAccess::UnlockData(const fep::IUserDataSample* poSample)
{
    if (!poSample) return ERR_INVALID_ARG;

    cDataSampleBuffer* pDataSampleBuffer;
    if (fep::isFailed(GetSampleBuffer(poSample->GetSignalHandle(), pDataSampleBuffer)))
    {
        return ERR_UNEXPECTED;
    }

    return pDataSampleBuffer->UnlockData(poSample);
}

fep::Result cDataAccess::RegisterDataListener(IUserDataListener* poDataListener, handle_t hSignalHandle)
{
    if (!poDataListener) { return ERR_POINTER; }
    fep::Result nResult = ERR_NOERROR;
    if (m_poSignalRegistryPrivate->IsMappedSignal(hSignalHandle))
    {
        nResult = m_poSignalMappingPrivate->RegisterDataListener(poDataListener, hSignalHandle);
    }
    else
    {
        fep::cDataListenerAdapter * poDataListenerAdapter;

        // Register Listener for a regular signal

        /* as RegisterDataListener should only be called for RX signals,
        * hSignalHandle SHOULD not be a cSignalChoke and thus does not need to
        * become reinterpreted => just forward to "real" ITransmissionAdapter */
        poDataListenerAdapter =
            new cDataListenerAdapter(poDataListener, m_poIncidentHandler, hSignalHandle);


        nResult = m_poTransmissionAdapter->RegisterDataListener(
            poDataListenerAdapter, hSignalHandle);
        if (fep::isOk(nResult))
        {
            m_listListenerAdapter.push_back(poDataListenerAdapter);
        }
        else
        {
            delete poDataListenerAdapter;
        }
    }

    return nResult;
}

fep::Result cDataAccess::TransmitData(IUserDataSample* poSample, bool bSync)
{
    if (!poSample) { return ERR_POINTER; }
    // TBD: is this needed?
    //MEASURE_POINT(ApiTransmitCalled);

    fep::Result nResult = ERR_NOERROR;

    cSignalCounter * poSignalCounter =
        static_cast<cSignalCounter*>(poSample->GetSignalHandle());
    IPreparationDataSample* poPrepSample =
        dynamic_cast<IPreparationDataSample*>(poSample);

    assert(poPrepSample);  // if this throws; the FEP Core programmers failed epically!

    if (!poSignalCounter)
    {
        nResult = ERR_POINTER;
    }
    else
    {
        poPrepSample->SetSyncFlag(bSync);
        nResult = poSignalCounter->SendNow(poPrepSample);
    }
    return nResult;
}

fep::Result cDataAccess::UnregisterDataListener(IUserDataListener* poDataListener, const handle_t hSignalHandle)
{
    if (!poDataListener) { return ERR_POINTER; }

    fep::Result nResult = ERR_NOERROR;
    if (m_poSignalRegistryPrivate->IsMappedSignal(hSignalHandle))
    {
        nResult = m_poSignalMappingPrivate->UnregisterDataListener(poDataListener, hSignalHandle);
    }
    else
    {
        /* as UnregisterDataListener should only be called for RX signals,
        * hSignalHandle SHOULD not be a cSignalChoke and thus does not need to
        * become reinterpreted => just forward to "real" ITransmissionAdapter */

        fep::IPreparationDataListener * poDataListenerAdapter = NULL;
        tDataListenerAdapterContainer::iterator itAdapterList =
            m_listListenerAdapter.begin();

        /* only first occurrence of is handled and removed (&& NULL ==
        * poDataListenerAdapter); if one module is registered multiple times for
        * one signal, it has to be de-registered multiple times aswell; */
        while (itAdapterList != m_listListenerAdapter.end() && NULL == poDataListenerAdapter)
        {
            fep::IUserDataListener* pIUDListener = (*itAdapterList)->GetUserDataListener();
       
                if (poDataListener == pIUDListener &&
                    hSignalHandle == (*itAdapterList)->GetSignalHandle())
                {
                    poDataListenerAdapter = *itAdapterList;
                }

            if(NULL == poDataListenerAdapter)
            {
                ++itAdapterList;
            }

        }

        if (NULL == poDataListenerAdapter)
        {
            nResult = ERR_NOT_FOUND;
        }
        else
        {
            nResult =
                m_poTransmissionAdapter->UnregisterDataListener(poDataListenerAdapter, hSignalHandle);
            itAdapterList = m_listListenerAdapter.erase(itAdapterList);
            delete poDataListenerAdapter;
        }
    }
    return nResult;
}

fep::Result cDataAccess::CreateUserDataSample(IUserDataSample*& pSample, const handle_t hSignal) const
{
    size_t szSize = 0;
    fep::Result nResult = ERR_NOERROR;
    if (hSignal != NULL)
    {
        nResult = m_poSignalRegistryPrivate->GetSignalSampleSize(hSignal, szSize);
    }

    if (fep::isOk(nResult))
    {
        nResult = cDataSampleBuffer::CreateUserDataSample(pSample, hSignal, szSize);
    }

    return nResult;
}

fep::Result cDataAccess::GetSampleBuffer(handle_t hSignalHandle, cDataSampleBuffer*& pBuffer)
{
    fep::Result nResult = ERR_NOT_FOUND;
    tSampleBuffers::iterator it = m_mapSampleBuffers.find(hSignalHandle);
    if (it != m_mapSampleBuffers.end())
    {
        nResult = ERR_NOERROR;
        pBuffer = it->second;
    }
    return nResult;
}

fep::Result cDataAccess::LockDataAtUpperBound(handle_t hSignalHandle, const fep::IUserDataSample *& poSample, bool& bSampleIsvalid,
    timestamp_t tmSimulationUpperBound)
{
    fep::Result nResult = ERR_NOT_FOUND;
    cDataSampleBuffer* pBuffer;
    if (isOk(GetSampleBuffer(hSignalHandle, pBuffer)))
    {
        nResult = pBuffer->LockDataAtUpperBound(poSample, bSampleIsvalid, tmSimulationUpperBound);
    }
    return nResult;
}

fep::Result cDataAccess::Update(const IUserDataSample* poSample)
{
    cDataSampleBuffer* pDataSampleBuffer;
    if (fep::isFailed(GetSampleBuffer(poSample->GetSignalHandle(), pDataSampleBuffer)))
    {
        return ERR_UNEXPECTED;
    }

    fep::Result nRes = pDataSampleBuffer->Update(poSample);

    // the only error returned by cDataSampleBuffer::Update is ERR_RESOURCE_IN_USE
    // Report this error and return
    if (fep::isFailed(nRes))
    {
        // if we haven't got an empty slot, drop it and return
        INVOKE_INCIDENT(m_poIncidentHandler, FSI_SAMPLE_DROPPED_FROM_BACKLOG, SL_Warning,
            "A sample was dropped from the receive backlog since all available slots are locked for reading!");
        // behave like previous
        nRes = ERR_NOERROR;
    }

    return nRes;
}

fep::Result cDataAccess::Update(IMuteSignalCommand const * poCommand)
{
    fep::Result nResult = ERR_NOERROR;
    std::string strAffectedSignal = poCommand->GetSignalName();
    bool bMuteSignal = poCommand->GetMutingStatus();
    if (poCommand->GetSignalDirection() == fep::SD_Input)
    {
        nResult = ERR_NOT_SUPPORTED;
    }
    if (isOk(nResult))
    {
        if (strAffectedSignal == "*")
        {
            if (bMuteSignal)
            {
                nResult = MuteAll();
            }
            else
            {
                nResult = UnmuteAll();
            }
        }
        else
        {
            handle_t hSignal;
            nResult = m_poSignalRegistryPrivate->GetSignalHandleFromName(strAffectedSignal.c_str(),
                poCommand->GetSignalDirection(),
                hSignal);
            if (isOk(nResult))
            {
                if (bMuteSignal)
                {
                    nResult = MuteSignal(hSignal);
                }
                else
                {
                    nResult = UnmuteSignal(hSignal);
                }
            }
        }
    }
    cResultCodeNotification oNotification(poCommand->GetCommandCookie(), nResult,
        poCommand->GetReceiver(), poCommand->GetSender(), GetTimeStampMicrosecondsUTC(),
        poCommand->GetSimulationTime());
    nResult = m_poTransmissionAdapter->TransmitNotification(&oNotification);
    return nResult;
}

fep::Result cDataAccess::ClearAll()
{
    {
        // Clear all sample backlogs upon IdleEntry. The user has a chance to cleanup
        // in RunningExit. Thus we can warn upon any unreleased samples here.
        for (tSampleBuffers::iterator it = m_mapSampleBuffers.begin();
            it != m_mapSampleBuffers.end(); ++it)
        {
            cDataSampleBuffer* pSampleSlotMap = it->second;

            // lock backlog
            pSampleSlotMap->m_lock.lock();

            tSampleSlots& oBuffer = pSampleSlotMap->m_samples;
            tSampleSlots oNewSlots;
            for (tSampleSlots::const_iterator it2 = oBuffer.begin(); it2 != oBuffer.end(); ++it2)
            {
                if (it2->nLockCount > 0)
                {
                    INVOKE_INCIDENT(m_poIncidentHandler, FSI_SAMPLE_STILL_LOCKED, SL_Warning,
                        a_util::strings::format("Sample at 0x%p has non-zero lock count. This can lead to "
                            "undefined behaviour since all samples in the backlog are reset after exiting FS_RUNNING!",
                            (void*)it2->poSample.get()).c_str());
                }

                // take out sample, actual sample is moved to the new copy
                tSampleSlot& oSample = const_cast<tSampleSlot&>(*it2);
                tSampleSlot oCopy(oSample);

                // zero sample
                oCopy.tmSample = -1;
                oCopy.nLockCount = 0;
                a_util::memory::zero(oCopy.poSample->GetPtr(), oCopy.poSample->GetCapacity(), oCopy.poSample->GetCapacity());
                oCopy.poSample->SetTime(-1);

                // insert copy
                oNewSlots.insert(oCopy);
            }
            pSampleSlotMap->m_samples = oNewSlots;

            oBuffer = pSampleSlotMap->m_samples;

            // reinsert one sample with default signal content
            tSampleSlots::const_iterator it2 = oBuffer.begin();
            tSampleSlot& oSample = const_cast<tSampleSlot&>(*it2);
            tSampleSlot oCopy(oSample);
            oBuffer.erase(it2);

            handle_t hSignal = oCopy.poSample->GetSignalHandle();
            if (m_poSignalRegistryPrivate->IsMappedSignal(hSignal))
            {
                m_poSignalMappingPrivate->CopyBuffer(hSignal,
                    oCopy.poSample->GetPtr(), oCopy.poSample->GetSize());
            }
            else
            {
                m_poTransmissionAdapter->GetRecentSample(hSignal,
                    dynamic_cast<IPreparationDataSample*>(oCopy.poSample.get()));
            }

            oCopy.poSample->SetTime(0);
            oCopy.tmSample = 0;
            oBuffer.insert(oCopy);

            while (!pSampleSlotMap->m_oDeletedSlots.empty())
            {
                tSampleSlots::iterator it_del = pSampleSlotMap->m_oDeletedSlots.begin();
                if (it_del->nLockCount > 0)
                {
                    INVOKE_INCIDENT(m_poIncidentHandler, FSI_SAMPLE_STILL_LOCKED, SL_Warning,
                        a_util::strings::format("Sample at 0x%p has non-zero lock count. This can lead to "
                            "undefined behaviour since all samples in the backlog are reset after exiting FS_RUNNING!",
                            (void*)it_del->poSample.get()).c_str());
                }

                pSampleSlotMap->m_oDeletedSlots.erase(it_del);
            }

            // unlock backlog
            pSampleSlotMap->m_lock.unlock();
        }
    }

    return ERR_NOERROR;
}
