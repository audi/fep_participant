/**
 * Implementation of the Class cAI.
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

#define _SCL_SECURE_NO_WARNINGS // disable warning about std::copy

#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <utility>
#include <a_util/regex/regularexpression.h>
#include <a_util/result/result_info_decl.h>
#include <a_util/strings/strings_format.h>
#include <a_util/strings/strings_functions.h>
#include <a_util/system/system.h>

#include "_common/fep_timestamp.h"
#include "automation_interface/fep_ai_notification_listener.h"
#include "automation_interface/fep_ai_state_aggregator.h"
#include "automation_interface/fep_automation.h"
#include "automation_interface/fep_automation_templates.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/fep_module_header_config.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep3/components/rpc/fep_rpc_intf.h"
#include "fep_sdk_participant_version.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "messages/fep_command_access_intf.h"
#include "messages/fep_command_get_property.h"
#include "messages/fep_command_get_signal_info.h"
#include "messages/fep_command_mapping_configuration.h"
#include "messages/fep_command_mute_signal.h"
#include "messages/fep_command_name_change.h"
#include "messages/fep_command_resolve_signal_type.h"
#include "messages/fep_command_signal_description.h"
#include "messages/fep_notification_access_intf.h"
#include "messages/fep_notification_listener.h"
#include "messages/fep_notification_name_changed_intf.h"
#include "messages/fep_notification_state_intf.h"
#include "module/fep_module.h"
#include "module/fep_module_options.h"
#include "signal_registry/fep_user_signal_options.h"
#include "fep3/base/states/fep2_state.h"
#include "statemachine/fep_statemachine_intf.h"
#include "transmission_adapter/fep_signal_direction.h"

#if (defined(_MSC_VER))
// WIN
#include <Windows.h>
#define FULL_QUALIFIED_ELEMENT_NAME(strBaseName) \
        (a_util::strings::format("%s_%s_%lu", (strBaseName), \
         a_util::system::getHostname().c_str(), GetCurrentProcessId()).c_str())
#elif (defined (__linux))
// LINUX x64 gcc46
#include <unistd.h>

#define FULL_QUALIFIED_ELEMENT_NAME(strBaseName) \
        (a_util::strings::format("%s_%s_%lu", (strBaseName), \
         a_util::system::getHostname().c_str(), getpid()).c_str())
#elif (defined(__QNX__))
// QNX x86
#include <unistd.h>
#define FULL_QUALIFIED_ELEMENT_NAME(strBaseName) \
        (a_util::strings::format("%s_%s_%lu", (strBaseName), \
         a_util::system::getHostname().c_str(), static_cast<unsigned long>(getpid())).c_str())
#else
// this goes for vc120 or apple or arm or whatever.
assert("Platform currently not supported");
#endif // Version check

namespace fep {
    class ICommand;
}  // namespace fep

using namespace fep;
using namespace fep::component_config;

// AI implementation
class AutomationInterface::Implementation : public cNotificationListener
{
public:
    Implementation() = default;

public:
    /// The module holding this AI instance
    fep::cModule* m_pModule;
    /// mutex to protect GetAvailableParticipants
    a_util::concurrency::recursive_mutex m_oAvailableModulesMutex;
    /// map of monitors
    typedef std::map<std::string, std::vector<IAutomationParticipantMonitor*>> monitorMapType;
    monitorMapType m_mapMonitors;
    /// mutex for the monitor map
    a_util::concurrency::recursive_mutex _monitor_maps_sync;

public:
    fep::Result PerformModuleHeaderQuery(const char * strPath,
        double & fValue, const std::string& strParticipantName, timestamp_t tmTimeout) const
    {
        fep::Result nResult = ERR_INVALID_ARG;

        if (tmTimeout > 0)
        {
            // remote access
            IProperty * poProp = NULL;
            nResult = m_pModule->GetPropertyTree()->
                GetRemoteProperty(strParticipantName.c_str(), strPath, &poProp, tmTimeout);
            if (fep::isOk(nResult))
            {
                nResult = poProp->GetValue(fValue);
                delete poProp;
            }
        }

        return nResult;
    }

    fep::Result PerformModuleHeaderQuery(const char * strPath,
        std::string& strValue, const std::string& strParticipantName, 
        timestamp_t tmTimeout) const
    {
        fep::Result nResult = ERR_INVALID_ARG;

        if (tmTimeout > 0)
        {
            const char * strPtr = NULL;
            // remote access
            IProperty * poProp = NULL;
            nResult = m_pModule->GetPropertyTree()->
                GetRemoteProperty(strParticipantName.c_str(), strPath, &poProp, tmTimeout);
            if (fep::isOk(nResult))
            {
                nResult = poProp->GetValue(strPtr);
                if (fep::isOk(nResult))
                {
                    strValue.assign(strPtr);
                }
                delete poProp;
            }
        }

        return nResult;
    }

    fep::Result RegisterAndTransmit(cAINotificationListener& oListener,
        ICommand* poCommand)
    {
        ICommandAccess * poCmdAccess = m_pModule->GetCommandAccess();
        INotificationAccess * poNotAccess = m_pModule->GetNotificationAccess();
        if (!poCmdAccess || !poNotAccess)
        {
            return ERR_INVALID_STATE;
        }

        // The user must unregister the notification listener himself if the command is 
        // successfully transmitted (function returns no error)
        poNotAccess->RegisterNotificationListener(&oListener);

        if (fep::isFailed(poCmdAccess->TransmitCommand(poCommand)))
        {
            poNotAccess->UnregisterNotificationListener(&oListener);
            return ERR_FAILED;
        }

        return ERR_NOERROR;
    }

    template<typename T>
    fep::Result GetAvailableParticipants(T& tParticipants, timestamp_t tmDuration /*= FEP_AI_AVAILABLE_PARTICIPANTS_DISCOVER_TIME_MS*/)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(
            m_oAvailableModulesMutex);

        if (0 > tmDuration)
        {
            return ERR_INVALID_ARG;
        }

        cGetPropertyCommand oCmd(g_strElementHeaderPath_strElementCurrentState, m_pModule->GetName(),
            "*", GetTimeStampMicrosecondsUTC(), m_pModule->GetTimingInterface()->GetTime());
        cAINotificationListener oListener(&tParticipants);

        RETURN_IF_FAILED(RegisterAndTransmit(oListener, &oCmd));

        oListener.WaitForAvlModules(tmDuration);
        return m_pModule->GetNotificationAccess()->UnregisterNotificationListener(&oListener);
    }

    fep::Result RegisterMonitoring(const std::string& strParticipantName,
        IAutomationParticipantMonitor* pMonitor)
    {
        using namespace a_util::concurrency;
        unique_lock<recursive_mutex> lock(_monitor_maps_sync);
        std::string strRegexName = strParticipantName;
        a_util::strings::replace(strRegexName, "*", ".*");
        m_mapMonitors[strRegexName].push_back(pMonitor);
        return ERR_NOERROR;
    }
        
    fep::Result UnregisterMonitoring(IAutomationParticipantMonitor* pMonitor)
    {
        using namespace a_util::concurrency;
        unique_lock<recursive_mutex> lock(_monitor_maps_sync);

        std::vector<std::string> vecParticipants;
        /// Remove monitors from vectors
        for (monitorMapType::iterator it = m_mapMonitors.begin(); it != m_mapMonitors.end(); it++)
        {
            for (std::vector<IAutomationParticipantMonitor*>::iterator it2 = it->second.begin();
                it2 != it->second.end(); it2++)
            {
                if (*it2 == pMonitor)
                {
                    it->second.erase(it2);
                    if (it->second.size() == 0)
                    {
                        // if the list is empty, this element should be removed from the map
                        vecParticipants.push_back(it->first);
                    }
                    break;
                }
            }
        }
        // Remove elements without monitor
        for (std::vector<std::string>::iterator it = vecParticipants.begin(); it != vecParticipants.end(); it++)
        {
            m_mapMonitors.erase(*it);
        }
        return ERR_NOERROR;
    }

    /**
    * The method \c Update will be called whenever a state notification was received.
    *
    * @param [in] pStateNotification  The state notification.
    * @returns  Standard result code.
    * @retval ERR_NOERROR  Everything went fine
    */
    fep::Result Update(IStateNotification const * pNotification)
    {
        using namespace a_util::concurrency;
        unique_lock<recursive_mutex> lock(_monitor_maps_sync);

        std::string strSender = pNotification->GetSender();
        for (monitorMapType::iterator it = m_mapMonitors.begin(); it != m_mapMonitors.end(); it++)
        {
            a_util::regex::RegularExpression reg(it->first);
            if (reg.fullMatch(strSender))
            {
                for (std::vector<IAutomationParticipantMonitor*>::iterator it2 = it->second.begin();
                    it2 != it->second.end(); it2++)
                {
                    (*it2)->OnStateChanged(strSender, pNotification->GetState());
                }
            }
        }
        return ERR_NOERROR;
    }

    /**
    * The method \c Update will be called whenever a name changed notification was received.
    *
    * @param [in] pNotification  The notification
    * @returns  Standard result code.
    * @retval ERR_NOERROR  Everything went fine
    */
    fep::Result Update(INameChangedNotification const * pNotification)
    {
        using namespace a_util::concurrency;
        unique_lock<recursive_mutex> lock(_monitor_maps_sync);

        std::string strSender = pNotification->GetSender();
        for (monitorMapType::iterator it = m_mapMonitors.begin(); it != m_mapMonitors.end(); it++)
        {
            a_util::regex::RegularExpression reg(it->first);
            if (reg.fullMatch(strSender))
            {
                for (std::vector<IAutomationParticipantMonitor*>::iterator it2 = it->second.begin();
                    it2 != it->second.end(); it2++)
                {
                    (*it2)->OnNameChanged(strSender, pNotification->GetOldParticipantName());
                }
            }
        }
        return ERR_NOERROR;
    }
};

