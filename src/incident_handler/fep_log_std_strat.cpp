/**
 * Implementation of the Class cLogConsoleStrategy.
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

#include <iostream>
#include <a_util/logging/log.h>
#include <a_util/result/result_type.h>
#include <a_util/system/system.h>

#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep_errors.h"
#include "fep_incident_handler_common.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "module/fep_module_intf.h"
#include "incident_handler/fep_log_std_strat.h"

using namespace fep;


cLogConsoleStrategy::cLogConsoleStrategy() :
    m_oLogMessageMgr(1000, 100, ENTRY_MESSAGE_LENGTH + 10),
    m_strLogString(ENTRY_MESSAGE_LENGTH, '\0'), m_bEnabled(true)
{
    // relevant since we may happen to log remote incidents to file.
    m_strHostname = a_util::system::getHostname();
}

cLogConsoleStrategy::~cLogConsoleStrategy()
{
}

fep::Result cLogConsoleStrategy::HandleLocalIncident(fep::IModule* pModuleContext,
                                                 const int16_t nIncident,
                                                 const  tSeverityLevel severity,
                                                 const char* strOrigin,
                                                 int nLine,
                                                 const char* strFile,
                                                 const timestamp_t tmSimTime,
                                                 const char* strDescription)
{
    fep::Result nResult = ERR_NOERROR;
    const char *strSource = pModuleContext->GetName();
    if (m_bEnabled)
    {
        if (!pModuleContext)
        {
            LOG_ERROR("::HandleIncident() called without FEP Module context (NULL)!");
            nResult = ERR_INVALID_ARG;
        }
        else if(!strSource)
        {
            LOG_ERROR("::HandleIncident() called without FEP Module name!");
            nResult = ERR_INVALID_ARG;
        }
        else
        {
            std::unique_lock<std::recursive_mutex> oSync(m_oLogGuard);
            m_strLogString.clear();
            nResult = formatIncidentString(m_strLogString, strSource, m_strHostname.c_str(),nIncident,
                                           severity, strOrigin, nLine, strFile,
                                           tmSimTime, strDescription);
            switch(severity)
            {
            default:
                std::cerr << "Invalid severity!" << std::endl;
                nResult = ERR_INVALID_ARG;
                break;
            case SL_Info:
                m_oLogMessageMgr.QueueConsoleLog(m_strLogString, RT::cLogMessageMgr::LOG_LVL_INFO);
                break;
            case SL_Warning:
                m_oLogMessageMgr.QueueConsoleLog(m_strLogString, RT::cLogMessageMgr::LOG_LVL_WARNING);
                break;
            case SL_Critical:
                m_oLogMessageMgr.QueueConsoleLog(m_strLogString, RT::cLogMessageMgr::LOG_LVL_ERROR);
                break;
            }
        }
    }

    return nResult;
}

fep::Result cLogConsoleStrategy::HandleGlobalIncident(const char *strSource,
                                                  const int16_t nIncident,
                                                  const tSeverityLevel severity,
                                                  const timestamp_t tmSimTime,
                                                  const char *strDescription)
{
    fep::Result nResult = ERR_NOERROR;

    if (m_bEnabled)
    {
        if (!strSource)
        {
            LOG_ERROR("::HandleIncident() called without FEP Module name!");
            nResult = ERR_INVALID_ARG;
        }
        else
        {
            std::unique_lock<std::recursive_mutex> oSync(m_oLogGuard);
            m_strLogString.clear();
            // Note: This implementation resides within the shared fep library which will have
            // and encapsulate it's own symbols. Therefore, the fep_utils/adtf_utils cLog will
            // NOT be interchangeable when run inside the context of an ADTF component
            // (e.g. filter or service). For this to work it would need the ucom:: namepsace
            // to be integrated and used within FEP - which is of no use! Any LOG_XYZ will
            // therefore only be visible on stdout even without an ADTF running. To be able
            // to log to the ADTF console, it is documented that the notification strategy should
            // be used instead whilst having the fep_log_collector filter in the active ADTF
            // configuration.

            // Note as well: Using stdout here instead of LOG_XY. First, it is no no help
            // for the reasons above, secondly it only adds this fep_log_std_strat.cpp file
            // with the respective line as the source of the log message to the log string -
            // which is of absolutely no help either....
            nResult = formatIncidentString(m_strLogString, strSource, m_strHostname.c_str(),nIncident,
                                           severity, NULL, 0, NULL,
                                           tmSimTime, strDescription);

            switch(severity)
            {
            default:
                std::cerr << "Invalid severity!" << std::endl;
                nResult = ERR_INVALID_ARG;
                break;
            case SL_Info:
                m_oLogMessageMgr.QueueConsoleLog(m_strLogString, RT::cLogMessageMgr::LOG_LVL_INFO);
                break;
            case SL_Warning:
                m_oLogMessageMgr.QueueConsoleLog(m_strLogString, RT::cLogMessageMgr::LOG_LVL_WARNING);
                break;
            case SL_Critical:
                m_oLogMessageMgr.QueueConsoleLog(m_strLogString, RT::cLogMessageMgr::LOG_LVL_ERROR);
                break;
            }
        }
    }

    return nResult;
}

fep::Result cLogConsoleStrategy::RefreshConfiguration(
        const fep::IProperty* pStrategyProperty,
        const fep::IProperty* pAffectedProperty)
{
    fep::Result nResult = ERR_NOERROR;

    if (!pStrategyProperty || !pAffectedProperty)
    {
        nResult = ERR_POINTER;
    }

    if (fep::isOk(nResult))
    {
        if (std::string(pAffectedProperty->GetPath()) == component_config::g_strIncidentConsoleLogPath_bEnable)
        {
            nResult = pAffectedProperty->GetValue(m_bEnabled);
        }
    }

    return nResult;
}
