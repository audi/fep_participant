/************************************************************************
 * Implementation of the remote properties element
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
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <string.h>
#include <fep_participant_sdk.h>
#include "remote_properties.h"

// Helper function that converts any trivial value into a std::string
template<typename T>
std::string ToString(T value)
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

// Helper function that formats a property value into a string
static void FormatValue(const fep::IProperty* pProp, std::string& strDest)
{
    if (pProp->IsArray())
    {
        strDest.append("{");
    }

    for (size_t nIdx = 0; nIdx < pProp->GetArraySize(); ++nIdx)
    {
        std::string strValue;
        if (pProp->IsString())
        {
            const char* strVal = NULL;
            pProp->GetValue(strVal, nIdx);

            if (strlen(strVal) != 0 || pProp->IsArray() || pProp->GetSubProperties().empty())
            {
                strDest.append(std::string("\"") + strVal + "\"");
            }
        }
        else if (pProp->IsBoolean())
        {
            bool bVal = false;
            pProp->GetValue(bVal, nIdx);
            strDest.append(bVal ? "true" : "false");
        }
        else if (pProp->IsFloat())
        {
            double fVal = 0;
            pProp->GetValue(fVal, nIdx);
            strDest.append(ToString(fVal));
        }
        else if (pProp->IsInteger())
        {
            int32_t nVal = 0;
            pProp->GetValue(nVal, nIdx);
            strDest.append(ToString(nVal));
        }

        if (nIdx + 1 < pProp->GetArraySize())
        {
            strDest.append(", ");
        }
    }

    if (pProp->IsArray())
    {
        strDest.append("}");
    }
}

// Helper function that formats a received property into a string that is readable on the console
static void FormatProperty(const fep::IProperty* pProp, std::string& strDest, int nIndent = 0)
{
    if (!pProp) return;

    std::string strProp(nIndent, ' ');
    strProp.append(pProp->GetName());
    strProp.append(": ");
    FormatValue(pProp, strProp);
    strProp.append("\n");

    const fep::IProperty::tPropertyList& lstSubs = pProp->GetSubProperties();
    for (fep::IProperty::tPropertyList::const_iterator it = lstSubs.begin();
        it != lstSubs.end(); ++it)
    {
        FormatProperty(*it, strProp, nIndent + 2);
    }

    strDest.append(strProp);
}

cRemoteProperties::cRemoteProperties(const char * strRemoteElement,
    const char * strRemotePath, timestamp_t tmTimeout)
    : m_strRemoteElement(strRemoteElement), m_strRemotePath(strRemotePath),
    m_tmTimeout(tmTimeout), m_nCode(0)
{
}

cRemoteProperties::~cRemoteProperties()
{ }

int cRemoteProperties::GetResultCode() const
{
    return m_nCode;
}

fep::Result cRemoteProperties::Run(const fep::cModuleOptions& oModuleOptions)
{
    fep::Result nRes = Create(oModuleOptions);
    if (fep::isOk(nRes))
    {
        nRes = WaitForShutdown();
    }

    return nRes;
}

fep::Result cRemoteProperties::ProcessInitializingEntry(const fep::tState eOldState)
{
    GetStateMachine()->InitDoneEvent();
    return fep::ERR_NOERROR;
}

fep::Result cRemoteProperties::ProcessIdleEntry(const fep::tState eOldState)
{
    // only advance if we're coming from startup
    if (fep::FS_STARTUP == eOldState)
    {
        GetStateMachine()->InitializeEvent();
    }
    else
    {
        GetStateMachine()->ShutdownEvent();
    }
    return fep::ERR_NOERROR;
}

fep::Result cRemoteProperties::ProcessReadyEntry(const fep::tState eOldState)
{
    GetStateMachine()->StartEvent();
    return fep::ERR_NOERROR;
}

fep::Result cRemoteProperties::ProcessRunningEntry(const fep::tState eOldState)
{
    fep::IProperty * poPtr = NULL;
    fep::Result nRes = GetPropertyTree()->GetRemoteProperty(m_strRemoteElement,
        m_strRemotePath, &poPtr, m_tmTimeout);
    std::unique_ptr<fep::IProperty> poProperty(poPtr);

    if (fep::isOk(nRes))
    {
        // root property comes with empty name -> adjust for display purposes
        if (strlen(poProperty->GetName()) == 0)
        {
            poProperty->SetName(m_strRemoteElement);
        }

        std::string strFormat;
        FormatProperty(poProperty.get(), strFormat);
        std::cout << strFormat;
    }
    else if (nRes == fep::ERR_TIMEOUT)
    {
        std::cout << "Timeout\n";
        m_nCode = 1;
    }
    else
    {
        std::cout << "Unknown error: " << nRes.getErrorLabel() << "\n";
        m_nCode = nRes.getErrorCode();
    }

    GetStateMachine()->StopEvent();
    return fep::ERR_NOERROR;
}

fep::Result cRemoteProperties::ProcessStartupEntry(const fep::tState eOldState)
{
    // Filling the Element header properly ;)
    double fVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;

        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, fVersion);
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION,"Remote Property Demo");
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fVersion);
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, fVersion);
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR,"Audi Electronics Venture GmbH");
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, "fa2e6a9d-816b-4202-8788-ba7b691ef52d");

    SetStandAloneModeEnabled(true);

    GetStateMachine()->StartupDoneEvent();
    return fep::ERR_NOERROR;
}