/*****************************************
* FEP Automation Interface
*****************************************/

fep::AutomationInterface::AutomationInterface(const cModuleOptions& oOptions /*= cModuleOptions()*/, 
    const std::string& strTypeId /*= "082d3108-c94d-11e7-abc4-cec278b6b50a"*/, 
    const std::string& strDescription /*= "FEP Automation Interface"*/, float fVersion, 
    const std::string& strVendor /*= "Audi Electronics Venture GmbH"*/, 
    const std::string& strDisplayName /*= ""*/, const std::string& strContext /*= "none"*/, 
    int nContextVersion /*= -1*/) : _impl(new Implementation()), m_bExternModule(false)
{
    _impl->m_pModule = new fep::cModule();

    cModuleOptions oOptionsTmp = oOptions;
    if (!oOptionsTmp.GetParticipantName() || a_util::strings::isEmpty(oOptionsTmp.GetParticipantName()))
    {
        oOptionsTmp.SetParticipantName(FULL_QUALIFIED_ELEMENT_NAME("AutomationInterface"));
    }
    else
    {
        oOptionsTmp.SetParticipantName(oOptionsTmp.GetParticipantName());
    }

    Result nRes = _impl->m_pModule->Create(oOptionsTmp);

    /*Fill participant header*/
    _impl->m_pModule->GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fFEPVersion,
        static_cast<float>(static_cast<float>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
            static_cast<float>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10));
    _impl->m_pModule->GetPropertyTree()->SetPropertyValue(
        fep::g_strElementHeaderPath_strTypeID, strTypeId.c_str());
    _impl->m_pModule->GetPropertyTree()->SetPropertyValue(
        fep::g_strElementHeaderPath_strElementDescription, strDescription.c_str());
    _impl->m_pModule->GetPropertyTree()->SetPropertyValue(
        fep::g_strElementHeaderPath_fElementVersion, fVersion);
    _impl->m_pModule->GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementVendor,
        strVendor.c_str());
    std::string strDisplayNameTmp = strDisplayName;
    if (strDisplayNameTmp.empty())
    {
        strDisplayNameTmp = a_util::strings::format("%s (FEP Automation Interface) @ %s", (_impl->m_pModule->GetName()),
            a_util::system::getHostname().c_str());
    }
    _impl->m_pModule->GetPropertyTree()->SetPropertyValue(
        fep::g_strElementHeaderPath_strElementDisplayName,
        strDisplayNameTmp.c_str());
    _impl->m_pModule->GetPropertyTree()->SetPropertyValue(\
        fep::g_strElementHeaderPath_strElementContext, strContext.c_str());
    _impl->m_pModule->GetPropertyTree()->SetPropertyValue(\
        fep::g_strElementHeaderPath_fElementContextVersion, nContextVersion);

    if (isOk(nRes))
    {
        // Enable Incident Handling
        nRes = _impl->m_pModule->GetPropertyTree()->SetPropertyValue(
            common_config::g_strIncidentHandlerPath_bEnable, true);
        nRes = _impl->m_pModule->GetPropertyTree()->SetPropertyValue(
            common_config::g_strIncidentHandlerPath_bEnableGlobalScope, true);
        // Disable console strategy
        nRes = _impl->m_pModule->GetPropertyTree()->SetPropertyValue(
            common_config::g_strIncidentConsoleLogPath_bEnable, false);
        _impl->m_pModule->GetNotificationAccess()->RegisterNotificationListener(_impl.get());
        _impl->m_pModule->SetStandAloneModeEnabled(true);
        _impl->m_pModule->GetStateMachine()->StartupDoneEvent();
        _impl->m_pModule->WaitForState(FS_IDLE);
    }
}

