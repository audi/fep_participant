/**
 *
 * Bus Compat Stimuli: Bus Check for Custom Command
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

#include "stdafx.h"
#include "bus_check_incidents.h"
#include "module_client.h"

#include <iostream>
#include "bus_check_state_changes.h"

using namespace fep;

cBusCheckIncidents::cBusCheckIncidents(const int16_t nFEPIncident, const fep::tSeverityLevel eSeverity, const char* strDescription, const int nLine, const char* strFile) 
    : m_nFEPIncident(nFEPIncident)
    , m_eSeverity(eSeverity)
    , m_strDescription(strDescription)
    , m_nLine(nLine)
    , m_strFile(strFile)
{
}

fep::Result cBusCheckIncidents::Update(fep::IIncidentNotification const * poNotification) 
{
    fep::Result nResult= ERR_NOERROR;

    CompareString(poNotification->GetDescription(), m_strDescription.c_str(), "Description Mismatch");
    Compare(poNotification->GetIncidentCode(), m_nFEPIncident, "Incident Code Mismatch");
    Compare(poNotification->GetSeverity(), m_eSeverity, "Severity Mismatch");
    CompareString(poNotification->GetReceiver(), GetClientModule()->GetName(), "Receiver/Sender Mismatch");
    CompareString(poNotification->GetSender(), GetClientModule()->GetServerElementName(), "Sender/Receiver Mismatch");

    NotifyGotResult();

    return nResult;
}

fep::Result cBusCheckIncidents::DoSend()
{
    fep::Result nResult= ERR_NOERROR;
    
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
    nResult|= GetClientModule()->GetIncidentHandler()->InvokeIncident(
        m_nFEPIncident, 
        m_eSeverity,   
        m_strDescription.c_str()
        );
#else
    nResult|= GetClientModule()->GetIncidentHandler()->InvokeIncident(
        m_nFEPIncident, 
        m_eSeverity,   
        m_strDescription.c_str(),  
        GetClientModule()->GetName(),
        m_nLine,                   
        m_strFile.c_str()        
        );
#endif

    return nResult;
}
    
