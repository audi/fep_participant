/************************************************************************
 * Implementation of an exemplary FEP Base Element
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
#include <fep_participant_sdk.h>
#include <a_util/system.h>
#include "demo_master_element.h"

#define FEP_PREP_CMD_VERSION 1.0

// The error code that our error recovery strategy can handle and fix
static int16_t s_nErrorIncidentCode = -0xF00;
// The incident code to indicate that the slave is finished
static int16_t s_nFinishedIncidentCode = -0xF01;

cMasterElement::cMasterElement() : m_bInitSlave(true)
{ }

cMasterElement::~cMasterElement()
{ }

fep::Result cMasterElement::FillElementHeader()
{
    if (!GetPropertyTree())
    {
        return fep::ERR_POINTER;
    }
    double fFepVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, 1.0));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_NAME, GetName()));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
        FEP_PARTICIPANT_HEADER_DESCRIPTION, "Error Recovery Demo: Master element"));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_PLATFORM, FEP_SDK_PARTICIPANT_PLATFORM_STR));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "Example"));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, fFepVersion));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "AEV"));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DISPLAY_NAME, "Demo Error Recovery"));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, "75162962-eea2-4003-8858-fe35f6955da1"));

    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::ProcessStartupEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "Startup",NULL,0,NULL);

    // Fill participant header
    RETURN_IF_FAILED(FillElementHeader());

    // Enable the incident handler
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
        FEP_INCIDENT_HANDLER_ENABLE, true));

    // Enable handling of remote incidents
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
        FEP_INCIDENT_HANDLER_ENABLE_GLOBAL_SCOPE, true));

    // Associate our own error recovery incident strategy with the incident we can handle
    // as well as the incident that signals that the slave has finished
    RETURN_IF_FAILED(GetIncidentHandler()->
        AssociateStrategy(s_nErrorIncidentCode, this));
    RETURN_IF_FAILED(GetIncidentHandler()->
        AssociateStrategy(s_nFinishedIncidentCode, this));

    // Register ourself as notification listener
    RETURN_IF_FAILED(GetNotificationAccess()->RegisterNotificationListener(this));

    // Continue
    GetStateMachine()->StartupDoneEvent();

    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::ProcessIdleEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "Idle",NULL,0,NULL);

    if (eOldState == fep::FS_STARTUP)
    {
        GetStateMachine()->InitializeEvent();
    }
    else
    {
        GetStateMachine()->ShutdownEvent();
    }
    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "Initializing",NULL,0,NULL);

    GetStateMachine()->InitDoneEvent();
    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::ProcessReadyEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "Ready",NULL,0,NULL);

    GetStateMachine()->StartEvent();
    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::ProcessRunningEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "Running",NULL,0,NULL);

    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::CleanUp(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION,
        fep::SL_Info, "Cleanup", NULL, 0, NULL);
    RETURN_IF_FAILED(GetNotificationAccess()->UnregisterNotificationListener(this));

    // Disassociate our own error recovery incident strategy
    RETURN_IF_FAILED(GetIncidentHandler()->
        DisassociateStrategy(s_nErrorIncidentCode, this));
    RETURN_IF_FAILED(GetIncidentHandler()->
        DisassociateStrategy(s_nFinishedIncidentCode, this));

    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::ProcessShutdownEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "Shutdown",NULL,0,NULL);
    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::ProcessErrorEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "Error",NULL,0,NULL);

    GetStateMachine()->ShutdownEvent();
    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::Update(fep::IStateNotification const * pNotification)
{
    if (std::string(pNotification->GetSender()) == "SlaveElement")
    {
        bool bReact = false;
        fep::tControlEvent eEvent = fep::CE_Shutdown;

        switch (pNotification->GetState())
        {
        case fep::FS_IDLE:
            if (m_bInitSlave)
            {
                eEvent = fep::CE_Initialize;
                bReact = true;
            }
            break;
        case fep::FS_SHUTDOWN:
            GetStateMachine()->StopEvent();
            GetStateMachine()->ShutdownEvent();
            break;
        case fep::FS_READY:
            eEvent = fep::CE_Start;
            bReact = true;
            break;
        default:
            break;
        }

        if (bReact)
        {
            GetStateMachine()->TriggerRemoteEvent(eEvent, pNotification->GetSender());
        }
    }

    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::HandleLocalIncident(fep::IModule *pElementContext, const int16_t nIncident,
    const fep::tSeverityLevel eSeverity, const char *strOrigin, int nLine, const char *strFile,
                                            const timestamp_t tmSimTime, const char *strDescription)
{
    // we only handle remote incidents here, nothing to do
    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::HandleGlobalIncident(const char *strSource, const int16_t nIncident,
    const fep::tSeverityLevel eSeverity, const timestamp_t tmSimTime, const char *strDescription)
{
    // is this the error incident or the finished signal?
    if (nIncident == s_nErrorIncidentCode)
    {
        GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info,
            "Received error incident, fixing now",NULL,0,NULL);
        // fix the error here (we only pretend to)
        a_util::system::sleepMilliseconds(1000);

        // instruct the element that we fixed the error
        GetStateMachine()->TriggerRemoteEvent(fep::CE_ErrorFixed, strSource);
    }
    else if (nIncident == s_nFinishedIncidentCode)
    {
        // client finished, stop initializing it
        m_bInitSlave = false;

        // instruct the element that we fixed to stop & shutdown
        GetStateMachine()->TriggerRemoteEvent(fep::CE_Stop, strSource);
        GetStateMachine()->TriggerRemoteEvent(fep::CE_Shutdown, strSource);

        GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info,
            "Received finished incident, stopping now",NULL,0,NULL);
    }
    else
    {
        // we havent associated this strategy with any other incidents, this is an error
        return fep::ERR_UNEXPECTED;
    }

    return fep::ERR_NOERROR;
}

fep::Result cMasterElement::RefreshConfiguration(const fep::IProperty *pStrategyProperty,
    const fep::IProperty *pAffectedProperty)
{
    // this strategy doesn't have any configuration options
    return fep::ERR_NOERROR;
}