fep::AutomationInterface::AutomationInterface(cModule& oModule) : _impl(new Implementation()), m_bExternModule(true)
{
    _impl->m_pModule = &oModule;

    if (!_impl->m_pModule->GetName() || (strlen(_impl->m_pModule->GetName()) == 0))
    {
        throw std::invalid_argument("module must be created");
    }       
    else
    {
        // Enable Incident Handling
        Result nRes = _impl->m_pModule->GetPropertyTree()->SetPropertyValue(
            common_config::g_strIncidentHandlerPath_bEnable, true);
        nRes = _impl->m_pModule->GetPropertyTree()->SetPropertyValue(
            common_config::g_strIncidentHandlerPath_bEnableGlobalScope, true);
        // Disable console strategy
        nRes = _impl->m_pModule->GetPropertyTree()->SetPropertyValue(
            common_config::g_strIncidentConsoleLogPath_bEnable, false);
        // Add notification listener
        _impl->m_pModule->GetNotificationAccess()->RegisterNotificationListener(_impl.get());
    }
}

AutomationInterface::~AutomationInterface()
{
    _impl->m_pModule->GetNotificationAccess()->UnregisterNotificationListener(_impl.get());
    // Shutdown Module if it is our own
    if (!m_bExternModule)
    {
        _impl->m_pModule->Destroy();
        delete _impl->m_pModule;
    }
}

/*        * Local Incidents Monitoring        */
fep::Result AutomationInterface::SetIncidentHistoryEnabled(bool bEnable)
{
    Result nRes = _impl->m_pModule->GetPropertyTree()->SetPropertyValue(
        common_config::g_strIncidentHistoryLogPath_bEnable, bEnable);
    if (isOk(nRes))
    {
        nRes = _impl->m_pModule->GetPropertyTree()->SetPropertyValue(
            common_config::g_strIncidentHistoryLogPath_bEnableCatchAll, bEnable);
    }
    return nRes;
}

fep::Result AutomationInterface::SetIncidentConsoleLogEnabled(bool bEnable)
{
    Result nRes = _impl->m_pModule->GetPropertyTree()->SetPropertyValue(
        common_config::g_strIncidentConsoleLogPath_bEnable, bEnable);
    if (isOk(nRes))
    {
        nRes = _impl->m_pModule->GetPropertyTree()->SetPropertyValue(
            common_config::g_strIncidentConsoleLogPath_bEnableCatchAll, bEnable);
    }
    return nRes;
}

fep::Result AutomationInterface::GetLastIncident(
    const fep::tIncidentEntry** ppIncidentEntry) const
{
    return _impl->m_pModule->GetIncidentHandler()->GetLastIncident(ppIncidentEntry);
}

fep::Result AutomationInterface::RetrieveIncidentHistory(
    tIncidentListConstIter& io_iterHistBegin, tIncidentListConstIter& io_iterHistEnd) const
{
    return _impl->m_pModule->GetIncidentHandler()->RetrieveIncidentHistory(io_iterHistBegin, 
        io_iterHistEnd);
}

fep::Result AutomationInterface::FreeIncidentHistory()
{
    return _impl->m_pModule->GetIncidentHandler()->FreeIncidentHistory();
}

fep::Result AutomationInterface::AssociateStrategy(int16_t nFEPIncident, 
    IAutomationIncidentStrategy* pStrategyDelegate, 
    const std::string& strConfigurationPath /*= ""*/, 
    tStrategyAssociation eAssociation /*= SA_REPLACE*/)
{
    return _impl->m_pModule->GetIncidentHandler()->AssociateStrategy(nFEPIncident, 
        pStrategyDelegate, strConfigurationPath.c_str(), eAssociation);
}

fep::Result AutomationInterface::AssociateCatchAllStrategy(
    IAutomationIncidentStrategy* pStrategyDelegate,
    const std::string& strConfigurationPath /*= ""*/, 
    const tStrategyAssociation eAssociation /*= SA_APPEND*/)
{
    return _impl->m_pModule->GetIncidentHandler()->AssociateCatchAllStrategy(
        pStrategyDelegate, strConfigurationPath.c_str(), eAssociation);
}

fep::Result AutomationInterface::DisassociateStrategy(const int16_t eFEPIncident,
    fep::IAutomationIncidentStrategy* pStrategyDelegate)
{
    return _impl->m_pModule->GetIncidentHandler()->DisassociateStrategy(eFEPIncident,
        pStrategyDelegate);
}

fep::Result AutomationInterface::DisassociateCatchAllStrategy(
    IAutomationIncidentStrategy* pStrategyDelegate)
{
    return _impl->m_pModule->GetIncidentHandler()->DisassociateCatchAllStrategy(
        pStrategyDelegate);
}

/*********************Configuring****************************/
/*        * Signals        */
fep::Result AutomationInterface::ResolveSignalType(const std::string& strSignalType, 
    std::string& strSignalDescription, const std::string& strParticipantName, 
    timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    if (strParticipantName.empty() || ContainsWildcards(strParticipantName) ||
        0 >= tmTimeout)
    {
        return ERR_INVALID_ARG;
    }

    fep::Result nResult = ERR_NOERROR;
    char const * strPtr = NULL;

    // to signal registry
    cResolveSignalTypeCommand oCmd(strSignalType.c_str(), _impl->m_pModule->GetName(),
        strParticipantName.c_str(), GetTimeStampMicrosecondsUTC(), 
        _impl->m_pModule->GetTimingInterface()->GetTime());

    cAINotificationListener oListener(
        cAINotificationListener::AIRT_RESOLVE_SIGNAL_TYPE, strParticipantName.c_str());
    nResult = _impl->RegisterAndTransmit(oListener, &oCmd);
    if (fep::isOk(nResult) && ERR_TIMEOUT == oListener.WaitForDescription(tmTimeout))
    {
        nResult = ERR_TIMEOUT;
    }
    if (fep::isOk(nResult))
    {
        _impl->m_pModule->GetNotificationAccess()->UnregisterNotificationListener(&oListener);
        nResult = oListener.GetSignalDescription(strPtr);
    }
    if (fep::isOk(nResult))
    {
        strSignalDescription.assign(strPtr);
    }
    return nResult;
}



