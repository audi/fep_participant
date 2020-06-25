/************************************************************************
 * Implementation of the slave element used in the error recovery demo
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
#include "demo_error_element.h"

// The error code we're using
static int16_t s_nErrorIncidentCode = -0xF00;
// The incident code to indicate that the slave is finished
static int16_t s_nFinishedIncidentCode = -0xF01;

cSlaveElement::cSlaveElement() : m_nErrorCount(0)
{
}

cSlaveElement::~cSlaveElement()
{
}

fep::Result cSlaveElement::FillElementHeader()
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
        FEP_PARTICIPANT_HEADER_DESCRIPTION, "Error Recovery Demo: Slave element"));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_PLATFORM, FEP_SDK_PARTICIPANT_PLATFORM_STR));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "Example"));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, fFepVersion));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "AEV"));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DISPLAY_NAME, "Demo Error Recovery"));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, 
        "65d8321a-13b1-4559-8e29-525d597f4550"));

    return fep::ERR_NOERROR;
}

fep::Result cSlaveElement::ProcessStartupEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "Startup",NULL,0,NULL);

    // Fill participant header
    fep::Result nRes = FillElementHeader();
    
    if (fep::isFailed(nRes))
    {
        // Non critical error, just log an incident
        GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_WARNING, fep::SL_Warning, 
            "Element header could not be filled.", NULL, 0, NULL);
    }

    // Enable notification strategy to make incidents available to the master
    nRes = GetPropertyTree()->SetPropertyValue(FEP_NOTIFICATION_LOG_ENABLE, true);
    if (fep::isFailed(nRes))
    {
        // Critical error (for this example). Error recovery in not possible in this case
        // -> log incident -> shutdown
        GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_CRITICAL_FAILURE, fep::SL_Critical_Local,
            "Incident notification strategy could not be set.", NULL, 0, NULL);

        GetStateMachine()->ShutdownEvent();

        // Returning no error since returning an error code will result in the participant 
        // not being created correctly, instead of being shutdown well-ordered by the shutdown event
        return fep::ERR_NOERROR;
    }

    // Associate notification strategy with our finished incident code
    GetIncidentHandler()
        ->AssociateStrategy(s_nFinishedIncidentCode, fep::ES_LogNotification, fep::SA_APPEND);

    GetStateMachine()->StartupDoneEvent();
    return fep::ERR_NOERROR;
}

fep::Result cSlaveElement::ProcessIdleEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "Idle",NULL,0,NULL);

    return fep::ERR_NOERROR;
}

fep::Result cSlaveElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "Initializing",NULL,0,NULL);

    GetStateMachine()->InitDoneEvent();
    return fep::ERR_NOERROR;
}

fep::Result cSlaveElement::ProcessReadyEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "Ready",NULL,0,NULL);

    return fep::ERR_NOERROR;
}

fep::Result cSlaveElement::ProcessRunningEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "Running",NULL,0,NULL);

    a_util::system::sleepMilliseconds(2*1000);

    if (m_nErrorCount < 2)
    {
        m_nErrorCount++;
        GetStateMachine()->ErrorEvent();
        GetIncidentHandler()->InvokeIncident(s_nErrorIncidentCode, fep::SL_Critical,
            "Invoking critical incident",NULL,0,NULL);
        return fep::ERR_NOERROR;
    }

    RETURN_IF_FAILED(GetIncidentHandler()->
        InvokeIncident(s_nFinishedIncidentCode, fep::SL_Info, "Finished work", NULL, 0, NULL));

    return fep::ERR_NOERROR;
}

fep::Result cSlaveElement::CleanUp(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "CleanUp",NULL,0,NULL);
    // Disassociate notification strategy with our finished incident code
    GetIncidentHandler()->DisassociateStrategy(s_nFinishedIncidentCode, fep::ES_LogNotification);

    return fep::ERR_NOERROR;
}

fep::Result cSlaveElement::ProcessShutdownEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "Shutdown",NULL,0,NULL);

    return fep::ERR_NOERROR;
}

fep::Result cSlaveElement::ProcessErrorEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info, "Error",NULL,0,NULL);

    return fep::ERR_NOERROR;
}
