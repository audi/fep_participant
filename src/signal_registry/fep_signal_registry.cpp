/**
* Implementation of the Class cSignalRegistry.
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

#include "signal_registry/fep_signal_registry.h"
#include <cassert>
#include <memory>
#include <utility>
#include <vector>
#include <a_util/filesystem/filesystem.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>
#include <a_util/strings/strings_functions.h>

#include "_common/fep_optional.h"
#include "_common/fep_stringlist.h"
#include "_common/fep_stringlist_intf.h"
#include "_common/fep_timestamp.h"
#include "data_access/fep_data_access.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep_dptr.h"
#include "fep_errors.h"
#include "fep_signal_registry_common.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "mapping/fep_mapping.h"
#include "messages/fep_command_access_intf.h"
#include "messages/fep_command_get_signal_info_intf.h"
#include "messages/fep_command_resolve_signal_type_intf.h"
#include "messages/fep_command_signal_description_intf.h"
#include "messages/fep_notification_access_intf.h"
#include "messages/fep_notification_resultcode.h"
#include "messages/fep_notification_signal_description.h"
#include "messages/fep_notification_signal_info.h"
#include "module/fep_module_intf.h"
#include "module/fep_module_private_intf.h"
#include "signal_registry/fep_user_signal_options.h"
#include "signal_registry/fep_user_signal_options_private.h"
#include "transmission_adapter/fep_serialization_helpers.h"
#include "transmission_adapter/fep_signal_serialization.h"
#include "transmission_adapter/fep_transmission.h"

using namespace fep;
using namespace fep::component_config;

/// Mock property for limiter
static const char* const s_strLimiterPath_bGlobalOn = "ComponentConfig.Limiter.bGlobalOn";
/// Default signal mute status
static const bool s_bDefaultMuteStatus = false;

cSignalRegistry::cSignalRegistry() :
m_poModule(NULL), m_poAdapter(NULL),
    m_poMappingComponent(NULL), m_poDataAccess(NULL), 
    m_pPropertyTree(NULL),
    m_bEnabledSerialization(true),
    m_bAllowRegistration(true)
{
}

cSignalRegistry::~cSignalRegistry()
{
    SetModule(NULL);
}

fep::Result cSignalRegistry::SetModule(IModulePrivate * pPrivateModule)
{
    fep::Result nRes = ERR_NOERROR;
    IModule* pPublicModule = pPrivateModule ? pPrivateModule->GetModule() : NULL;
    if (pPublicModule != m_poModule)
    {
        if (NULL != m_poModule)
        {
            if (NULL != m_pPropertyTree)
            {
                m_pPropertyTree->UnregisterListener(g_strDescriptionPath_strRemoteDescription, this);
                m_pPropertyTree->UnregisterListener(g_strSignalRegistryPath_bSerialization, this);
            }
            m_poModule->GetCommandAccess()->UnregisterCommandListener(this);
            ClearSignalDescriptions();
            m_poAdapter = NULL;
            m_poMappingComponent = NULL;
            m_poDataAccess = NULL;
            m_pPropertyTree = NULL;
            m_strRemoteDescriptionPath.clear();
            
        }

        m_poModule = pPublicModule;

        if (NULL != m_poModule)
        {
            m_oDescriptionMan.ClearDDL();
            m_poModule->GetCommandAccess()->RegisterCommandListener(this);

            m_poAdapter = pPrivateModule->GetTransmissionAdapter();
            m_poMappingComponent = pPrivateModule->GetSignalMapping();
            m_poDataAccess = pPrivateModule->GetDataAccess();
            m_pPropertyTree = m_poModule->GetPropertyTree();

            if (!m_poAdapter || !m_poMappingComponent || !m_poDataAccess)
            {
                nRes = ERR_UNEXPECTED;
            }
            if (fep::isOk(nRes))
            {
                nRes = CreateDefaultProperties();
            }
            if (NULL != m_pPropertyTree)
            {
                m_pPropertyTree->RegisterListener(g_strSignalRegistryPath_bSerialization, this);
                m_pPropertyTree->SetPropertyValue(g_strDescriptionPath_strRemoteDescription, "");
                m_pPropertyTree->RegisterListener(g_strDescriptionPath_strRemoteDescription, this);
            }
        }
    }
    return nRes;
}

fep::Result cSignalRegistry::RegisterSignal(const cUserSignalOptions & oUserSignalOptions,
    handle_t& hSignalHandle)
{
    fep::Result nResult = ERR_NOERROR;
    if(m_bAllowRegistration)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oRegistrationMutex);
        if (!oUserSignalOptions.CheckValidity()) { return ERR_INVALID_ARG; }

        const char* strSignalDescription = NULL;
        if(!oUserSignalOptions.IsSignalRaw())
        {
            if (fep::isFailed(ResolveSignalType(oUserSignalOptions.GetSignalType().c_str(), strSignalDescription)))
            {
                nResult = ERR_INVALID_TYPE;   
            }
        }

        tSignal* pSignal = NULL;

        if (fep::isOk(nResult)) 
        {
            // register signal in central registry
            nResult = InternalRegisterSignal(oUserSignalOptions, strSignalDescription);

            pSignal = FindSignal(oUserSignalOptions.GetSignalName().c_str(), oUserSignalOptions.GetSignalDirection());
            if (!pSignal)
            {
                nResult = ERR_UNEXPECTED;
            }
        }

        if (fep::isOk(nResult))
        {
            nResult = RegisterAtDataAccess(*pSignal, hSignalHandle);
        }

        if (fep::isOk(nResult))
        {
            // creating information properties in the property tree
            // signal type
            std::string strSetProp;
            if (fep::SD_Output == oUserSignalOptions.GetSignalDirection())
            {
                strSetProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredOutSignals,
                    oUserSignalOptions.GetSignalName().c_str(), g_strSignalRegistryField_SignalType);
                m_pPropertyTree->SetPropertyValue(strSetProp.c_str(), pSignal->strSignalType.c_str());
                // sample size
                strSetProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredOutSignals,
                    oUserSignalOptions.GetSignalName().c_str(), g_strSignalRegistryField_SignalSize);
                int32_t nSetHelper = static_cast<int32_t>(pSignal->szSampleSize);
                m_pPropertyTree->SetPropertyValue(strSetProp.c_str(), nSetHelper);
                // mapping info
                strSetProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredOutSignals,
                    oUserSignalOptions.GetSignalName().c_str(), g_strSignalRegistryField_MappedSignal);
                m_pPropertyTree->SetPropertyValue(strSetProp.c_str(), pSignal->bIsMapped);
                // muting info
                strSetProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredOutSignals,
                    oUserSignalOptions.GetSignalName().c_str(), g_strSignalRegistryField_MutedSignal);
                m_pPropertyTree->SetPropertyValue(strSetProp.c_str(), s_bDefaultMuteStatus);
            }
            else if (fep::SD_Input == oUserSignalOptions.GetSignalDirection())
            {
                strSetProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredInSignals,
                    oUserSignalOptions.GetSignalName().c_str(), g_strSignalRegistryField_SignalType);
                m_pPropertyTree->SetPropertyValue(strSetProp.c_str(), pSignal->strSignalType.c_str());
                // sample size
                strSetProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredInSignals,
                    oUserSignalOptions.GetSignalName().c_str(), g_strSignalRegistryField_SignalSize);
                int32_t nSetHelper = static_cast<int32_t>(pSignal->szSampleSize);
                m_pPropertyTree->SetPropertyValue(strSetProp.c_str(), nSetHelper);
                // mapping info
                strSetProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredInSignals,
                    oUserSignalOptions.GetSignalName().c_str(), g_strSignalRegistryField_MappedSignal);
                m_pPropertyTree->SetPropertyValue(strSetProp.c_str(), pSignal->bIsMapped);
                // sample backlog size
                strSetProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredInSignals,
                    oUserSignalOptions.GetSignalName().c_str(), g_strSignalRegistryField_SampleBacklogLength);
                nSetHelper = static_cast<int32_t>(pSignal->szSampleBacklog);
                m_pPropertyTree->SetPropertyValue(strSetProp.c_str(), nSetHelper);
            }
        }

    }
    else
    {
        nResult = ERR_INVALID_STATE;
    }
    return nResult;
}

fep::Result cSignalRegistry::InternalRegisterSignal(const cUserSignalOptions & oUserSignalOptions,
    const char * strSignalDescription)
{
    fep::Result nResult = ERR_NOERROR;

        tSignal * pSignal = FindSignal(oUserSignalOptions.GetSignalName().c_str(),
            oUserSignalOptions.GetSignalDirection());

        if (pSignal)
        {
            nResult = ERR_RESOURCE_IN_USE;
        }

        if (fep::isOk(nResult))
        {
            tSignal sSig;
            sSig.strSignalName = oUserSignalOptions.GetSignalName();
            
            sSig.strSignalType = oUserSignalOptions.GetSignalType();

            if(NULL != strSignalDescription)
            {
                sSig.strSignalDesc = strSignalDescription;
            }
            else
            {
                sSig.strSignalDesc = "";
            }

            sSig.eDirection = oUserSignalOptions.GetSignalDirection();
            sSig.bIsMapped = false;
            sSig.bIsRaw = oUserSignalOptions._d->m_bIsRawSignal;
            sSig.bIsReliable = oUserSignalOptions._d->m_bReliability;
            sSig.szSampleBacklog = 1;
            if(!oUserSignalOptions.IsSignalRaw())
            {
                nResult = fep::helpers::
                    CalculateSignalSizeFromDescription(oUserSignalOptions.GetSignalType().c_str(), 
                        strSignalDescription, sSig.szSampleSize);
            }
            else
            {
                sSig.szSampleSize = 0;
            }

            sSig.eSerialization = (m_bEnabledSerialization && !oUserSignalOptions.IsSignalRaw()) ? 
                fep::SER_Ddl : fep::SER_Raw;

            //RTI specific settings
            sSig.bRTIAsyncPub = oUserSignalOptions._d->m_bUseAsyncPubliser;
            sSig.bRTILowLat = oUserSignalOptions._d->m_bUseLowLatProfile;
            sSig.strRTIMulticast.SetDefaultValue("");

            if (fep::isOk(nResult))
            {
                m_lstSignals.push_back(sSig);
            }
        }
    return nResult;
}

fep::Result cSignalRegistry::RegisterAtDataAccess(tSignal& oSignal, handle_t& hSignalHandle)
{
    fep::Result nResult = ERR_NOERROR;
    // forward to data access
    nResult = m_poDataAccess->RegisterSignal(oSignal, hSignalHandle);

    if (fep::isFailed(nResult))
    {
        // registration did not go through cleanly, unregister the signal from the registry
        InternalUnregisterSignal(oSignal);
    }
    else
    {
        AssociateHandle(oSignal, hSignalHandle);
        m_poDataAccess->SignalRegistered(hSignalHandle);
    }

    return nResult;
}

fep::Result cSignalRegistry::AssociateHandle(tSignal& oSignal, const handle_t hHandle)
{
    fep::Result nResult = ERR_NOERROR;

    if (!m_mapHandles.insert(std::make_pair(hHandle, &oSignal)).second)
    {
        nResult = ERR_RESOURCE_IN_USE;
    }

    return nResult;
}

fep::Result cSignalRegistry::InternalUnregisterSignal(const tSignal& oSignal)
{
    // erase all handles associated with the same signal
    for (tHandleMap::iterator itHandles = m_mapHandles.begin();
        itHandles != m_mapHandles.end();)
    {
        // see http://stackoverflow.com/a/4600610
        // post-inc operator returns the old iterator, common idiom before C++11
        if (itHandles->second == &oSignal)
        {
            m_mapHandles.erase(itHandles++);
        }
        else
        {
            ++itHandles;
        }
    }

    // erase actual signal
    for (tSignalList::iterator itSignals = m_lstSignals.begin();
        itSignals != m_lstSignals.end();
        ++itSignals)
    {
        // comparing pointers is enough
        if (&(*itSignals) == &oSignal)
        {
            // no need to get the next iterator before erasing the current one
            // because we'll leave the loop directory afterwards anyway
            m_lstSignals.erase(itSignals);
            break;
        }
    }
    return ERR_NOERROR;
}


fep::Result cSignalRegistry::UnregisterSignal(handle_t hSignalHandle)
{
    fep::Result  nResult = ERR_NOT_FOUND;
    if(m_bAllowRegistration)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oRegistrationMutex);
        const tSignal* pSignal = FindSignal(hSignalHandle);

        if (pSignal)
        {
            if (pSignal->eDirection == fep::SD_Undefined)
            {
                nResult = ERR_UNEXPECTED;
            }
            else
            {
                nResult = m_poDataAccess->UnregisterSignal(*pSignal, hSignalHandle);
            }
        }

        if (fep::isOk(nResult))
        {
            // creating information properties in the property tree
            // signal type
            std::string strDeleteProp;
            if (fep::SD_Output == pSignal->eDirection)
            {
                strDeleteProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredOutSignals, pSignal->strSignalName.c_str(), g_strSignalRegistryField_SignalType);
                m_pPropertyTree->DeleteProperty(strDeleteProp.c_str());
                // sample size
                strDeleteProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredOutSignals, pSignal->strSignalName.c_str(), g_strSignalRegistryField_SignalSize);
                m_pPropertyTree->DeleteProperty(strDeleteProp.c_str());
                // mapping info
                strDeleteProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredOutSignals, pSignal->strSignalName.c_str(), g_strSignalRegistryField_MappedSignal);
                m_pPropertyTree->DeleteProperty(strDeleteProp.c_str());
                // muting info
                strDeleteProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredOutSignals, pSignal->strSignalName.c_str(), g_strSignalRegistryField_MutedSignal);
                m_pPropertyTree->DeleteProperty(strDeleteProp.c_str());

            }
            else if (fep::SD_Input == pSignal->eDirection)
            {
                strDeleteProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredInSignals, pSignal->strSignalName.c_str(), g_strSignalRegistryField_SignalType);
                m_pPropertyTree->DeleteProperty(strDeleteProp.c_str());
                // sample size
                strDeleteProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredInSignals, pSignal->strSignalName.c_str(), g_strSignalRegistryField_SignalSize);
                m_pPropertyTree->DeleteProperty(strDeleteProp.c_str());
                // mapping info
                strDeleteProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredInSignals, pSignal->strSignalName.c_str(), g_strSignalRegistryField_MappedSignal);
                m_pPropertyTree->DeleteProperty(strDeleteProp.c_str());
                // sample backlog size
                strDeleteProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredInSignals, pSignal->strSignalName.c_str(), g_strSignalRegistryField_SampleBacklogLength);
                m_pPropertyTree->DeleteProperty(strDeleteProp.c_str());
            }
            nResult = InternalUnregisterSignal(*pSignal);
        }

    }
    else
    {
        nResult = ERR_INVALID_STATE;
    }
    return nResult;
}

fep::Result cSignalRegistry::GetSignalSampleSize(const handle_t hHandle, size_t & szSize) const
{
    fep::Result nResult = ERR_NOT_FOUND;

    tHandleMap::const_iterator itHandles = m_mapHandles.find(hHandle);
    if (itHandles != m_mapHandles.end())
    {
        nResult = ERR_NOERROR;
        szSize = itHandles->second->szSampleSize;
    }

    return nResult;
}

fep::Result cSignalRegistry::GetSignalSampleSize(const char * strSignalName,
    const tSignalDirection eDirection, size_t & szSize) const
{
    fep::Result nResult = ERR_NOERROR;
    if(fep::SD_Undefined == eDirection)
    {
        nResult = ERR_INVALID_ARG;
    }
    else
    {
        const tSignal * pSignal = FindSignal(strSignalName, eDirection);
        if (!pSignal)
        {
            nResult = ERR_NOT_FOUND;
        }

        if (fep::isOk(nResult))
        {
            szSize = pSignal->szSampleSize;
        }
    }
    return nResult;
}

tSignal * cSignalRegistry::FindSignal(const char * strSignalName,
    const tSignalDirection eDirection)
{
    return const_cast<tSignal *>(
        static_cast<const cSignalRegistry*>(this)
        ->FindSignal(strSignalName, eDirection));
}

const tSignal * cSignalRegistry::FindSignal(const char * strSignalName,
    const tSignalDirection eDirection) const
{
    const tSignal * pSignal = NULL;
    for (tSignalList::const_iterator itSignals = m_lstSignals.begin();
        itSignals != m_lstSignals.end(); ++itSignals)
    {
        if (itSignals->strSignalName == strSignalName && itSignals->eDirection == eDirection)
        {
            pSignal = &(*itSignals);
            break;
        }
    }

    return pSignal;
}

const tSignal* cSignalRegistry::FindSignal(handle_t hSignalhandle) const
{
    const tSignal * pSignal = NULL;
    tHandleMap::const_iterator itHandles = m_mapHandles.find(hSignalhandle);
    if (itHandles != m_mapHandles.end())
    {
        pSignal = itHandles->second;
    }

    return pSignal;
}

fep::Result cSignalRegistry::GetSignalNameFromHandle(handle_t const hSignal, char const *& strSignal) const
{
    tHandleMap::const_iterator itSignal = m_mapHandles.find(hSignal);
    if (m_mapHandles.end() != itSignal)
    {
        strSignal = itSignal->second->strSignalName.c_str();
        return ERR_NOERROR;
    } 
    else
    {
        return ERR_NOT_FOUND;
    }
}

fep::Result cSignalRegistry::GetSignalTypeFromHandle(const handle_t hSignal, const char *&strSignalType) const
{
    tHandleMap::const_iterator itSignal = m_mapHandles.find(hSignal);
    if(m_mapHandles.end() != itSignal)
    {
        strSignalType = itSignal->second->strSignalType.c_str();
        return ERR_NOERROR;
    }
    else
    {
        return ERR_NOT_FOUND;
    }
}

fep::Result cSignalRegistry::GetSignalTypeFromName(const char *strSignalName, const tSignalDirection eDirection, const char *&strSignalType) const
{
    fep::Result nResult = ERR_NOT_FOUND;
    if(fep::SD_Undefined == eDirection || strSignalName == nullptr)
    {
        nResult = ERR_INVALID_ARG;
    }
    else
    {
        for (tSignalList::const_iterator itSignals = m_lstSignals.begin();
            itSignals != m_lstSignals.end(); ++itSignals)
        {
            if (itSignals->strSignalName == strSignalName && itSignals->eDirection == eDirection)
            {
                strSignalType = itSignals->strSignalType.c_str();
                nResult = ERR_NOERROR;
                break;
            }
        }
    }
    return nResult;
}

fep::Result cSignalRegistry::GetSignalHandleFromName(const char* strSignalName, tSignalDirection eDirection,
    handle_t &hSignalHandle) const
{
    fep::Result nResult = ERR_NOT_FOUND;
    if (fep::SD_Undefined == eDirection)
    {
        nResult = ERR_INVALID_ARG;
    }
    else
    {
        const tSignal* pSignal = FindSignal(strSignalName, eDirection);
        if (pSignal)
        {
            for (tHandleMap::const_iterator itr = m_mapHandles.begin();
                itr != m_mapHandles.end();)
            {
                if((*itr).second == pSignal)
                {
                    nResult = ERR_NOERROR;
                    hSignalHandle = (*itr).first;
                    break;
                }
                else
                {
                    ++itr;
                }
            }
        }
    }
    return nResult;
}

fep::Result cSignalRegistry::ConfigureRemoteDescription()
{
    fep::Result nResult = ERR_NOERROR;
    const char* strRemoteDescriptionPath;
    nResult = m_pPropertyTree->GetPropertyValue(g_strDescriptionPath_strRemoteDescription, strRemoteDescriptionPath);
    if (fep::isOk(nResult) && NULL != strRemoteDescriptionPath)
    {
        m_strRemoteDescriptionPath = strRemoteDescriptionPath;
    }

    if (!m_strRemoteDescriptionPath.empty() && m_strRemoteDescriptionPath != "")
    {
        if (fep::isFailed(RegisterRemoteDescriptionConfig()))
        {
            INVOKE_INCIDENT(m_poModule->GetIncidentHandler(),
                fep::FSI_MAPPING_REMOTE_PROP_CONFIG_FAILED, fep::SL_Critical_Global,
                a_util::strings::format(
                    "Failed to register remotely set description file: %s",
                    m_strRemoteDescriptionPath.c_str()).c_str());
        }
    }
    // giving back result codes to entry listeners does nothing
    return ERR_NOERROR;
}

fep::Result cSignalRegistry::RegisterRemoteDescriptionConfig()
{
    fep::Result nResult = ERR_NOERROR;
    std::vector<std::string> vecStrings;
    vecStrings = a_util::strings::splitToken(m_strRemoteDescriptionPath, ";,");
    for (auto string : vecStrings)
    {
        string = a_util::strings::trim(string);
        if (fep::isFailed(nResult = RegisterSignalDescription(string.c_str(),
            DF_MERGE | DF_DESCRIPTION_FILE)))
        {
            //rollback
            ClearSignalDescriptions();
            break;
        }
    }
    return nResult;
}

fep::Result cSignalRegistry::RegisterSignalDescription(const char* strDescription, uint32_t ui32DescriptionFlags)
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oRegistrationMutex);
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync2(m_oDescriptionMutex);
    bool bReplace = (ui32DescriptionFlags & DF_REPLACE) == DF_REPLACE;
    bool bMerge = (ui32DescriptionFlags & DF_MERGE) == DF_MERGE;

    // both replace and merge are not allowed
    if (!strDescription || (bReplace && bMerge))
    {
        return ERR_INVALID_ARG;
    }

    // replace is default behaviour
    if (!bReplace && !bMerge)
    {
        bReplace = true;
    }

    std::string desc;
    if ((ui32DescriptionFlags & DF_DESCRIPTION_FILE) == DF_DESCRIPTION_FILE)
    {
        if (a_util::filesystem::readTextFile(strDescription, desc) != a_util::filesystem::OK)
        {
            return ERR_INVALID_FILE;
        }
    }
    else
    {
        desc = strDescription;
    }

    fep::Result nRes = ERR_NOERROR;
    if (bReplace)
    {
        nRes = m_oDescriptionMan.LoadDDL(desc);
    }
    else if (bMerge)
    {
        nRes = m_oDescriptionMan.MergeDDL(desc);
    }

    if (fep::isFailed(nRes))
    {
        const std::string& strErr = m_oDescriptionMan.GetErrorDesc();
        INVOKE_INCIDENT(m_poModule->GetIncidentHandler(),FSI_SIGNAL_DESCRIPTION_INVALID, SL_Critical_Local, strErr.c_str());
    }

    // notify mapping
    if (fep::isOk(nRes) && m_poMappingComponent)
    {
        nRes = m_poMappingComponent->ResetSignalDescription(m_oDescriptionMan.GetDDL());
    }

    return nRes;
}

fep::Result cSignalRegistry::ClearSignalDescriptions()
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oRegistrationMutex);
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync2(m_oDescriptionMutex);
    fep::Result nRes = m_oDescriptionMan.ClearDDL();
    m_oDescriptionMap.clear();

    // notify mapping
    if (fep::isOk(nRes) && m_poMappingComponent)
    {
        nRes = m_poMappingComponent->ResetSignalDescription(m_oDescriptionMan.GetDDL());
    }

    return nRes;
}

fep::Result cSignalRegistry::ResolveSignalType(const char* strSignalType, const char*& strDescription)
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oDescriptionMutex);
    if (!strSignalType) { return ERR_POINTER; }

    std::string strType(strSignalType);
    tDescriptionMap::const_iterator it = m_oDescriptionMap.find(strType);
    if (it == m_oDescriptionMap.end())
    {
        std::string desc;
        if (fep::isOk(m_oDescriptionMan.ResolveType(strType, desc)))
        {
            std::pair<tDescriptionMap::iterator, bool> oRes =
                m_oDescriptionMap.insert(std::make_pair(strType, desc));
            strDescription = oRes.first->second.c_str();
        }
        else
        {
            return ERR_NOT_FOUND;
        }
    }
    else
    {
        strDescription = it->second.c_str();
    }

    return ERR_NOERROR;
}

fep::Result cSignalRegistry::SetSignalSampleBacklog(handle_t hSignal, size_t szSampleBacklog)
{
    tSignal* pSignal = const_cast<tSignal*>(FindSignal(hSignal));
    if (!pSignal)
    {
        return ERR_INVALID_ARG;
    }

    if (pSignal->eDirection != SD_Input)
    {
        return ERR_INVALID_TYPE;
    }

    pSignal->szSampleBacklog = szSampleBacklog;
    std::string strBacklogProp = a_util::strings::format("%s.%s.%s", g_strSignalRegistryPath_RegisteredInSignals, 
        pSignal->strSignalName.c_str(), g_strSignalRegistryField_SampleBacklogLength);
    m_pPropertyTree->SetPropertyValue(strBacklogProp.c_str(), static_cast<int32_t>(szSampleBacklog));
    return m_poDataAccess->SignalBacklogChanged(hSignal, szSampleBacklog);
}

fep::Result cSignalRegistry::GetSignalSampleBacklog(handle_t hSignal, size_t& szSampleBacklog) const
{
    const tSignal* pSignal = FindSignal(hSignal);
    if (!pSignal)
    {
        return ERR_INVALID_ARG;
    }

    if (pSignal->eDirection != SD_Input)
    {
        return ERR_INVALID_TYPE;
    }

    szSampleBacklog = pSignal->szSampleBacklog;
    return ERR_NOERROR;
}

const ddl::DDLDescription& cSignalRegistry::GetSignalDescription() const
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oDescriptionMutex);
    return m_oDescriptionMan.GetDDL();
}

fep::Result cSignalRegistry::GetSignalNamesAndTypes(fep::IStringList *& poRxSignals, fep::IStringList *& poTxSignals) const
{
    std::unique_ptr<fep::cStringList> poRxList(new fep::cStringList);
    std::unique_ptr<fep::cStringList> poTxList(new fep::cStringList);

    for (tSignalList::const_iterator itSignal = m_lstSignals.begin();
        itSignal != m_lstSignals.end(); ++itSignal)
    {
        if (SD_Input == itSignal->eDirection)
        {
            poRxList->push_back(itSignal->strSignalName);
            poRxList->push_back(itSignal->strSignalType);
        }
        else if (SD_Output == itSignal->eDirection)
        {
            poTxList->push_back(itSignal->strSignalName);
            poTxList->push_back(itSignal->strSignalType);
        }
        else
        {
            /* MISRA 14.10 */ /* no other direction so far (e.g. SD_InOut) */
            assert(false);
        }
    }

    poRxSignals = poRxList.release();
    poTxSignals = poTxList.release();

    return ERR_NOERROR;
}