fep::Result AutomationInterface::RegisterSignalDescription(const std::string& strDescription, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/, 
    uint32_t ui32DescriptionFlags /*= ISignalRegistry::DF_REPLACE*/) const
{
    if (strDescription.empty())
    {
        return ERR_INVALID_ARG;
    }

    fep::Result nRes = ERR_NOERROR;
    if (strParticipantName.empty() || ContainsWildcards(strParticipantName) || 0 >= tmTimeout)
    {
        nRes = ERR_INVALID_ARG;
    }
    else
    {
        // do call remotely
        cSignalDescriptionCommand oCmd(strDescription.c_str(), ui32DescriptionFlags,
            _impl->m_pModule->GetName(), strParticipantName.c_str(), GetTimeStampMicrosecondsUTC(),
            _impl->m_pModule->GetTimingInterface()->GetTime());

        nRes = PerformCommandAwaitResult<cSignalDescriptionCommand>(_impl->m_pModule->GetCommandAccess(),
            _impl->m_pModule->GetNotificationAccess(), strParticipantName.c_str(), tmTimeout, oCmd);
    }

    return nRes;
}

fep::Result AutomationInterface::ClearSignalDescriptions(const std::string& strParticipantName, 
    timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nRes = ERR_NOERROR;
    if (strParticipantName.empty() || ContainsWildcards(strParticipantName) || 0 >= tmTimeout)
    {
        nRes = ERR_INVALID_ARG;
    }
    else
    {
        // do call remotely
        cSignalDescriptionCommand oCmd(_impl->m_pModule->GetName(),
            strParticipantName.c_str(), GetTimeStampMicrosecondsUTC(), 
            _impl->m_pModule->GetTimingInterface()->GetTime());

        nRes = PerformCommandAwaitResult<cSignalDescriptionCommand>(_impl->m_pModule->GetCommandAccess(),
            _impl->m_pModule->GetNotificationAccess(), strParticipantName.c_str(), tmTimeout, oCmd);
    }

    return nRes;
}

fep::Result AutomationInterface::RegisterMappingConfiguration(const std::string& strConfiguration, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/, 
    uint32_t ui32MappingFlags /*= ISignalMapping::MF_REPLACE*/) const
{
    if (strConfiguration.empty())
    {
        return ERR_INVALID_ARG;
    }

    fep::Result nRes = ERR_NOERROR;
    if (strParticipantName.empty() || ContainsWildcards(strParticipantName) || 0 >= tmTimeout)
    {
        nRes = ERR_INVALID_ARG;
    }
    else
    {
        // do call remotely
        cMappingConfigurationCommand oCmd(strConfiguration.c_str(), ui32MappingFlags,
            _impl->m_pModule->GetName(), strParticipantName.c_str(), GetTimeStampMicrosecondsUTC(),
            _impl->m_pModule->GetTimingInterface()->GetTime());

        nRes = PerformCommandAwaitResult<cMappingConfigurationCommand>(_impl->m_pModule->GetCommandAccess(),
            _impl->m_pModule->GetNotificationAccess(), strParticipantName.c_str(), tmTimeout, oCmd);
    }

    return nRes;
}

fep::Result AutomationInterface::ClearMappingConfiguration(const std::string& strParticipantName, 
    timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nRes = ERR_NOERROR;
    if (strParticipantName.empty() || ContainsWildcards(strParticipantName) || 0 >= tmTimeout)
    {
        nRes = ERR_INVALID_ARG;
    }
    else
    {
        // do call remotely
        cMappingConfigurationCommand oCmd(_impl->m_pModule->GetName(),
            strParticipantName.c_str(), GetTimeStampMicrosecondsUTC(), 
            _impl->m_pModule->GetTimingInterface()->GetTime());

        nRes = PerformCommandAwaitResult<cMappingConfigurationCommand>(_impl->m_pModule->GetCommandAccess(),
            _impl->m_pModule->GetNotificationAccess(), strParticipantName.c_str(), tmTimeout, oCmd);
    }

    return nRes;
}

/*        * Properties        */

fep::Result AutomationInterface::SetPropertyValue(const std::string& strPropPath,
    const std::string& strValue, const std::string& strParticipantName, 
    const timestamp_t tmTimeout /* = 0*/) const
{
    return _impl->m_pModule->GetPropertyTree()->SetRemotePropertyValue(
        strParticipantName.c_str(), strPropPath.c_str(), strValue.c_str(),
        tmTimeout);
}

fep::Result AutomationInterface::SetPropertyValue(const std::string& strPropPath, 
    double f64Value, const std::string& strParticipantName,
    const timestamp_t tmTimeout /* = 0*/) const
{
    return _impl->m_pModule->GetPropertyTree()->SetRemotePropertyValue(
        strParticipantName.c_str(), strPropPath.c_str(), f64Value,
        tmTimeout);
}

fep::Result AutomationInterface::SetPropertyValue(const std::string& strPropPath, 
    int32_t n32Value, const std::string& strParticipantName,
    const timestamp_t tmTimeout /* = 0*/) const
{
    return _impl->m_pModule->GetPropertyTree()->SetRemotePropertyValue(
        strParticipantName.c_str(), strPropPath.c_str(), n32Value,
        tmTimeout);
}

fep::Result AutomationInterface::SetPropertyValue(const std::string& strPropPath, 
    bool bValue, const std::string& strParticipantName,
    const timestamp_t tmTimeout /* = 0*/) const
{
    return _impl->m_pModule->GetPropertyTree()->SetRemotePropertyValue(
        strParticipantName.c_str(), strPropPath.c_str(), bValue,
        tmTimeout);
}

fep::Result AutomationInterface::SetPropertyValues(const std::string& strPropPath, 
    const std::vector<std::string>& strValues, const std::string& strParticipantName,
    const timestamp_t tmTimeout /* = 0*/) const
{
    std::vector<const char*> cstrings;
    for (size_t i = 0; i < strValues.size(); ++i)
    {
        cstrings.push_back(strValues[i].c_str());
    }
    return _impl->m_pModule->GetPropertyTree()->SetRemotePropertyValues(
        strParticipantName.c_str(), strPropPath.c_str(), &cstrings[0], cstrings.size(),
        tmTimeout);
}

