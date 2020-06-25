/**

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

#include "mapping/fep_mapping.h"
#include <mapping/configuration/map_target.h>
#include <algorithm>
#include <memory>
#include <string>
#include <cinttypes>    // PRId64
#include <utility>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>
#include <a_util/strings/strings_functions.h>
#include <a_util/xml/dom.h>

#include "_common/fep_timestamp.h"
#include "data_access/fep_user_data_access_intf.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep_errors.h"
#include "fep_mapping_common.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "messages/fep_command_access_intf.h"
#include "messages/fep_command_mapping_configuration_intf.h"
#include "messages/fep_notification_access_intf.h"
#include "messages/fep_notification_resultcode.h"
#include "module/fep_module_intf.h"
#include "module/fep_module_private_intf.h"
#include "signal_registry/fep_signal_registry.h"
#include "signal_registry/fep_signal_struct.h"
#include "signal_registry/fep_user_signal_options.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include "transmission_adapter/fep_signal_direction.h"
#include "transmission_adapter/fep_user_data_sample_intf.h"

using namespace fep;
using namespace fep::component_config;
using namespace mapping;

#ifdef _MSC_VER
// Please always insert some reason why this pragma is used
#pragma warning(disable: 4355)
#endif

cSignalMapping::cSignalMapping() :
    m_pModule(NULL), m_pTiming(NULL), m_pSignalRegistry(NULL),
    m_pUserDataAccess(NULL), m_pPropertyTree(NULL), m_oEngine(*this)
{
}

fep::Result cSignalMapping::SetModule(IModulePrivate* pPrivateModule)
{
    fep::Result nResult = ERR_NOERROR;
    IModule* pPublicModule = pPrivateModule ? pPrivateModule->GetModule() : NULL;
    if (m_pModule != pPublicModule)
    {
        if (NULL != m_pModule)
        {
            if(NULL != m_pPropertyTree)
            {
                m_pPropertyTree->UnregisterListener(g_strMappingPath_strRemoteMapping,this);
            }
            m_pModule->GetCommandAccess()->UnregisterCommandListener(this);

            m_mapDataListener.clear();
            m_oEngine.stop();
            m_oEngine.unmapAll();
            ClearMappingConfiguration();

            m_pTiming = NULL;
            m_pUserDataAccess = NULL;
            m_pSignalRegistry = NULL;
            m_pPropertyTree = NULL;
            m_strRemoteMappingPath.clear();
        }

        m_pModule = pPublicModule;

        if (NULL != m_pModule)
        {
            m_pTiming = m_pModule->GetTimingInterface();
            m_pUserDataAccess = m_pModule->GetUserDataAccess();
            m_pSignalRegistry = pPrivateModule->GetSignalRegistry();
            m_pPropertyTree = m_pModule->GetPropertyTree();
            m_oConfig.setDescription(&m_pSignalRegistry->GetSignalDescription());
            m_oEngine.setConfiguration(m_oConfig);

            m_pModule->GetCommandAccess()->RegisterCommandListener(this);
            if(NULL != m_pPropertyTree)
            {
                m_pPropertyTree->SetPropertyValue(g_strMappingPath_strRemoteMapping, "");
                m_pPropertyTree->RegisterListener(g_strMappingPath_strRemoteMapping, this);
            }
        }
    }

    if (fep::isFailed(nResult))
    {
        return ERR_FAILED;
    }
    return ERR_NOERROR;
}

fep::Result cSignalMapping::ResetSignalDescription(const ddl::DDLDescription& oDDL)
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oConfigMutex);
    m_oConfig.setDescriptionWithoutConsistency(&oDDL);
    m_oEngine.setConfiguration(m_oConfig);
    return ERR_NOERROR;
}

fep::Result cSignalMapping::RegisterSignal(const tSignal& oSignal, handle_t& hHandle)
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oConfigMutex);
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync2(m_oListenerMutex);
    fep::Result nResult = ERR_NOERROR;
    if(m_oConfig.getTarget(oSignal.strSignalName.c_str()))
    {
        if (!m_oConfig.isConsistencyChecked())
        {
            m_oConfig.checkDDLConsistency();
            m_oEngine.setConfiguration(m_oConfig);
        }
        if (!m_oConfig.isConsistent())
        {
            const mapping::oo::MapErrorList& lstErrors = m_oConfig.getErrorList();
            std::string strErr;
            for (size_t idx = 0; idx < lstErrors.size(); ++idx)
            {
                if (idx > 0)
                {
                    strErr.append("\n");
                }
                strErr.append(lstErrors[idx].c_str());
            }
            INVOKE_INCIDENT(m_pModule->GetIncidentHandler(), FSI_MAPPING_CONFIG_DDL_INCONSISTENCY,
                            SL_Critical_Local, strErr.c_str());
            nResult = ERR_UNKNOWN_FORMAT;
        }
    }
    if (fep::isOk(nResult))
    {
        nResult = m_oEngine.Map(oSignal.strSignalName.c_str(), hHandle).getErrorCode();
    }
    if (nResult == ERR_INVALID_ARG)
    {
        nResult = ERR_NOT_FOUND;
    }
    else if (fep::isOk(nResult))
    {
        m_mapDataListener.insert(std::make_pair(hHandle, std::vector<fep::IUserDataListener*>()));
    }

    return nResult;
}

fep::Result cSignalMapping::UnregisterSignal(handle_t hHandle)
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oListenerMutex);
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync2(m_oConfigMutex);
    fep::Result nRes = m_oEngine.unmap(hHandle).getErrorCode();
    if (fep::isOk(nRes))
    {
        m_mapDataListener.erase(hHandle);
    }

    return nRes;
}

fep::Result cSignalMapping::CopyBuffer(handle_t hSignalHandle, void* pDestination, size_t szBuffer) const
{
    return m_oEngine.getCurrentData(hSignalHandle, pDestination, szBuffer).getErrorCode();
}

fep::Result cSignalMapping::RegisterDataListener(IUserDataListener* poDataListener,
    handle_t hSignalHandle)
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oListenerMutex);
    if (!poDataListener) { return ERR_POINTER; }

    fep::Result nRes = ERR_INVALID_ARG;
    tDataListenerMap::iterator itMap = m_mapDataListener.find(hSignalHandle);
    if (itMap != m_mapDataListener.end())
    {
        tListenerList::const_iterator itLst =
            std::find(itMap->second.begin(), itMap->second.end(), poDataListener);
        if (itLst != itMap->second.end())
        {
            nRes = ERR_RESOURCE_IN_USE;
        }
        else
        {
            itMap->second.push_back(poDataListener);
            nRes = ERR_NOERROR;
        }
    }
    
    return nRes;
}

fep::Result cSignalMapping::UnregisterDataListener(IUserDataListener* poDataListener,
    const handle_t hSignalHandle)
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oListenerMutex);
    if (!poDataListener) { return ERR_POINTER; }

    fep::Result nRes = ERR_INVALID_ARG;
    tDataListenerMap::iterator itMap = m_mapDataListener.find(hSignalHandle);
    if (itMap != m_mapDataListener.end())
    {
        tListenerList::iterator itLst =
            std::find(itMap->second.begin(), itMap->second.end(), poDataListener);
        if (itLst == itMap->second.end())
        {
            nRes = ERR_NOT_FOUND;
        }
        else
        {
            itMap->second.erase(itLst);
            nRes = ERR_NOERROR;
        }
    }

    return nRes;
}

fep::Result cSignalMapping::RegisterMappingConfiguration(const char* strConfig,
    uint32_t ui32MappingFlags)
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oConfigMutex);
    bool bReplace = (ui32MappingFlags & MF_REPLACE) == MF_REPLACE;
    bool bMerge = (ui32MappingFlags & MF_MERGE) == MF_MERGE;
    bool bFile = (ui32MappingFlags & MF_MAPPING_FILE) == MF_MAPPING_FILE;
    fep::Result nRes = ERR_NOERROR;

    if (!strConfig || a_util::strings::getLength(strConfig) == 0)
    {
        nRes = ERR_INVALID_ARG;
    }

    // both replace and merge is not allowed
    if (fep::isOk(nRes) && ((bReplace && bMerge)))
    {
        nRes = ERR_NOT_SUPPORTED;
    }

    // replace is default
    if (!bReplace && !bMerge)
    {
        bReplace = true;
    }

    // load into dom
    a_util::xml::DOM oDom;
    if (fep::isOk(nRes))
    {
        if (bFile)
        {
            if (!oDom.load(strConfig))
            {
                nRes = ERR_INVALID_FILE;
            }
        }
        else
        {
            if (!oDom.fromString(strConfig))
            {
                nRes = ERR_FAILED;
            }
        }
    }

    // load config
    if (fep::isOk(nRes))
    {
        oo::MapConfiguration::MapConfigFlags eFlags = bMerge ?
            oo::MapConfiguration::mc_merge_mapping : oo::MapConfiguration::mc_load_mapping;
        nRes = m_oConfig.loadFromDOMWithoutDDLConsistency(oDom, eFlags).getErrorCode();

        if (fep::isFailed(nRes))
        {
            const mapping::oo::MapErrorList& lstErrors = m_oConfig.getErrorList();
            std::string strErr;
            for (size_t idx = 0; idx < lstErrors.size(); ++idx)
            {
                if (idx > 0)
                {
                    strErr.append("\n");
                }
                strErr.append(lstErrors[idx].c_str());
            }
            INVOKE_INCIDENT(m_pModule->GetIncidentHandler(), FSI_MAPPING_CONFIG_INVALID,
                            SL_Critical_Local, strErr.c_str());
        }
    }

    // notify engine
    if (fep::isOk(nRes))
    {
        m_oEngine.setConfiguration(m_oConfig);
    }

    return nRes;
}

fep::Result cSignalMapping::ClearMappingConfiguration()
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oConfigMutex);
    m_oConfig.reset();
    m_oEngine.setConfiguration(m_oConfig);
    return ERR_NOERROR;
}

fep::Result cSignalMapping::registerSource(const char* strSourceName,
    const char* strTypeName, mapping::rt::ISignalListener* pListener, handle_t& hHandle)
{
    cUserSignalOptions oOptions(strSourceName, SD_Input, strTypeName);
    fep::Result nRes = m_pSignalRegistry->RegisterSignal(oOptions, hHandle);
    if (fep::isOk(nRes))
    {
        nRes = m_pUserDataAccess->RegisterDataListener(this, hHandle);
    }

    if (fep::isOk(nRes))
    {
        m_mapListenerTranslation.insert(std::make_pair(hHandle, pListener));
    }

    return fep::Result(nRes);
}

fep::Result cSignalMapping::unregisterSource(handle_t hHandle)
{
    fep::Result nRes = m_pSignalRegistry->UnregisterSignal(hHandle);
    if (fep::isOk(nRes))
    {
        nRes = m_pUserDataAccess->UnregisterDataListener(this, hHandle);
    }

    if (fep::isOk(nRes))
    {
        m_mapListenerTranslation.erase(hHandle);
    }

    return fep::Result(nRes);
}

fep::Result cSignalMapping::sendTarget(handle_t hTarget, const void* pData,
    size_t szSize, timestamp_t tmTimeStamp)
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oListenerMutex);
    IUserDataSample* pSample = m_mapSourceSamples[hTarget];
    pSample->CopyFrom(pData, szSize);
    (void)tmTimeStamp;

    // we use the current simulation time for the sample
    pSample->SetTime(getTime());

    tDataListenerMap::iterator itMap = m_mapDataListener.find(hTarget);
    if (itMap != m_mapDataListener.end())
    {
        const tListenerList& lst = itMap->second;
        for (tListenerList::const_iterator it = lst.begin(); it != lst.end(); ++it)
        {
            (*it)->Update(pSample); // ignore return code
        }
    }

    return ERR_NOERROR;
}

fep::Result cSignalMapping::resolveType(const char* strTypeName,
    const char*& strTypeDescription)
{
    fep::Result nRet = m_pSignalRegistry->ResolveSignalType(strTypeName, strTypeDescription);
    return fep::Result(nRet);
}

timestamp_t cSignalMapping::getTime() const
{
    return m_pModule->GetTimingInterface()->GetTime();
}

fep::Result cSignalMapping::registerPeriodicTimer(timestamp_t tmPeriod_us,
    mapping::rt::IPeriodicListener* pListener)
{
    tPeriodicWrappers::iterator it = m_mapPeriodicWrappers.find(pListener);
    if (it == m_mapPeriodicWrappers.end())
    {
        sPeriodicWrapper oWrap;
        oWrap.pListener = pListener;

        m_mapPeriodicWrappers[pListener] = oWrap;
        return m_pTiming->RegisterStepListener(a_util::strings::format("periodic_mapping_trigger_%" PRId64, tmPeriod_us).c_str(),
            fep::StepConfig(tmPeriod_us), sPeriodicWrapper::ProcessStep, &m_mapPeriodicWrappers[pListener]);
    }
    
    return ERR_NOERROR;
}

fep::Result cSignalMapping::unregisterPeriodicTimer(timestamp_t tmPeriod_us,
    mapping::rt::IPeriodicListener* pListener)
{
    tPeriodicWrappers::iterator it = m_mapPeriodicWrappers.find(pListener);
    if (it != m_mapPeriodicWrappers.end())
    {
        m_pTiming->UnregisterStepListener(a_util::strings::format("periodic_mapping_trigger_%" PRId64, tmPeriod_us).c_str());
        m_mapPeriodicWrappers.erase(it);
    }

    return ERR_NOERROR;
}

fep::Result cSignalMapping::Update(fep::IMappingConfigurationCommand const *poCommand)
{
    fep::Result nResult = ERR_NOERROR;
    if (poCommand->GetCommandType() == IMappingConfigurationCommand::CT_REGISTER_MAPPING)
    {
        nResult = RegisterMappingConfiguration(poCommand->GetConfigurationString(),
            poCommand->GetMappingFlags());
    }
    else if (poCommand->GetCommandType() == IMappingConfigurationCommand::CT_CLEAR_MAPPING)
    {
        nResult = ClearMappingConfiguration();
    }

    // transmit result code back
    cResultCodeNotification oNot(poCommand->GetCommandCookie(), nResult,
        m_pModule->GetName(), poCommand->GetSender(), GetTimeStampMicrosecondsUTC(),
        m_pModule->GetTimingInterface()->GetTime());

    m_pModule->GetNotificationAccess()->TransmitNotification(&oNot);

    return ERR_NOERROR;
}

fep::Result cSignalMapping::ConfigureRemoteMapping()
{
    fep::Result nResult = ERR_NOERROR;
    const char* strRemoteMappingPath;
    nResult = m_pPropertyTree->GetPropertyValue(g_strMappingPath_strRemoteMapping, strRemoteMappingPath);
    if(fep::isOk(nResult) && NULL != strRemoteMappingPath)
    {
        m_strRemoteMappingPath = strRemoteMappingPath;
    }

    if(!m_strRemoteMappingPath.empty() && m_strRemoteMappingPath != "")
    {
        if (m_pSignalRegistry->AnyMappedSignals())
        {
            INVOKE_INCIDENT(m_pModule->GetIncidentHandler(),
                            fep::FSI_MAPPING_REMOTE_PROP_CHANGED, fep::SL_Critical_Global,
                            "Registering remotely set mapping configuration while mapped signals "
                            "already exist - this may cause undefined behavior!");
        }
        if (fep::isFailed(RegisterRemoteMappingConfig()))
        {
            INVOKE_INCIDENT(m_pModule->GetIncidentHandler(),
                            fep::FSI_MAPPING_REMOTE_PROP_CONFIG_FAILED, fep::SL_Critical_Global,
                            a_util::strings::format(
                                     "Failed to register remotely set mapping configuration: %s",
                                     m_strRemoteMappingPath.c_str()).c_str());
        }
    }
    // giving back result codes to entry listeners does nothing
    return ERR_NOERROR;
}

void cSignalMapping::StartMappingEngine()
{
    //This is only used in state entry callback of cModulePrivate
    //there is no point in checking the result code since there is no
    //way to react upon ti
    m_oEngine.start();
}

void cSignalMapping::ResetMappingEngine()
{
    //is called in listener callback no point in
    //returning error code since it is not handled
    //anyway
    if (isFailed(m_oEngine.stop().getErrorCode()))
    {
        return;
    }
   m_oEngine.reset();
}

fep::Result cSignalMapping::Update(const IUserDataSample* poSample)
{
    m_mapListenerTranslation[poSample->GetSignalHandle()]->
        onSampleReceived(poSample->GetPtr(), poSample->GetSize());

    return ERR_NOERROR;
}

fep::Result cSignalMapping::targetMapped(const char* strTargetName,
    const char* strTargetType, handle_t hTarget, size_t szTargetType)
{
    (void)strTargetName;
    (void)strTargetType;

    IUserDataSample* pSample = NULL;
    cDataSampleFactory::CreateSample(&pSample);
    pSample->SetSignalHandle(hTarget);
    pSample->SetSize(szTargetType);
    m_mapSourceSamples[hTarget] = pSample;

    return ERR_NOERROR;
}

fep::Result cSignalMapping::targetUnmapped(const char* strTargetName, handle_t hTarget)
{
    (void)strTargetName;
    tSampleMap::iterator it = m_mapSourceSamples.find(hTarget);
    if (it != m_mapSourceSamples.end())
    {
        delete it->second;
        m_mapSourceSamples.erase(it);
    }

    return ERR_NOERROR;
}

        

fep::Result cSignalMapping::ProcessPropertyAdd(IProperty const *poProperty,
    IProperty const *poAffectedProperty, char const *strRelativePath)
{
    return ERR_NOERROR;
}

fep::Result cSignalMapping::ProcessPropertyChange(IProperty const *poProperty,
    IProperty const *poAffectedProperty, char const *strRelativePath)
{
    std::string strRel = strRelativePath;
    if (strRel == g_strMappingField_strRemoteMapping)
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
            if (m_pSignalRegistry->AnyMappedSignals())
            {
                INVOKE_INCIDENT(m_pModule->GetIncidentHandler(),
                                fep::FSI_MAPPING_REMOTE_PROP_CLEAR, fep::SL_Critical_Global,
                                "Clearing mapping configuration while mapped signals exist - "
                                "this may cause undefined behavior!");
            }
            ClearMappingConfiguration();
        }
    }
    return ERR_NOERROR;
}

fep::Result cSignalMapping::ProcessPropertyDelete(IProperty const *poProperty,
    IProperty const *poAffectedProperty, char const *strRelativePath)
{
    return ERR_NOERROR;
}

bool cSignalMapping::IsSignalMappable(const std::string& strSignalName, const std::string& strSignalType) const
{
    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oConfigMutex);
    const oo::MapTarget* pTarget = m_oConfig.getTarget(strSignalName.c_str());
    return pTarget && pTarget->getType() == strSignalType.c_str();
}

fep::Result cSignalMapping::RegisterRemoteMappingConfig()
{
    fep::Result nResult = ERR_NOERROR;
    std::vector<std::string> vecStrings;
    vecStrings = a_util::strings::splitToken(m_strRemoteMappingPath, ";,");
    for (auto string : vecStrings)
    {
        string = a_util::strings::trim(string);
        if (fep::isFailed(nResult = RegisterMappingConfiguration(string.c_str(),
                                                             MF_MERGE | MF_MAPPING_FILE)))
        {
            //rollback
            ClearMappingConfiguration();
            break;
        }
    }
    return nResult;
}

bool fep::cSignalMapping::HandleHasTriggers(const handle_t hSignalHandle)
{
    return m_oEngine.hasTriggers(hSignalHandle);
}
