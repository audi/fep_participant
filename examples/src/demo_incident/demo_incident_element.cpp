/**
 * Implementation of an badly coded FEP Element
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
#include <cstdio>
#include <ctime>

#include <fep_participant_sdk.h>
#include <a_util/system.h>
#include <a_util/strings.h>

//Disable warning for fopen
#ifdef WIN32
#pragma warning(disable:4996)
#endif

#include "demo_incident_element.h"

cBadlyCodedElement::cBadlyCodedElement()
{ }

cBadlyCodedElement::~cBadlyCodedElement()
{ }

fep::Result cBadlyCodedElement::ProcessIdleEntry(const fep::tState eOldState)
{
    if (fep::FS_STARTUP == eOldState)
    {
        std::cout << "Startup Done " << GetName() << std::endl;
    }
    else
    {
        std::cout << "Stopped " << GetName() << std::endl;
    }

    return fep::ERR_NOERROR;
}

fep::Result cBadlyCodedElement::ProcessReadyEntry(const fep::tState eOldState)
{
    std::cout << "Ready " << GetName() << std::endl;
    return fep::ERR_NOERROR;
}

fep::Result cBadlyCodedElement::ProcessRunningEntry(const fep::tState eOldState)
{
    std::cout << GetName() << " reached running mode" << std::endl;
    // sleeping 5 seconds as if this element had more work to do...
    a_util::system::sleepMilliseconds(5000);   
    return fep::ERR_NOERROR;
}

fep::Result cBadlyCodedElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    std::cout << "Initializing " << GetName() << std::endl;
    GetStateMachine()->InitDoneEvent();
    return fep::ERR_NOERROR;
}

fep::Result cBadlyCodedElement::ProcessStartupEntry(const fep::tState eOldState)
{
    if (eOldState == fep::FS_ERROR || eOldState == fep::FS_IDLE)
    {
        std::cout << "Shutting down " << GetName() << " entirely" << std::endl;
    }
    else
    {
        // Configuring the element's incident handling
        // This is not mandatory, defaults apply. See SDK documentation for details.

        // # Enabling the incident handler for all purposes including but not limited to
        // # error handling, logging and post-processing.
        RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
            FEP_INCIDENT_HANDLER_ENABLE, true));
        // # Disable handling of remove incidents issued by other FEP Elements on the bus.
        // # This feature is mostly required by and kind of Master Element or
        // # Test Evaluation Element.
        RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
            FEP_INCIDENT_HANDLER_ENABLE_GLOBAL_SCOPE, false));

        // # Never print log output; this examples runs the Master Element in the same process
        // # and on the same stdout. One log is sufficient ;)
        RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
            FEP_CONSOLE_LOG_ENABLE, false));
        RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
            FEP_CONSOLE_LOG_ENABLE_CATCHALL, false));

        // # Turn off the incident history. It is not necessary for this example. It's
        // # mostly useful to replace the console log strategy when running in a RT compliant
        // # context and to process / analyze the contents after returning from the running
        // # state.
        RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
                             fep::component_config::g_strIncidentHistoryLogPath_bEnable, false));

        // # Turning off file logging - it is out of the scope if this example
        RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
                             fep::component_config::g_strIncidentFileLogPath_bEnable, false));

        // # Turn off the history strategy for the same reason
        RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
                             fep::component_config::g_strIncidentHistoryLogPath_bEnable, false));

        // # Turning on the Notification Strategy. This will allow the master to fill in
        // # its own incident log with this element's incidents as well.
        RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
            FEP_NOTIFICATION_LOG_ENABLE, true));
        // # Capturing all incidents
        RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
            FEP_NOTIFICATION_LOG_ENABLE_CATCHALL, true));
        // # For convenience all incidents are being reported to every element on the FEP
        // # bus (broadcasting)
        RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
                             fep::component_config::g_strIncidentNotificationLogField_strTarget, "*"));

        // now we're good to go
        std::cout << "Startup " << GetName() << std::endl;
        a_util::system::sleepMilliseconds(2000);
        GetStateMachine()->StartupDoneEvent();
    }

    return fep::ERR_NOERROR;
}

fep::Result cBadlyCodedElement::ProcessShutdownEntry(const fep::tState eOldState)
{
    std::cout << "Shutting down " << GetName() << " entirely" << std::endl;
    return fep::ERR_NOERROR;
}

fep::Result cBadlyCodedElement::ProcessErrorEntry(const fep::tState eOldState)
{
    std::cout << GetName() << " threw an error! Something went wrong!" << std::endl;
    return fep::ERR_NOERROR;
}

fep::Result cBadlyCodedElement::FillInElementHeader()
{
    if (!GetPropertyTree())
    {
        return fep::ERR_POINTER;
    }
    double fFepVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;

    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, 1.0));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_NAME, GetName()));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION, 
        "A generic slave element demonstrating the incident handling mechanisms of FEP Core."));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_PLATFORM, FEP_SDK_PARTICIPANT_PLATFORM_STR));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "Example"));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, fFepVersion));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR,"AEV"));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DISPLAY_NAME, "Demo Badly Coded Element"));
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, "ddf2d0e0-1a7d-4d90-ae00-cc5535481264"));

    return fep::ERR_NOERROR;
}