fep::Result AutomationInterface::SetPropertyValues(const std::string& strPropPath, 
    const std::vector<double>& f64Values, const std::string& strParticipantName,
    const timestamp_t tmTimeout /* = 0*/) const
{
    return _impl->m_pModule->GetPropertyTree()->SetRemotePropertyValues(
        strParticipantName.c_str(), strPropPath.c_str(), &f64Values[0], f64Values.size(),
        tmTimeout);
}

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic ignored "-Wattributes" // standard type attributes are ignored when used in templates
#endif

fep::Result AutomationInterface::SetPropertyValues(const std::string& strPropPath, 
    const std::vector<int32_t>& n32Values, const std::string& strParticipantName,
    const timestamp_t tmTimeout /* = 0*/) const
{
    return _impl->m_pModule->GetPropertyTree()->SetRemotePropertyValues(
        strParticipantName.c_str(), strPropPath.c_str(), &n32Values[0], n32Values.size(),
        tmTimeout);
}

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic warning "-Wattributes" // standard type attributes are ignored when used in templates
#endif

struct Bool { bool b; };

fep::Result AutomationInterface::SetPropertyValues(const std::string& strPropPath, 
    const std::vector<bool>& bValues, const std::string& strParticipantName,
    const timestamp_t tmTimeout /* = 0*/) const
{
    /// Need a conversion because vector<bool> is not a real container
    std::vector<Bool> vecTmpBool;
    for (size_t i = 0; i < bValues.size(); ++i)
    {
        Bool oTmpBool;
        oTmpBool.b = bValues[i];
        vecTmpBool.push_back(oTmpBool);
    }
    return _impl->m_pModule->GetPropertyTree()->SetRemotePropertyValues( strParticipantName.c_str(), 
        strPropPath.c_str(), (bool*)&vecTmpBool[0], vecTmpBool.size(), tmTimeout);
}

fep::Result AutomationInterface::DeleteProperty(const std::string& strPropPath, 
    const std::string& strParticipantName) const
{
    return _impl->m_pModule->GetPropertyTree()->DeleteRemoteProperty(
        strParticipantName.c_str(), strPropPath.c_str());
}

/*********************Controling****************************/
/*        * Participants        */
fep::Result AutomationInterface::MuteParticipant(const std::string& strParticipantName, 
    timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nResult = ERR_NOERROR;
    cMuteSignalCommand oCmd("*", SD_Output, true,
        _impl->m_pModule->GetName(), strParticipantName.c_str(),
        GetTimeStampMicrosecondsUTC(),
        _impl->m_pModule->GetTimingInterface()->GetTime());
    nResult = PerformCommandAwaitResult<cMuteSignalCommand>(_impl->m_pModule->GetCommandAccess(),
        _impl->m_pModule->GetNotificationAccess(), strParticipantName.c_str(), tmTimeout, oCmd);
    return nResult;
}

fep::Result AutomationInterface::UnmuteParticipant(const std::string& strParticipantName, 
    timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nResult = ERR_NOERROR;
    cMuteSignalCommand oCmd("*", SD_Output, false,
        _impl->m_pModule->GetName(), strParticipantName.c_str(),
        GetTimeStampMicrosecondsUTC(),
        _impl->m_pModule->GetTimingInterface()->GetTime());
    nResult = PerformCommandAwaitResult<cMuteSignalCommand>(_impl->m_pModule->GetCommandAccess(),
        _impl->m_pModule->GetNotificationAccess(), strParticipantName.c_str(), tmTimeout, oCmd);
    return nResult;
}

fep::Result AutomationInterface::IsParticipantMuted(bool& bStatus, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nResult = ERR_NOERROR;
    bool bGlobalMute = false;

    nResult = GetPropertyValue(g_strElementHeaderPath_bGlobalMute,
        bGlobalMute, strParticipantName, tmTimeout);

    if (fep::isOk(nResult))
    {
        bStatus = bGlobalMute;
    }

    if (ERR_INVALID_TYPE == nResult || ERR_PATH_NOT_FOUND == nResult)
    {
        nResult = ERR_UNEXPECTED;
    }

    return nResult;
}

fep::Result  AutomationInterface::RenameParticipant(const std::string& strNewParticipantName, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nRes = ERR_NOERROR;
    fep::cModuleOptions oModuleOptions(strNewParticipantName.c_str());
    bool bIsValid = false;
    if (oModuleOptions.CheckValidity() == ERR_NOERROR)
    {
        bIsValid = true;
    }
    if (strNewParticipantName.empty() || strParticipantName.empty() || ContainsWildcards(strParticipantName)
        || !bIsValid || 0 >= tmTimeout)
    {
        nRes = ERR_INVALID_ARG;
    }
    else
    {
        // do call remotely
        cNameChangeCommand oCmd(strNewParticipantName.c_str(), _impl->m_pModule->GetName(),
            strParticipantName.c_str(), GetTimeStampMicrosecondsUTC(), 
            _impl->m_pModule->GetTimingInterface()->GetTime());

        cAINotificationListener oListener(cAINotificationListener::AIRT_NAME_CHANGED, 
            strNewParticipantName.c_str());
        oListener.SetNameChangedParams(strParticipantName.c_str());

        nRes = _impl->RegisterAndTransmit(oListener, &oCmd);

        if (fep::isOk(nRes))
        {
            nRes = oListener.WaitForNameChanged(tmTimeout);
            _impl->m_pModule->GetNotificationAccess()->UnregisterNotificationListener(&oListener);
        }
    }

    return nRes;
};

/*        * Signals        */
fep::Result AutomationInterface::MuteSignal(const std::string& strSignalName, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nResult = ERR_NOERROR;
    if (strParticipantName.empty() || ContainsWildcards(strParticipantName))
    {
        return ERR_INVALID_ARG;
    }

    cMuteSignalCommand oCmd(strSignalName.c_str(), fep::SD_Output, true,
        _impl->m_pModule->GetName(), strParticipantName.c_str(),
        GetTimeStampMicrosecondsUTC(),
        _impl->m_pModule->GetTimingInterface()->GetTime());

    nResult = PerformCommandAwaitResult<cMuteSignalCommand>(_impl->m_pModule->GetCommandAccess(),
        _impl->m_pModule->GetNotificationAccess(), strParticipantName.c_str(), tmTimeout, oCmd);

    return nResult;
}

