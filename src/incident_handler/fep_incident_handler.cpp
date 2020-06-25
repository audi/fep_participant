/**
 * Implementation of the Class cIncidentHandler.
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
#include <cstdint>
#include <utility>
#include <a_util/strings/strings_format.h>
#include <a_util/strings/strings_functions.h>

#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep_incident_handler_common.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_log_file_strat.h"
#include "incident_handler/fep_log_history_strat.h"
#include "incident_handler/fep_log_notif_strat.h"
#include "incident_handler/fep_log_std_strat.h"
#include "messages/fep_notification_access_intf.h"
#include "messages/fep_notification_incident_intf.h"
#include "module/fep_module_intf.h"
#include "incident_handler/fep_incident_handler.h"

using namespace fep;

#define LOG_DEBUG(str)
//#define LOG_DEBUG(str) LOG_INFO(str)

static fep::Result ConfigureDefaultsInternal(IPropertyTree* pPropertyTree,
                                  cIncidentHandler* pHandler)
{
    fep::Result nResult = ERR_NOERROR;

    if (!pPropertyTree || !pHandler)
    {
        nResult = ERR_INVALID_ARG;
    }

    // first of all, subscribe to the root to be able to process the added properties
    // "online".
    if (fep::isOk(nResult))
    {
        // no registration without existing root
        nResult = pPropertyTree->SetPropertyValue(
                    component_config::g_strIncidentHandlerBase, "");
    }

    if (fep::isOk(nResult))
    {
        nResult = pPropertyTree->RegisterListener(
                    component_config::g_strIncidentHandlerBase, pHandler);
    }

    if (fep::isOk(nResult))
    {
        nResult = pHandler->SetDefaultProperties();
    }

    return nResult;
}

cIncidentHandler::cIncidentHandler() : m_pFEPModule(NULL),
m_pPropertyTree(NULL)
{
}

#define DELETE_POINTER(pointer) \
    if (pointer) \
    { \
        delete pointer; \
        pointer = NULL; \
    }

cIncidentHandler::~cIncidentHandler()
{
}

fep::Result cIncidentHandler::SetModule(fep::IModule * pModule)
{
    fep::Result nResult = ERR_NOERROR;
    if (m_pFEPModule != pModule)
    {
        if (NULL != m_pFEPModule)
        {
            std::unique_lock<std::recursive_mutex> oSync(m_oAssocMutex);

            // clear property tree & listener
            m_pFEPModule->GetPropertyTree()->UnregisterListener(
                component_config::g_strIncidentHandlerBase, this);
            m_pFEPModule->GetPropertyTree()->SetPropertyValue(
                component_config::g_strIncidentHandlerBase, "");
            m_pPropertyTree = m_pFEPModule->GetPropertyTree();

            // clear strategies
            m_mapIncidentDelegates.clear();
            m_mapCatchAllDelegates.clear();
            // none of the individual assigned strategies are not owned by this class.
            tGlobalStrategyIter itGlobalStrat = m_setGlobalStrategies.begin();
            for (; itGlobalStrat != m_setGlobalStrategies.end(); itGlobalStrat++)
            {
                delete *itGlobalStrat;
            }
            m_setGlobalStrategies.clear();
            
            m_pFEPModule->GetNotificationAccess()->UnregisterNotificationListener(this);

            DELETE_POINTER(m_pLogStratRef);
            DELETE_POINTER(m_pFileStratRef);
            DELETE_POINTER(m_pHistoryStratRef);
            DELETE_POINTER(m_pNotifStratRef);
        }

        m_pFEPModule = pModule;

        if (NULL == m_pFEPModule)
        {
            m_pPropertyTree = NULL;
        }

        if (NULL != m_pFEPModule)
        {
            // creating all the default catch-all strategies.
            // These will always be available and are configurable through the
            // respective module's property tree.
            m_pLogStratRef = new cLogConsoleStrategy();
            m_pFileStratRef = new cLogFileStrategy();
            m_pHistoryStratRef = new cIncidentHistoryStrategy();
            m_pNotifStratRef = new cNotificationStrategy();

            m_pPropertyTree = m_pFEPModule->GetPropertyTree();

            m_bEnabled = true;
            m_bEnableGlobalScope = false;

            // init configuration
            nResult |= AssociateCatchAllStrategy(m_pLogStratRef,
                component_config::g_strIncidentConsoleLogBase);
            nResult |= AssociateCatchAllStrategy(m_pFileStratRef,
                component_config::g_strIncidentFileLogBase);
            nResult |= AssociateCatchAllStrategy(m_pHistoryStratRef,
                component_config::g_strIncidentHistoryLogBase);
            nResult |= AssociateCatchAllStrategy(m_pNotifStratRef,
                component_config::g_strIncidentNotificationLogBase);
            nResult |= ConfigureDefaultsInternal(m_pFEPModule->GetPropertyTree(), this);
            nResult |= RefreshConfiguration();

            m_pFEPModule->GetNotificationAccess()->RegisterNotificationListener(this);
        }
    }
    if (fep::isFailed(nResult))
    {
        return ERR_FAILED;
    }
    return ERR_NOERROR;
}

fep::Result cIncidentHandler::AssociateStrategy(const int16_t eFEPIncident,
                                               const tIncidentStrategy eStrategyDelegate,
                                               const tStrategyAssociation eAssociation)
{
    fep::Result nResult = ERR_NOERROR;

    assert(NULL != m_pFileStratRef);
    assert(NULL != m_pLogStratRef);
    assert(NULL != m_pHistoryStratRef);
    assert(NULL != m_pNotifStratRef);

    switch(eStrategyDelegate)
    {
    case ES_LogFile:
        nResult = AssociateStrategy(eFEPIncident, m_pFileStratRef,
                                    component_config::g_strIncidentFileLogBase, eAssociation);
        break;
    case ES_LogConsole:
        nResult = AssociateStrategy(eFEPIncident, m_pLogStratRef,
                                    component_config::g_strIncidentConsoleLogBase, eAssociation);
        break;
    case ES_LogHistory:
        nResult = AssociateStrategy(eFEPIncident, m_pHistoryStratRef,
                                    component_config::g_strIncidentHistoryLogBase, eAssociation);
        break;
    case ES_LogNotification:
        nResult = AssociateStrategy(eFEPIncident, m_pNotifStratRef,
                                    component_config::g_strIncidentNotificationLogBase, eAssociation);
        break;
    default:
        nResult = ERR_NOT_SUPPORTED;
    }

    return  nResult;
}


fep::Result cIncidentHandler::AssociateStrategy(const int16_t nFEPIncident,
                                               fep::IIncidentStrategy* pStrategyDelegate,
                                               const char* strConfigurationPath,
                                               const tStrategyAssociation eAssociation)
{
    std::unique_lock<std::recursive_mutex> oSync(m_oAssocMutex);

    fep::Result nResult = ERR_NOERROR;
    if (!m_pFEPModule)
    {
        nResult = ERR_NOT_INITIALISED;
    }

    if (fep::isOk(nResult) && !pStrategyDelegate)
    {
        nResult = ERR_POINTER;
    }

    if (fep::isOk(nResult) && !strConfigurationPath)
    {
        nResult = ERR_INVALID_ARG;
    }

    if (fep::isOk(nResult) && 0 == nFEPIncident)
    {
        nResult = ERR_INVALID_INDEX;
    }

    if (fep::isOk(nResult))
    {
        std::pair<tStrategyMapIter, tStrategyMapIter> pairStrategies;
        pairStrategies = m_mapIncidentDelegates.equal_range(nFEPIncident);
        tStrategyMapIter itStrategy = pairStrategies.first;
        for (; itStrategy != pairStrategies.second && fep::isOk(nResult);)
        {
            if (eAssociation == SA_APPEND)
            {
                if (itStrategy->second == pStrategyDelegate &&
                        itStrategy->first == nFEPIncident)
                {
                    nResult = ERR_RESOURCE_IN_USE;
                }
                ++itStrategy;
            }
            else // SA_REPLACE
            {
                tStrategyMapIter itToBeErased = itStrategy;
                ++itStrategy;
                m_mapIncidentDelegates.erase(itToBeErased);
            }
        }
    }

    if (fep::isOk(nResult))
    {
#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic ignored "-Wattributes" // standard type attributes are ignored when used in templates
#endif

        m_mapIncidentDelegates.insert(
                    std::pair<const int16_t, IIncidentStrategy*>(nFEPIncident, pStrategyDelegate));

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic warning "-Wattributes" // standard type attributes are ignored when used in templates
#endif
       // preparing and subscribing to the requested configuration path (if not existing yet)
       IProperty* pProperty =
               m_pFEPModule->GetPropertyTree()->GetLocalProperty(strConfigurationPath);
       if (!pProperty)
       {
           nResult = m_pFEPModule->GetPropertyTree()->SetPropertyValue(strConfigurationPath, "");
       }

       if (fep::isOk(nResult))
       {
           pProperty = m_pFEPModule->GetPropertyTree()->GetLocalProperty(strConfigurationPath);

           // make sure, the strategy is properly configured - even if the branch has already
           // been configured and does not issue change/add callbacks.
           nResult = pStrategyDelegate->RefreshConfiguration(pProperty, pProperty);
           if (fep::isFailed(nResult))
           {
                INVOKE_INCIDENT(FSI_INCIDENT_CONFIG_FAILED, SL_Warning,
                              a_util::strings::format(
                                  "Unable to configure incident strategy; "\
                                  "RefreshConfiguration() returned with error %d",
                                  nResult.getErrorCode()).c_str());
           }
       }

       if (fep::isOk(nResult))
       {
           // eventually insert strategies here if not yet known.
           cStrategyConfigWrapper* pConfigWrapperRef = NULL;
           if (fep::isFailed(GetConfigWrapperReference(pStrategyDelegate, &pConfigWrapperRef)))
           {
               m_setGlobalStrategies.insert(
                           new cStrategyConfigWrapper(pStrategyDelegate, pProperty));
           }
       }
    }

    return nResult;
}

fep::Result cIncidentHandler::AssociateCatchAllStrategy(IIncidentStrategy *pStrategyDelegate,
                                                       const char* strConfigurationPath,
                                                       const tStrategyAssociation eAssociation)
{
    std::unique_lock<std::recursive_mutex> oSync(m_oAssocMutex);

    fep::Result nResult = ERR_NOERROR;
    if (!m_pFEPModule)
    {
        nResult = ERR_NOT_INITIALISED;
    }

    if (!pStrategyDelegate)
    {
        nResult = ERR_POINTER;
    }

    if (!strConfigurationPath)
    {
        nResult = ERR_INVALID_ARG;
    }

    // note: Whilst integrated catch-all strategies are being configured through
    // the property tree path of the incident handler itself, the handler itself
    // may or may not enable or disable these strategies. If custom catch-all strategies
    // are being implemented, these have to be configured in their own way, including
    // registering and unregistering property listeners to their respective property paths.
    /// FIXME: "make this homogenous! Use console logger as example for users")

    if (fep::isOk(nResult))
    {
        if (eAssociation == SA_APPEND)
        {
            if (m_mapCatchAllDelegates.find(pStrategyDelegate) == m_mapCatchAllDelegates.end())
            {
                m_mapCatchAllDelegates.insert(
                            std::pair<IIncidentStrategy*, bool>(pStrategyDelegate, true));
            }
            else
            {
                nResult = ERR_RESOURCE_IN_USE;
            }
        }
        else //SA_REPLACE
        {
            // replacing all catchall strategies.
            m_mapCatchAllDelegates.clear();
            m_mapCatchAllDelegates.insert(
                        std::pair<IIncidentStrategy*, bool>(pStrategyDelegate, true));
        }
    }

    if (fep::isOk(nResult))
    {
        // preparing and subscribing to the requested configuration path (if not existing yet)
        IProperty* pProperty =
                m_pFEPModule->GetPropertyTree()->GetLocalProperty(strConfigurationPath);
        if (!pProperty)
        {
            nResult = m_pFEPModule->GetPropertyTree()->SetPropertyValue(strConfigurationPath, "");
        }

        if (fep::isOk(nResult))
        {
            pProperty = m_pFEPModule->GetPropertyTree()->GetLocalProperty(strConfigurationPath);

            // make sure, the strategy is properly configured - even if the branch has already
            // been configured and does not issue change/add callbacks.
            nResult = pStrategyDelegate->RefreshConfiguration(pProperty, pProperty);
            if (fep::isFailed(nResult))
            {
                INVOKE_INCIDENT(FSI_INCIDENT_CONFIG_FAILED, SL_Warning,
                               a_util::strings::format(
                                   "Unable to configure incident strategy; "\
                                   "RefreshConfiguration() returned with error %d",
                                   nResult.getErrorCode()).c_str());
            }
        }

        if (fep::isOk(nResult))
        {
            // eventually insert strategies here if not already known...
            cStrategyConfigWrapper* pConfigWrapperRef = NULL;
            if (fep::isFailed(GetConfigWrapperReference(pStrategyDelegate, &pConfigWrapperRef)))
            {
                m_setGlobalStrategies.insert(new cStrategyConfigWrapper(pStrategyDelegate, pProperty));
            }
        }
    }

    return  nResult;
}

fep::Result cIncidentHandler::DisassociateStrategy(const int16_t eFEPIncident,
                                                  fep::IIncidentStrategy* pStrategyDelegate)
{
    std::unique_lock<std::recursive_mutex> oSync(m_oAssocMutex);

    fep::Result nResult = ERR_NOERROR;

    if (!pStrategyDelegate)
    {
        nResult = ERR_POINTER;
    }

    if (fep::isOk(nResult))
    {
        nResult = ERR_NOT_FOUND;
        std::pair<tStrategyMapIter, tStrategyMapIter> pairStrategies;
        pairStrategies = m_mapIncidentDelegates.equal_range(eFEPIncident);
        tStrategyMapIter itStrategy = pairStrategies.first;
        for (; itStrategy != pairStrategies.second;)
        {
            if (itStrategy->second == pStrategyDelegate)
            {
                tStrategyMapIter itToBeErased = itStrategy;
                itStrategy++;
                m_mapIncidentDelegates.erase(itToBeErased);
                nResult = ERR_NOERROR;
            }
            else
            {
                itStrategy++;
            }
        }

        // eventually check, if this was the last association; if so, remove the strategy from
        // the list of global strategies.
        cStrategyConfigWrapper* pConfigWrapperRef = NULL;
        if (!StrategyIsAssociatedAnywhere(pStrategyDelegate, &pConfigWrapperRef) &&
                pConfigWrapperRef)
        {
            // note: pConfigWrapper may as well be NULL if a "double-erase" would happen
            m_setGlobalStrategies.erase(pConfigWrapperRef);
            if (pConfigWrapperRef)
            {
                delete pConfigWrapperRef;
                pConfigWrapperRef = NULL;
            }
        }
    }

    return  nResult;
}

fep::Result cIncidentHandler::DisassociateCatchAllStrategy(IIncidentStrategy *pStrategyDelegate)
{
    std::unique_lock<std::recursive_mutex> oSync(m_oAssocMutex);

    fep::Result nResult = ERR_NOERROR;

    if (!pStrategyDelegate)
    {
        nResult = ERR_POINTER;
    }

    if (fep::isOk(nResult))
    {
        tCAStrategyMapIter itStrategy = m_mapCatchAllDelegates.find(pStrategyDelegate);
        if (itStrategy != m_mapCatchAllDelegates.end())
        {
            m_mapCatchAllDelegates.erase(itStrategy);
        }
        else
        {
            nResult = ERR_NOT_FOUND;
        }
    }

    // eventually check, if this was the last association; if so, remove the strategy from
    // the list of global strategies.
    cStrategyConfigWrapper* pConfigWrapperRef = NULL;
    if (!StrategyIsAssociatedAnywhere(pStrategyDelegate, &pConfigWrapperRef) &&
            pConfigWrapperRef)
    {
        // note: pConfigWrapper may as well be NULL if a "double-erase" would happen
        m_setGlobalStrategies.erase(pConfigWrapperRef);
        if (pConfigWrapperRef)
        {
            delete pConfigWrapperRef;
            pConfigWrapperRef = NULL;
        }
    }
    return  nResult;
}

fep::Result cIncidentHandler::DisassociateStrategy(const int16_t nFEPIncident,
                                                  const tIncidentStrategy eStrategyDelegate)
{
    std::unique_lock<std::recursive_mutex> oSync(m_oAssocMutex);

    fep::Result nResult = ERR_NOERROR;

    IIncidentStrategy* pInternalStratRef = NULL;

    switch(eStrategyDelegate)
    {
    case ES_LogFile:
        pInternalStratRef = m_pFileStratRef;
        break;
    case ES_LogConsole:
        pInternalStratRef = m_pLogStratRef;
        break;
    case ES_LogHistory:
        pInternalStratRef = m_pHistoryStratRef;
        break;
    case ES_LogNotification:
        pInternalStratRef = m_pNotifStratRef;
        break;
    default:
        pInternalStratRef = NULL;
    }

    if (!pInternalStratRef)
    {
        nResult = ERR_INVALID_ARG;
    }

    if (fep::isOk(nResult))
    {
        nResult = DisassociateStrategy(nFEPIncident, pInternalStratRef);
    }

    return nResult;
}

// Remote incident handling!
fep::Result cIncidentHandler::ForwardIncident(int16_t nIncidentCode, tSeverityLevel severity,
                                            const char *strSource,
                                            const timestamp_t tmSimTime,
                                            const char *strDescription)
{
    std::unique_lock<std::recursive_mutex> oSync(m_oAssocMutex);
    std::unique_lock<std::recursive_mutex> oSync2(m_oPropConfigMutex);

    fep::Result nResult = ERR_NOERROR;
    if (!strSource)
    {
        nResult = ERR_INVALID_ARG;
    }

    if (fep::isOk(nResult) && !m_bEnabled)
    {
        nResult = ERR_NOT_READY;
    }

    if (fep::isOk(nResult) && 0 == nIncidentCode)
    {
        nResult = ERR_INVALID_ARG;
    }

    if (fep::isOk(nResult))
    {
        tCAStrategyMapIter itCAStrategy = m_mapCatchAllDelegates.begin();
        for (;itCAStrategy != m_mapCatchAllDelegates.end(); itCAStrategy++)
        {
            if (itCAStrategy->second)
            {
                // well, there is no point in evaluating the return code here..
                itCAStrategy->first->HandleGlobalIncident(strSource, nIncidentCode,
                                                          severity,tmSimTime, strDescription);
            }
        }

        std::pair<tStrategyMapIter, tStrategyMapIter> pairStrategies;
        pairStrategies = m_mapIncidentDelegates.equal_range(nIncidentCode);
        tStrategyMapIter itStrategy = pairStrategies.first;
        for (; itStrategy != pairStrategies.second; ++itStrategy)
        {
            // prevent (active!) catch-all delegates from being called again here...
            if (m_mapCatchAllDelegates.find(itStrategy->second) ==
                m_mapCatchAllDelegates.end() ||
                (false == m_mapCatchAllDelegates.find(itStrategy->second)->second))
            {
                // well, there is no point in evaluating the return code here..
                itStrategy->second->HandleGlobalIncident(strSource, nIncidentCode,
                                                         severity,tmSimTime, strDescription);
            }
        }
    }

    return nResult;
}

// Local incident handling!
fep::Result cIncidentHandler::InvokeIncident(int16_t nIncidentCode,
                                            tSeverityLevel severity,
                                            const char *strDescription,
                                            const char *strOrigin,
                                            int nLine,
                                            const char *strFile)
{   
    std::unique_lock<std::recursive_mutex> oSync(m_oAssocMutex);
    std::unique_lock<std::recursive_mutex> oSync2(m_oPropConfigMutex);

    fep::Result nResult = ERR_NOERROR;
    if (!m_pFEPModule)
    {
        nResult = ERR_NOT_INITIALISED;
    }   

    if (fep::isOk(nResult) && !m_bEnabled)
    {
        nResult = ERR_NOT_READY;
    }

    if (fep::isOk(nResult) && 0 == nIncidentCode)
    {
        nResult = ERR_INVALID_ARG;
    }
    // not beautiful but does the job in case of global criticals.
    // result: For global scope criticals, the user must only enable the incident
    // handler. That's it. No separate enabling of the notification strategy and
    // catchall association thereof as separate steps!

    bool bIsGlobalCritical = (SL_Critical_Global == severity);
    bool bCriticalHandledByNotification = false;
    if (fep::isOk(nResult))
    {   
        timestamp_t tmSimTime = m_pFEPModule->GetTimingInterface()->GetTime();
        tCAStrategyMapIter itCAStrategy = m_mapCatchAllDelegates.begin();
        for (;itCAStrategy != m_mapCatchAllDelegates.end(); itCAStrategy++)
        {
            if (itCAStrategy->second)
            {
                // well, there is no point in evaluating the return code here..
                itCAStrategy->first->HandleLocalIncident(m_pFEPModule, nIncidentCode,
                                                    severity,strOrigin,nLine,strFile,
                                                    tmSimTime,
                                                    strDescription);

                bCriticalHandledByNotification = (bCriticalHandledByNotification ||
                                                  (m_pNotifStratRef == itCAStrategy->first));
            }
        }

        std::pair<tStrategyMapIter, tStrategyMapIter> pairStrategies;
        pairStrategies = m_mapIncidentDelegates.equal_range(nIncidentCode);
        tStrategyMapIter itStrategy = pairStrategies.first;
        for (; itStrategy != pairStrategies.second; ++itStrategy)
        {
            // prevent (active!) catch-all delegates from being called again here...
            if (m_mapCatchAllDelegates.find(itStrategy->second) ==
                m_mapCatchAllDelegates.end() ||
                (false == m_mapCatchAllDelegates.find(itStrategy->second)->second))
            {
                // well, there is no point in evaluating the return code here..
                itStrategy->second->HandleLocalIncident(m_pFEPModule, nIncidentCode,
                                                        severity, strOrigin, nLine,
                                                        strFile,
                                                        tmSimTime,
                                                        strDescription);
                bCriticalHandledByNotification =
                        (bCriticalHandledByNotification || (m_pNotifStratRef == itStrategy->second));
            }
        }

        // if a global critical is not yet handled by the notification strategy
        // deal with it separately.
        if (bIsGlobalCritical && !bCriticalHandledByNotification && m_pNotifStratRef)
        {
            m_pNotifStratRef->HandleLocalIncident(m_pFEPModule, nIncidentCode,
                                                  severity, strOrigin, nLine,
                                                  strFile,
                                                  tmSimTime,
                                                  strDescription);
        }
    }

    return nResult;
}


fep::Result cIncidentHandler::GetLastIncident(const fep::tIncidentEntry** ppIncidentEntry)
{
    return m_pHistoryStratRef->GetLastIncident(ppIncidentEntry);
}

fep::Result cIncidentHandler::RetrieveIncidentHistory(tIncidentListConstIter& io_iterHistBegin,
                                                     tIncidentListConstIter& io_iterHistEnd)
{
    return m_pHistoryStratRef->LockHistory(io_iterHistBegin, io_iterHistEnd);
}

fep::Result cIncidentHandler::FreeIncidentHistory()
{
    return m_pHistoryStratRef->UnlockHistory();
}

fep::Result cIncidentHandler::RefreshConfiguration()
{
    std::unique_lock<std::recursive_mutex> oSync(m_oAssocMutex);

    fep::Result nResult = ERR_NOERROR;

    if (!m_pFEPModule)
    {
        nResult = ERR_NOT_INITIALISED;
    }

    if (fep::isOk(nResult))
    {
        tGlobalStrategyIter itGlobalStrategy = m_setGlobalStrategies.begin();
        for (; itGlobalStrategy != m_setGlobalStrategies.end(); ++itGlobalStrategy)
        {
            IProperty* pStratConfig = (*itGlobalStrategy)->GetStrategyConfig();
            IIncidentStrategy* pStrat = (*itGlobalStrategy)->GetWrappedStrategy();
            if (fep::isFailed(pStrat->RefreshConfiguration(pStratConfig, pStratConfig)))
            {
                nResult = ERR_FAILED;
            }
        }
    }

    return nResult;
}

bool cIncidentHandler::StrategyIsAssociatedAnywhere(IIncidentStrategy *pStrategyHandle,
                                                cStrategyConfigWrapper** pConfigWrapper)
{
    bool m_bIsAssociated = false;

    tCAStrategyMapIter itCAStrategy = m_mapCatchAllDelegates.begin();
    for (;itCAStrategy != m_mapCatchAllDelegates.end() && !m_bIsAssociated; itCAStrategy++)
    {
        if (itCAStrategy->first == pStrategyHandle)
        {
            m_bIsAssociated = true;
        }
    }

    tStrategyMapIter itStrategy = m_mapIncidentDelegates.begin();
    for (;itStrategy != m_mapIncidentDelegates.end() && !m_bIsAssociated; itStrategy++)
    {
        if (itStrategy->second == pStrategyHandle)
        {
             m_bIsAssociated = true;
        }
    }

    GetConfigWrapperReference(pStrategyHandle, pConfigWrapper);

    return m_bIsAssociated;
}

fep::Result cIncidentHandler::GetConfigWrapperReference(IIncidentStrategy *pStrategyHandle,
                                                       cStrategyConfigWrapper **pConfigWrapper)
{
    fep::Result nResult = ERR_NOT_FOUND;

    // for internal use only; assuming sober FEP developers ;)
    assert(NULL != pConfigWrapper);

    // now to find the config wrapper reference to be able to remove the strategy for good.
    tGlobalStrategyIter itGlobalStrat = m_setGlobalStrategies.begin();
    *pConfigWrapper = NULL;
    for (; itGlobalStrat != m_setGlobalStrategies.end(); itGlobalStrat++)
    {
        if (pStrategyHandle == (*itGlobalStrat)->GetWrappedStrategy())
        {
            *pConfigWrapper = *itGlobalStrat;
            nResult = ERR_NOERROR;
            break;
        }
    }

    return nResult;
}



fep::Result cIncidentHandler::ProcessPropertyChange(const IProperty *poProperty,
                                                   IProperty const * poAffectedProperty,
                                                   char const * strRelativePath)
{
    std::unique_lock<std::recursive_mutex> oSync(m_oPropConfigMutex);

    fep::Result nResult = ERR_NOERROR;

    if (!poProperty)
    {
        nResult = ERR_POINTER;
    }

    if (fep::isOk(nResult))
    {
        std::string strPropFullPath = std::string(poAffectedProperty->GetPath());

        if (strPropFullPath == component_config::g_strIncidentHandlerPath_bEnable)
        {
            nResult = poAffectedProperty->GetValue(m_bEnabled);
        }
        else if (strPropFullPath == component_config::g_strIncidentHandlerPath_bEnableGlobalScope)
        {
            nResult = poAffectedProperty->GetValue(m_bEnableGlobalScope);
        }
        else if (strPropFullPath == component_config::g_strIncidentHandlerPath_strSourceFilter)
        {
            const char * strVal = NULL;
            nResult = poAffectedProperty->GetValue(strVal);

            if (fep::isOk(nResult))
            {
                m_strFilterGlobalSource = strVal;
                a_util::strings::replace(m_strFilterGlobalSource, "*", ".*"); // making it PRE compatible
                m_oSourceMatcher.setPattern(m_strFilterGlobalSource);
            }
        }

        // since the handler itself is responsible for the catch-all mechanism
        // the following properties need to be evaluated here. The strategy itself
        // cannot determine which context it is currently been called in.

        else if (strPropFullPath == component_config::g_strIncidentConsoleLogPath_bEnableCatchAll)
        {
            nResult = poAffectedProperty->GetValue(
                        m_mapCatchAllDelegates.find(m_pLogStratRef)->second);

        }
        else if (strPropFullPath == component_config::g_strIncidentHistoryLogPath_bEnableCatchAll)
        {
            nResult = poAffectedProperty->GetValue(
                        m_mapCatchAllDelegates.find(m_pHistoryStratRef)->second);
        }
        else if (strPropFullPath == component_config::g_strIncidentFileLogPath_bEnableCatchAll)
        {
            nResult = poAffectedProperty->GetValue(
                        m_mapCatchAllDelegates.find(m_pFileStratRef)->second);
        }
        else if (strPropFullPath == component_config::g_strIncidentNotificationLogPath_bEnableCatchAll)
        {
            nResult = poAffectedProperty->GetValue(
                            m_mapCatchAllDelegates.find(m_pNotifStratRef)->second);
        }

        // in any other case, forwarding the property change to the associated strategies
        // is not necessary; each strategy has it's very own property notification wrapper.
    }

    return nResult;
}

fep::Result cIncidentHandler::ProcessPropertyDelete(const IProperty *poProperty,
    IProperty const * poAffectedProperty, char const * strRelativePath)
{
    // does not really apply.
    return ERR_NOERROR;
}

fep::Result cIncidentHandler::ProcessPropertyAdd(const IProperty *poProperty,
    IProperty const * poAffectedProperty, char const * strRelativePath)
{
    // consider all propeties added to this as "changes" which need to be
    // re-read. This helps with the initial defaults (see above)
    return ProcessPropertyChange(poProperty, poAffectedProperty, strRelativePath);
}

fep::Result cIncidentHandler::Update(
        const fep::IIncidentNotification *pLogNotification)
{
    fep::Result nResult;
    if (NULL == m_pFEPModule)
    {
        nResult = ERR_POINTER;
    }

    // Support for remote incidents, issued by other modules.
    if (m_bEnableGlobalScope && pLogNotification && fep::isOk(nResult))
    {
        if (m_oSourceMatcher.fullMatch(pLogNotification->GetSender()))
        {
            LOG_DEBUG(a_util::strings::format("%s received from %s: %d - %s",
                                      m_pFEPModule->GetName(),
                                      pLogNotification->GetSender(),
                                      pLogNotification->GetIncidentCode(),
                                      pLogNotification->GetDescription()));

            nResult = ForwardIncident(pLogNotification->GetIncidentCode(),
                                     pLogNotification->GetSeverity(),
                                     pLogNotification->GetSender(),
                                     pLogNotification->GetSimulationTime(),
                                     pLogNotification->GetDescription());
        }
    }

    return nResult;
}

fep::Result cIncidentHandler::SetDefaultProperties()
{
    if (m_pPropertyTree == NULL)
    {
        return ERR_NOT_INITIALISED;
    }
    fep::Result nResult = ERR_NOERROR, nFinalResult = ERR_NOERROR;
    // this is always enabled by default - obviously
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentHandlerPath_bEnable, true);
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;
    // yet not using a global scope, e.g. collecting logs form external
    // modules. Use-case: Test-Evaluation Module, Master Module.
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentHandlerPath_bEnableGlobalScope, false);
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;
    // If enabled, accept all source modules by default
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentHandlerPath_strSourceFilter, "*");
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;

    // Enable the console logger by default - convenient for debugging purposes
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentConsoleLogPath_bEnable, true);
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;
    // Also, catch all local (and locally emitted, global) incidents.
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentConsoleLogPath_bEnableCatchAll, true);
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;

    // Disable the log history by default
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_bEnable, false);
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_bEnableCatchAll, false);
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentHistoryLogPath_nQueueSize,
        static_cast<int32_t>(500));
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;

    // Disable the file log by default (where to log to by default anyway?)
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_bEnable, false);
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_bEnableCSV, false);
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_bEnableCatchAll, false);
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_strPath, "");
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentFileLogPath_bOverwriteExisting, false);
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;

    // Enable propagation of global scope incidents
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentNotificationLogPath_bEnable, false);
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentNotificationLogPath_bEnableCatchAll, false);
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;
    nResult = m_pPropertyTree->SetPropertyValue(
        component_config::g_strIncidentNotificationLogPath_strTarget, "*");
    nFinalResult = fep::isFailed(nResult) ? nResult : nFinalResult;

    return nFinalResult;
}