fep::Result cSignalRegistry::Update(IGetSignalInfoCommand const *poCommand)
{
    if (!m_poModule) { return ERR_POINTER; }

    fep::Result nResult = ERR_NOERROR;
    fep::IStringList * poRxList = NULL;
    fep::IStringList * poTxList = NULL;
    nResult = GetSignalNamesAndTypes(poRxList, poTxList);
    if(fep::isOk(nResult))
    {
        cSignalInfoNotification oNoti(poRxList, poTxList,
            m_poModule->GetName(), poCommand->GetSender(), GetTimeStampMicrosecondsUTC(),
            m_poModule->GetTimingInterface()->GetTime());
        m_poModule->GetNotificationAccess()->TransmitNotification(&oNoti);
    }
    if(NULL != poRxList)
    {
        delete poRxList; poRxList = NULL;
    }
    if(NULL != poTxList)
    {
        delete poTxList; poTxList = NULL;
    }
    return ERR_NOERROR;
}

fep::Result cSignalRegistry::Update(IResolveSignalTypeCommand const *poCommand)
{
    if (!m_poModule) { return ERR_POINTER; }
    fep::Result nResult = ERR_NOERROR;
    const char* strSignalDescription = NULL;
    nResult = ResolveSignalType(poCommand->GetSignalType(),strSignalDescription);
    if (fep::isOk(nResult))
    {
        cSignalDescriptionNotification oNoti(strSignalDescription,
            m_poModule->GetName(), poCommand->GetSender(), GetTimeStampMicrosecondsUTC(),
            m_poModule->GetTimingInterface()->GetTime());
        m_poModule->GetNotificationAccess()->TransmitNotification(&oNoti);
    }
    else
    {
        cSignalDescriptionNotification oNoti("",
            m_poModule->GetName(), poCommand->GetSender(), GetTimeStampMicrosecondsUTC(),
            m_poModule->GetTimingInterface()->GetTime());
        m_poModule->GetNotificationAccess()->TransmitNotification(&oNoti);
    }
    return ERR_NOERROR;
}