fep::Result AutomationInterface::UnmuteSignal(const std::string& strSignalName, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nResult = ERR_NOERROR;
    cMuteSignalCommand oCmd(strSignalName.c_str(), fep::SD_Output, false,
        _impl->m_pModule->GetName(), strParticipantName.c_str(),
        GetTimeStampMicrosecondsUTC(),
        _impl->m_pModule->GetTimingInterface()->GetTime());

    nResult = PerformCommandAwaitResult<cMuteSignalCommand>(_impl->m_pModule->GetCommandAccess(),
        _impl->m_pModule->GetNotificationAccess(), strParticipantName.c_str(), tmTimeout, oCmd);

    return nResult;
}

fep::Result AutomationInterface::IsSignalMuted(const std::string& strSignalName, 
    bool& bStatus, const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nResult = ERR_NOERROR;
    std::string strPPSignalMuted = a_util::strings::format("%s.%s.%s",
        fep::component_config::g_strSignalRegistryPath_RegisteredOutSignals,
        strSignalName.c_str(), fep::component_config::g_strSignalRegistryField_MutedSignal);

    nResult = GetPropertyValue(strPPSignalMuted.c_str(),
        bStatus, strParticipantName, tmTimeout);

    if (ERR_PATH_NOT_FOUND == nResult || ERR_INVALID_TYPE == nResult)
    {
        nResult = ERR_UNEXPECTED;
    }
    return nResult;
}

/*        * Events        */
fep::Result AutomationInterface::TriggerEvent(fep::tControlEvent eEvent, 
    const std::string& strParticipantName) const
{
    IStateMachine * pSTM = _impl->m_pModule->GetStateMachine();

    /* trigger remote event */
    return pSTM->TriggerRemoteEvent(eEvent, strParticipantName.c_str());
}

fep::Result fep::AutomationInterface::TriggerEventSync(fep::tControlEvent eEvent, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nResult = ERR_NOERROR;
    IStateMachine * pSTM = _impl->m_pModule->GetStateMachine();

    if (tmTimeout <= 0 || ContainsWildcards(strParticipantName))
    {
        return ERR_INVALID_ARG;
    }

    // find state that should follow the specified event
    tState eState = FS_ERROR;
    switch (eEvent)
    {
    case CE_Initialize:
        eState = FS_READY;
        break;
    case CE_Start:
        eState = FS_RUNNING;
        break;
    case CE_Stop:
        eState = FS_IDLE;
        break;
    case CE_Shutdown:
        eState = FS_SHUTDOWN;
        break;
    case CE_ErrorFixed:
        eState = FS_IDLE;
        break;
    case CE_Restart:
        eState = FS_STARTUP;
        break;
    default:
        /* if we get here, at least one ControlEvent is missing above */
        assert(false);
    }

    cAINotificationListener oListener(strParticipantName.c_str(), eState);
    _impl->m_pModule->GetNotificationAccess()->RegisterNotificationListener(&oListener);

    /* trigger remote event */
    nResult = pSTM->TriggerRemoteEvent(eEvent, strParticipantName.c_str());

    // await state change notification and unregister listener
    if (fep::isOk(nResult))
    {
        nResult = oListener.WaitForState(tmTimeout);
    }
    _impl->m_pModule->GetNotificationAccess()->
        UnregisterNotificationListener(&oListener);

    return nResult;
}

/*********************Monitoring****************************/
/*        * Participants        */
fep::Result AutomationInterface::GetAvailableParticipants(std::vector<std::string>& vecParticipants, 
    timestamp_t tmDuration /*= FEP_AI_AVAILABLE_PARTICIPANTS_DISCOVER_TIME_MS*/) const
{
    vecParticipants.clear();
    std::set<std::string> setEls;
    RETURN_IF_FAILED(_impl->GetAvailableParticipants(setEls, tmDuration));
    vecParticipants.assign(setEls.begin(), setEls.end());
    return fep::ERR_NOERROR;
}

fep::Result fep::AutomationInterface::GetAvailableParticipants(std::map<std::string, 
    fep::tState>& mapParticipants, timestamp_t tmDuration /*= FEP_AI_AVAILABLE_PARTICIPANTS_DISCOVER_TIME_MS*/) const
{
    return _impl->GetAvailableParticipants(mapParticipants, tmDuration);
}

fep::Result AutomationInterface::GetParticipantVersion(double& fVersion, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    return _impl->PerformModuleHeaderQuery(g_strElementHeaderPath_fElementVersion,
        fVersion, strParticipantName, tmTimeout);
}

fep::Result AutomationInterface::GetParticipantDisplayName(std::string& strDisplayName, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    return _impl->PerformModuleHeaderQuery(g_strElementHeaderPath_strElementDisplayName,
        strDisplayName, strParticipantName, tmTimeout);
}

fep::Result AutomationInterface::GetParticipantDescription(std::string& strDescription, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nRes = _impl->PerformModuleHeaderQuery(g_strElementHeaderPath_strElementDescription,
        strDescription, strParticipantName, tmTimeout);
    return nRes;
}

fep::Result AutomationInterface::GetParticipantVendor(std::string& strVendor, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nRes = _impl->PerformModuleHeaderQuery(g_strElementHeaderPath_strElementVendor,
        strVendor, strParticipantName, tmTimeout);
    return nRes;
}

fep::Result AutomationInterface::GetParticipantCompilationDate(std::string& strCompDate, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nRes = _impl->PerformModuleHeaderQuery(g_strElementHeaderPath_strElementCompilationDate,
        strCompDate, strParticipantName, tmTimeout);
    return nRes;
}

fep::Result AutomationInterface::GetParticipantFEPVersion(double& fFEPVersion, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    return _impl->PerformModuleHeaderQuery(g_strElementHeaderPath_fFEPVersion,
        fFEPVersion, strParticipantName, tmTimeout);
}

fep::Result AutomationInterface::GetParticipantPlatform(std::string& strPlatform, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nRes = _impl->PerformModuleHeaderQuery(g_strElementHeaderPath_strElementPlatform,
        strPlatform, strParticipantName, tmTimeout);
    return nRes;
}

fep::Result AutomationInterface::GetParticipantContext(std::string& strContext, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nRes = _impl->PerformModuleHeaderQuery(g_strElementHeaderPath_strElementContext,
        strContext, strParticipantName, tmTimeout);
    return nRes;
}

fep::Result AutomationInterface::GetParticipantHostName(std::string& strHostName, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    return _impl->PerformModuleHeaderQuery(g_strElementHeaderPath_strElementHost,
        strHostName, strParticipantName, tmTimeout);
}

fep::Result AutomationInterface::GetParticipantInstanceID(std::string& strInstanceID, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    return _impl->PerformModuleHeaderQuery(g_strElementHeaderPath_strInstanceID,
        strInstanceID, strParticipantName, tmTimeout);
}

