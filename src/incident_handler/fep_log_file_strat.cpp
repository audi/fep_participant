/**
 * Implementation of the Class cLogFileStrategy.
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

#define _CRT_SECURE_NO_WARNINGS // disable warning about fopen

#include <cstdio>
#include <list>
#include <a_util/filesystem/filesystem.h>
#include <a_util/result/result_type.h>
#include <a_util/system/system.h>

#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep_errors.h"
#include "fep_incident_handler_common.h"
#include "incident_handler/fep_log_file_strat.h"
#include "module/fep_module_intf.h"

using namespace fep;

/// Maximum length of an incident's log string dump .
static const uint16_t s_nMaxLogEntryLength = 512;

#define LOG_DEBUG(msg)
//#define LOG_DEBGU(msg) LOG_INFO(msg)

cLogFileStrategy::cLogFileStrategy() :
    m_pLogFile(NULL), m_bEnabled(false), m_bEnableCSVFormat(false),
    m_bOverwriteExisting(false), m_strLogString(s_nMaxLogEntryLength, '\0')
{
    // relevant since we may happen to log remote incidents to file.
    m_strHostname = a_util::system::getHostname();
}

cLogFileStrategy::~cLogFileStrategy()
{
    if (m_pLogFile)
    {
        ::fclose(m_pLogFile);
        m_pLogFile = NULL;
    }
}

std::string FormatOutput(bool isDebug, const int16_t nIncident,
                     const tSeverityLevel severity,
                     const char* strOrigin,
                     int nLine,
                     const char* strFile,const char* strDescription)
{
    std::string strLogString = std::string("");


    return strLogString;
}


fep::Result cLogFileStrategy::HandleLocalIncident(fep::IModule* pModuleContext,
                                              const int16_t nIncident,
                                              const tSeverityLevel severity,
                                              const char* strOrigin,
                                              int nLine,
                                              const char* strFile,
                                              const timestamp_t tmSimTime,
                                              const char* strDescription)
{
    fep::Result nResult = ERR_NOERROR;

    if (m_bEnabled && pModuleContext)
    {
        std::unique_lock<std::recursive_mutex> oSync(m_oConfigGuard);

        const char *strSource = pModuleContext->GetName();
        if (m_bEnabled && strSource)
        {
            if (m_pLogFile)
            {
                m_strLogString.clear();

                if (m_bEnableCSVFormat)
                {
                    formatIncidentStringCSV(m_strLogString, strSource, m_strHostname.c_str(), nIncident, severity, strOrigin, nLine, strFile,
                                            tmSimTime, strDescription);
                }
                else
                {
                    formatIncidentString(m_strLogString, strSource, m_strHostname.c_str(),nIncident, severity, strOrigin, nLine, strFile,
                                         tmSimTime, strDescription);
                }

                if (::fprintf(m_pLogFile, "%s", m_strLogString.c_str()) < 0)
                {
                    LOG_DEBUG("Unable to write log to file!");
                    nResult = ERR_DEVICE_IO;
                }
                else
                {
                    ::fflush(m_pLogFile);
                }
            }
            else
            {
                LOG_DEBUG("No valid file path configured for FEP log file strategy!");
                nResult = ERR_EMPTY;
            }
        }
    }

    return nResult;
}

fep::Result cLogFileStrategy::HandleGlobalIncident(const char *strSource,const int16_t nIncident,
                                               const tSeverityLevel severity,
                                               const timestamp_t tmSimTime,
                                               const char *strDescription)
{
    std::unique_lock<std::recursive_mutex> oSync(m_oConfigGuard);

    fep::Result nResult = ERR_NOERROR;

    if (m_bEnabled && strSource)
    {
        if (m_pLogFile)
        {
            m_strLogString.clear();

            if (m_bEnableCSVFormat)
            {
                formatIncidentStringCSV(m_strLogString,strSource, m_strHostname.c_str(),nIncident,severity,NULL,0,NULL,tmSimTime,strDescription);
            }
            else
            {
               formatIncidentString(m_strLogString,strSource, m_strHostname.c_str(),nIncident,severity,NULL,0,NULL,tmSimTime,strDescription);
            }

            nResult = ERR_NOERROR;
            if (::fprintf(m_pLogFile, "%s\n", m_strLogString.c_str()) < 0)
            {
                nResult = ERR_DEVICE_IO;
            }
            else
            {
                ::fflush(m_pLogFile);
            }

            if (nResult == ERR_DEVICE_IO)
            {
                LOG_DEBUG("Unable to write log to file!");
            }
        }
        else
        {
            LOG_DEBUG("No valid file path configured for FEP log file strategy!");
            nResult = ERR_EMPTY;
        }
    }

    return nResult;
}


fep::Result cLogFileStrategy::RefreshConfiguration(const fep::IProperty* pStrategyProperty,
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

        if (std::string(pAffectedProperty->GetPath()) == component_config::g_strIncidentFileLogBase)
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

        else if (std::string(pAffectedProperty->GetPath()) == component_config::g_strIncidentFileLogPath_bEnable)
        {
            nResult = pAffectedProperty->GetValue(m_bEnabled);
            if (m_bEnabled)
            {
                nResult = OpenLogFile(m_strLogFilePath);
            }
        }

        else if (std::string(pAffectedProperty->GetPath()) == component_config::g_strIncidentFileLogPath_bEnableCSV)
        {
            nResult = pAffectedProperty->GetValue(m_bEnableCSVFormat);
        }

        else if (std::string(pAffectedProperty->GetPath()) == component_config::g_strIncidentFileLogPath_bOverwriteExisting)
        {
            nResult = pAffectedProperty->GetValue(m_bOverwriteExisting);
            if (m_bEnabled)
            {
                nResult = OpenLogFile(m_strLogFilePath);
            }
        }

        else if (std::string(pAffectedProperty->GetPath()) == component_config::g_strIncidentFileLogPath_strPath)
        {
            const char* strPath = NULL;
            nResult = pAffectedProperty->GetValue(strPath);

            if (fep::isOk(nResult))
            {
                m_strLogFilePath = a_util::filesystem::Path(strPath);
                if (m_bEnabled)
                {
                    nResult = OpenLogFile(m_strLogFilePath);
                }
            }
        }
    }

    return nResult;
}

fep::Result cLogFileStrategy::OpenLogFile(const a_util::filesystem::Path &oPath)
{
    fep::Result nResult = ERR_NOERROR;

    if (oPath.isEmpty())
    {
        LOG_DEBUG("Invalid log file path provided for FEP file log strategy!");
        nResult = ERR_INVALID_ARG;
    }

    if (fep::isOk(nResult))
    {
        if (m_pLogFile)
        {
            ::fclose(m_pLogFile);
            m_pLogFile = NULL;
        }

        std::string flags = "wb";
        if (a_util::filesystem::exists(oPath) && !m_bOverwriteExisting)
        {
            flags = "ab";
        }

        m_pLogFile = ::fopen(oPath.toString().c_str(), flags.c_str());
        if (!m_pLogFile)
        {
            LOG_DEBUG(a_util::strings::format("Unable to open file %s for writing",
                                      oPath.ToString().c_str()).c_str());
            nResult = ERR_OPEN_FAILED;
        }
    }

    return nResult;
}