fep::Result cSignalRegistry::Update(fep::ISignalDescriptionCommand const *poCommand)
{
    fep::Result nResult = ERR_NOERROR;
    if (poCommand->GetCommandType() == ISignalDescriptionCommand::CT_REGISTER_DESCRIPTION)
    {
        nResult = RegisterSignalDescription(poCommand->GetDescriptionString(),
            poCommand->GetDescriptionFlags());
    }
    else if (poCommand->GetCommandType() == ISignalDescriptionCommand::CT_CLEAR_DESCRIPTIONS)
    {
        nResult = ClearSignalDescriptions();
    }

    // transmit result code back
    cResultCodeNotification oNot(poCommand->GetCommandCookie(), nResult,
        m_poModule->GetName(), poCommand->GetSender(), GetTimeStampMicrosecondsUTC(),
        m_poModule->GetTimingInterface()->GetTime());

    m_poModule->GetNotificationAccess()->TransmitNotification(&oNot);

    return ERR_NOERROR;
}

bool fep::cSignalRegistry::IsMappedSignal(const handle_t hHandle) const
{
    const tSignal* pSignal = FindSignal(hHandle);
    return (NULL == pSignal) ? false : pSignal->bIsMapped;
}

bool fep::cSignalRegistry::AnyMappedSignals()
{
    bool bMappedSignals = false;
    for (tSignalList::iterator it = m_lstSignals.begin(); it != m_lstSignals.end(); ++it)
    {
        if ((*it).bIsMapped)
        {
            bMappedSignals = true;
            break;
        }
    }
    return bMappedSignals;
}