fep::Result AutomationInterface::GetParticipantTypeID(std::string& strTypeID, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    return _impl->PerformModuleHeaderQuery(g_strElementHeaderPath_strTypeID,
        strTypeID, strParticipantName, tmTimeout);
}

fep::Result AutomationInterface::GetParticipantContextVersion(double& fContextVersion, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    return _impl->PerformModuleHeaderQuery(g_strElementHeaderPath_fElementContextVersion,
        fContextVersion, strParticipantName, tmTimeout);
}

fep::Result AutomationInterface::GetParticipantSignals(std::vector<fep::cUserSignalOptions>& oSignals,
    const std::string& strParticipantName,
    timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nResult = ERR_NOERROR;

    if (strParticipantName.empty() || 0 >= tmTimeout || ContainsWildcards(strParticipantName))
    {
        nResult = ERR_INVALID_ARG;
    }
    else
    { /* valid remote request, single element, valid timeout */
        cGetSignalInfoCommand oCmd(_impl->m_pModule->GetName(), strParticipantName.c_str(),
            GetTimeStampMicrosecondsUTC(), _impl->m_pModule->GetTimingInterface()->GetTime());
        cAINotificationListener oListener(
            cAINotificationListener::AIRT_SIGNAL_INFO, strParticipantName.c_str());
        nResult = _impl->RegisterAndTransmit(oListener, &oCmd);
        if (fep::isOk(nResult))
        {
            nResult = oListener.WaitForSignalInfo(tmTimeout);
            _impl->m_pModule->GetNotificationAccess()->UnregisterNotificationListener(&oListener);
        }
        if (fep::isOk(nResult))
        {
            oListener.GetParticipantSignals(oSignals);
        }
    }

    return nResult;
}

fep::Result fep::AutomationInterface::GetParticipantsSignals(
    std::map< std::string, std::vector<fep::cUserSignalOptions>>& oSignalsMap, 
    const std::vector<std::string>& vecParticipantList, 
    timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    if (0 >= tmTimeout)
    {
        return ERR_INVALID_ARG;
    }
    oSignalsMap.clear();

    fep::Result nOutResult = ERR_NOERROR;
    for (std::vector<std::string>::const_iterator it = vecParticipantList.begin();
        it != vecParticipantList.end(); it++)
    {
        fep::Result nResult = ERR_NOERROR;
        /* valid remote request, single element, valid timeout */
        cGetSignalInfoCommand oCmd(_impl->m_pModule->GetName(), it->c_str(),
            GetTimeStampMicrosecondsUTC(), _impl->m_pModule->GetTimingInterface()->GetTime());
        cAINotificationListener oListener(
            cAINotificationListener::AIRT_SIGNAL_INFO, it->c_str());
        nResult = _impl->RegisterAndTransmit(oListener, &oCmd);
        if (fep::isOk(nResult))
        {
            nResult = oListener.WaitForSignalInfo(tmTimeout);
            _impl->m_pModule->GetNotificationAccess()->UnregisterNotificationListener(&oListener);
        }
        if (fep::isOk(nResult))
        {
            std::vector<fep::cUserSignalOptions> oSignals;
            oListener.GetParticipantSignals(oSignals);
            oSignalsMap[it->c_str()] = oSignals;
        }
        else
        {
            nOutResult = nResult;
        }
    }
    return nOutResult;
}

/*        * Properties        */
fep::Result fep::AutomationInterface::GetProperty(const std::string& strPropPath, 
    a_util::memory::unique_ptr<fep::IProperty>& pProperty, const std::string& strParticipantName,
    timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nResult;
    IProperty* pPropTmp;
    nResult = _impl->m_pModule->GetPropertyTree()->GetRemoteProperty(strParticipantName.c_str(),
        strPropPath.c_str(), &pPropTmp, tmTimeout);
    pProperty.reset(pPropTmp);
    return nResult;
}

fep::Result AutomationInterface::GetPropertyValue(const std::string& strPropPath, 
    std::string& strValue, const std::string& strParticipantName, 
    timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nResult = ERR_NOERROR;
    char const * pValue = NULL;
    IProperty * pProperty = NULL;

    if (0 >= tmTimeout)
    {
        nResult = ERR_INVALID_ARG;
    }
    else
    {
        nResult = _impl->m_pModule->GetPropertyTree()->GetRemoteProperty(
            strParticipantName.c_str(), strPropPath.c_str(), &pProperty, tmTimeout);
        if (fep::isOk(nResult))
        {
            nResult = pProperty->GetValue(pValue);
        }
    }

    if (fep::isOk(nResult))
    {
        strValue.assign(pValue);
    }
    delete pProperty;

    return nResult;
}

fep::Result AutomationInterface::GetPropertyValue(const std::string& strPropPath, 
    double& fValue, const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    return GetPropertyValueTemplate(
        _impl->m_pModule, strPropPath, fValue, strParticipantName, tmTimeout);
}

fep::Result AutomationInterface::GetPropertyValue(const std::string& strPropPath, 
    int32_t& nValue, const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    return GetPropertyValueTemplate(
        _impl->m_pModule, strPropPath, nValue, strParticipantName, tmTimeout);
}

fep::Result AutomationInterface::GetPropertyValue(const std::string& strPropPath, 
    bool& bValue, const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    return GetPropertyValueTemplate(
        _impl->m_pModule, strPropPath, bValue, strParticipantName, tmTimeout);
}

fep::Result AutomationInterface::GetPropertyValues(const std::string& strPropPath, 
    std::vector<std::string>& strValue, const std::string& strParticipantName,
    timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nResult = ERR_NOERROR;
    if (0 >= tmTimeout)
    {
        nResult = ERR_INVALID_ARG;
    }
    else
    {
        IProperty * pProperty = NULL;
        nResult = _impl->m_pModule->GetPropertyTree()->GetRemoteProperty(
            strParticipantName.c_str(), strPropPath.c_str(), &pProperty, tmTimeout);
        if (fep::isOk(nResult))
        {
            size_t szSize = pProperty->GetArraySize();
            for (size_t szIndex = 0; szIndex < szSize; szIndex++)
            {
                char const * pValue = NULL;
                nResult = pProperty->GetValue(pValue, szIndex);
                if (isFailed(nResult))
                {
                    strValue.clear();
                    break;
                }
                strValue.push_back(pValue);
            }
        }
        delete pProperty;
    }
    return nResult;
}

