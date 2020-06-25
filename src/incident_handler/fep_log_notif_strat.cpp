/**
 * Implementation of the Class cNotificationStrategy.
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
#include <a_util/result/result_type.h>

#include "_common/fep_timestamp.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep_errors.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_log_notif_strat.h"
#include "messages/fep_notification_access_intf.h"
#include "messages/fep_notification_incident.h"
#include "module/fep_module_intf.h"

using namespace fep;

#define LOG_DEBUG(str)
//#define LOG_DEBUG(str) LOG_INFO(str)

cNotificationStrategy::cNotificationStrategy()
{
    m_strTargetModuleName = "*";
}

cNotificationStrategy::~cNotificationStrategy()
{

}

fep::Result cNotificationStrategy::HandleLocalIncident(fep::IModule* pModuleContext,
                                                   const int16_t nIncident,
                                                   const fep::tSeverityLevel severity,
                                                   const char* strOrigin,
                                                   int nLine,
                                                   const char* strFile,
                                                   const timestamp_t tmSimTime,
                                                   const char* strDescription)
{
    // note: The IModule is convenient to have.... but DataAccess is protected and we
    // cannot access it from here. Instead we take our "hacked" detour over the internal
    // bus access

    std::unique_lock<std::recursive_mutex> oSync(m_oConfigGuard);

    fep::Result nResult = ERR_NOERROR;

    // Warning: To prevent endless loops and call stacks when the transmission adapter
    // is invoking incidents while this strategy is enabled:
    if (FSI_TRANSM_MSG_TX_FAILED == nIncident)
    {
        nResult = ERR_NOERROR;
    }
    else if (m_bEnabled || SL_Critical_Global == severity)
    {
        // in case the severity is of "global" scope, the notification will broadcast
        // it globally!
        if (SL_Critical_Global == severity)
        {
            fep::cIncidentNotification oLogNotification(nIncident,
                        strDescription, severity,
                        pModuleContext->GetName(), "*", GetTimeStampMicrosecondsUTC(),
                        pModuleContext->GetTimingInterface()->GetTime());

            LOG_DEBUG(a_util::strings::format("%s broadcast to %s: %d - %s",
                                     pModuleContext->GetName(), "*",
                                     nIncident, strDescription));

            nResult = pModuleContext->GetNotificationAccess()->TransmitNotification(
                &oLogNotification);
        }
        else
        {
            fep::cIncidentNotification oLogNotification(nIncident,
                        strDescription, severity,
                        pModuleContext->GetName(), m_strTargetModuleName.c_str(),
                        GetTimeStampMicrosecondsUTC(), pModuleContext->GetTimingInterface()->GetTime());

            LOG_DEBUG(a_util::strings::format("%s send to %s: %d - %s",
                                      pModuleContext->GetName(), m_strTargetModuleName.c_str(),
                                      nIncident, strDescription));

            nResult = pModuleContext->GetNotificationAccess()->TransmitNotification(
                &oLogNotification);
        }
        if (fep::isFailed(nResult))
        {
            LOG_DEBUG("Unable to transmit error notification!");
        }
    }

    return nResult;
}

fep::Result cNotificationStrategy::HandleGlobalIncident(const char *strSource,
                                                    const int16_t nIncident,
                                                    const tSeverityLevel severity,
                                                    const timestamp_t tmSimTime,
                                                    const char *strDescription)
{
    // No relevance since the FEP bus is a broadcast domain. No point in relaying
    // remote incidents.
    return ERR_NOERROR;
}

fep::Result cNotificationStrategy::RefreshConfiguration(
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

        if (std::string(pAffectedProperty->GetPath()) == component_config::g_strIncidentNotificationLogBase)
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

        else if (std::string(pAffectedProperty->GetPath()) == component_config::g_strIncidentNotificationLogPath_bEnable)
        {
            nResult = pAffectedProperty->GetValue(m_bEnabled);
        }

        else if (std::string(pAffectedProperty->GetPath()) == component_config::g_strIncidentNotificationLogPath_strTarget)
        {
            const char* strStrinValPtr = NULL;
            nResult = pAffectedProperty->GetValue(strStrinValPtr);
            if (strStrinValPtr && '\0' != *strStrinValPtr)
            {
                m_strTargetModuleName = strStrinValPtr;
            }
            else
            {
                LOG_DEBUG(a_util::strings::format("Notification Target Name invalid! Keeping "\
                                            "old one: \"%s\"",
                                            m_strTargetModuleName.c_str()).c_str());
            }
        }
    }

    return nResult;
}