fep::Result cSignalRegistry::AllowSignalRegistration()
{
    m_bAllowRegistration = true;
    return ERR_NOERROR;
}

fep::Result cSignalRegistry::DisallowSignalRegistration()
{
    m_bAllowRegistration = false;
    return ERR_NOERROR;
}

fep::Result cSignalRegistry::CreateDefaultProperties()
{
    fep::Result nResult = ERR_NOERROR;

    nResult = m_pPropertyTree->SetPropertyValue(g_strSignalRegistryPath_bSerialization, true);
    nResult |= m_pPropertyTree->SetPropertyValue(s_strLimiterPath_bGlobalOn, false);

    return nResult;
}

fep::Result cSignalRegistry::ProcessPropertyChange(const IProperty *poProperty,
    const IProperty *poAffectedProperty,
    const char *strRelativePath)
{
    fep::Result nResult = ERR_NOERROR;

    if (a_util::strings::isEqual(g_strSignalRegistryPath_bSerialization, poAffectedProperty->GetPath()))
    {
        bool bNewValue;
        m_pPropertyTree->GetPropertyValue(g_strSignalRegistryPath_bSerialization, bNewValue);
        if (bNewValue != m_bEnabledSerialization)
        {
            a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oRegistrationMutex);
            if (!m_lstSignals.empty())
            {
                INVOKE_INCIDENT(m_poModule->GetIncidentHandler(),FSI_SERIALIZATION_CHANGE_WITH_REGISTERED_SIGNALS,
                    fep::SL_Warning,
                    "The serialization flag was changed while signals "
                    "were already registered!");
            }
            m_bEnabledSerialization = bNewValue;
        }
    }
    else if (a_util::strings::isEqual(s_strLimiterPath_bGlobalOn, poAffectedProperty->GetPath()))
    {
        INVOKE_INCIDENT(m_poModule->GetIncidentHandler(), FSI_GENERAL_INFORMATION, fep::SL_Info,
            "The limiter functionality was removed from the FEP SDK. Please contact the AEV Support (aev.support@audi.de) and briefly describe "
            "your use case for the limiter.");
    }
    else if (a_util::strings::isEqual(strRelativePath, g_strDescriptionField_strRemoteDescription))
    {
        const char* strNew = NULL;
        std::string strNewVal;
        if (fep::isOk(poAffectedProperty->GetValue(strNew)))
        {
            if (NULL != strNew)
            {
                strNewVal = strNew;
            }
            else
            {
                strNewVal = "";
            }
        }
        if (strNewVal == "")
        {
            ClearSignalDescriptions();
        }
        else
        {
            ConfigureRemoteDescription();
        }
    }

    return nResult;
}

fep::Result cSignalRegistry::ProcessPropertyAdd(const IProperty *poProperty, 
    const IProperty *poAffectedProperty, const char *strRelativePath)
{
    // nothing to do here
    return ERR_NOERROR;
}

fep::Result cSignalRegistry::ProcessPropertyDelete(const IProperty *poProperty, 
    const IProperty *poAffectedProperty, const char *strRelativePath)
{
    // nothing to do here
    return ERR_NOERROR;
}