fep::Result AutomationInterface::GetPropertyValues(const std::string& strPropPath,
    std::vector<double>& fValue, const std::string& strParticipantName,
    timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    return GetPropertyValuesTemplate(
        _impl->m_pModule, strPropPath, fValue, strParticipantName, tmTimeout);
}

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic ignored "-Wattributes" // standard type attributes are ignored when used in templates
#endif

fep::Result AutomationInterface::GetPropertyValues(const std::string& strPropPath,
    std::vector<int32_t>& nValue, const std::string& strParticipantName,
    timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    return GetPropertyValuesTemplate(
        _impl->m_pModule, strPropPath, nValue, strParticipantName, tmTimeout);
}

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic warning "-Wattributes" // standard type attributes are ignored when used in templates
#endif

fep::Result AutomationInterface::GetPropertyValues(const std::string& strPropPath,
    std::vector<bool>& bValue, const std::string& strParticipantName,
    timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    return GetPropertyValuesTemplate(
        _impl->m_pModule, strPropPath, bValue, strParticipantName, tmTimeout);
}

/*        * States        */
fep::Result AutomationInterface::CheckParticipantAvailability(const std::string& strParticipantName, 
    timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    fep::Result nResult = NULL != _impl->m_pModule->GetCommandAccess();
    if (fep::isFailed(nResult))
    {
        nResult = ERR_INVALID_STATE;
    }
    else
    {
        IProperty * poProp = NULL;
        nResult = _impl->m_pModule->GetPropertyTree()->GetRemoteProperty(strParticipantName.c_str(),
            g_strElementHeaderPath_strElementName, &poProp, tmTimeout);

        // we don't care about the property, nResult is all we need
        delete poProp;
    }

    return nResult;
}

fep::Result AutomationInterface::WaitForParticipantState(tState eState, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    // register notification listener now, in case the state of the
    // remote module changes after GetRemoteState returns but before we
    // could register a listener
    cAINotificationListener oAwaiter(strParticipantName.c_str(), eState);
    _impl->m_pModule->GetNotificationAccess()->RegisterNotificationListener(&oAwaiter);

    // trigger the search
    tState eCurrentState;
    fep::Result nResult = _impl->m_pModule->GetStateMachine()->
        GetRemoteState(strParticipantName.c_str(), eCurrentState, 0);

    // If either the module could not be reached or the module's state was not the one ...
    if ((fep::isFailed(nResult) || (eCurrentState != eState)) && (nResult != ERR_INVALID_ARG))
    {
        // ... wait for state change for the specified duration
        nResult = oAwaiter.WaitForState(tmTimeout);
        nResult = (ERR_TIMEOUT == nResult) ? (ERR_INVALID_STATE) : (nResult);
    }

    _impl->m_pModule->GetNotificationAccess()->UnregisterNotificationListener(&oAwaiter);

    return nResult;
}

fep::Result AutomationInterface::GetParticipantState(tState& eState, 
    const std::string& strParticipantName, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const

{
    if (0 >= tmTimeout)
    {
        return ERR_INVALID_ARG;
    }
    return _impl->m_pModule->GetStateMachine()->
            GetRemoteState(strParticipantName.c_str(), eState, tmTimeout);
}

fep::Result AutomationInterface::GetSystemState(tState& eState, 
    const std::vector<std::string>& vecParticipantList, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    if (0 >= tmTimeout)
    {
        return ERR_INVALID_ARG;
    }

    cSystemStateCollector oCollector(_impl->m_pModule);
    fep::Result nResult = oCollector.AskForStates(vecParticipantList, tmTimeout);

    if (fep::isOk(nResult) || nResult == fep::ERR_TIMEOUT)
    {
        eState = oCollector.GetAggregatedState();
    }
    else
    {
        eState = fep::FS_UNKNOWN;
    }

    return nResult;
}

fep::Result AutomationInterface::WaitForSystemState(tState eState, 
    const std::vector<std::string>& vecParticipantList, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    // A tmTimeout with value -1 would signal an (potentially) endless timeout
    if (tmTimeout < -1)
    {
        return ERR_INVALID_ARG;
    }

    cSystemStateCollector oCollector(_impl->m_pModule, true, eState);
    fep::Result nResult =  oCollector.AskForStates(vecParticipantList, tmTimeout);
    if (nResult != ERR_TIMEOUT && nResult != ERR_INVALID_ARG)
    {
        nResult = (eState == oCollector.GetAggregatedState()) ? nResult : fep::ERR_INVALID_STATE;
    }
    return nResult;
}

fep::Result AutomationInterface::GetParticipantsState(std::map<std::string, tState>& eStateMap, 
    const std::vector<std::string>& vecParticipantList, timestamp_t tmTimeout /*= FEP_AI_DEFAULT_TIMEOUT_MS*/) const
{
    if (0 >= tmTimeout)
    {
        return ERR_INVALID_ARG;
    }

    // the helper object
    cSystemStateCollector oCollector(_impl->m_pModule);
    fep::Result nResult = oCollector.AskForStates(vecParticipantList, tmTimeout);

    if (isOk(nResult) || nResult == fep::ERR_TIMEOUT)
    {
        eStateMap = oCollector.GetStates();
    }
    else
    {
        eStateMap.clear();
    }

    return nResult;
}

fep::Result fep::AutomationInterface::RegisterMonitoring(const std::string& strParticipantName, 
    IAutomationParticipantMonitor* pMonitor)
{
    return _impl->RegisterMonitoring(strParticipantName, pMonitor);
}

fep::Result fep::AutomationInterface::UnregisterMonitoring(IAutomationParticipantMonitor* pMonitor)
{
    return _impl->UnregisterMonitoring(pMonitor);
}

fep::Result fep::AutomationInterface::RegisterRPCObjectServer(const char* strServerInstance, IRPCObjectServer& oRPCObjectServerInstance)
{
    return _impl->m_pModule->GetRPC()->GetRegistry()->RegisterObjectServer(strServerInstance, oRPCObjectServerInstance);
}

fep::Result fep::AutomationInterface::UnregisterRPCObjectServer(const char* strServerInstance)
{
    return _impl->m_pModule->GetRPC()->GetRegistry()->UnregisterObjectServer(strServerInstance);
}

fep::IModule& fep::AutomationInterface::getModule()
{
    return *_impl->m_pModule;
}

fep::IRPC& fep::AutomationInterface::getInternalRPC()
{
    return *_impl->m_pModule->GetRPC();
}

void fep::AutomationInterface::SetShutdownHandler(const std::function<void()>& oShutdownHandler)
{
    _impl->m_pModule->SetShutdownHandler(oShutdownHandler);
}
