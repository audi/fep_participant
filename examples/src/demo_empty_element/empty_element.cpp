/************************************************************************
 * Implementation of an exemplary Base FEP Element
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
#include <a_util/system.h>
#include <fep_participant_sdk.h>

#include "empty_element.h"

cEmptyElement::~cEmptyElement()
{
    Destroy();
}

fep::Result cEmptyElement::ProcessIdleEntry(const fep::tState eOldState)
{
    switch(eOldState)
    {
    case fep::FS_RUNNING:
        std::cout << "Stopped " << GetName() << std::endl;
        a_util::system::sleepMilliseconds(1000);
        break;
    case fep::FS_STARTUP:
        std::cout << "Startup Done " << GetName() << std::endl;
        a_util::system::sleepMilliseconds(1000);
        break;
    default:
        break;
    }
    return fep::ERR_NOERROR;
}

fep::Result cEmptyElement::ProcessReadyEntry(const fep::tState eOldState)
{
    // note: Ready can only be reached from "INITIALIZING"
    std::cout << "Initializing Done " << GetName() << std::endl;
    a_util::system::sleepMilliseconds(1000);
    return fep::ERR_NOERROR;
}

fep::Result cEmptyElement::ProcessRunningEntry(const fep::tState eOldState)
{
    // note: Running can only be reached from "READY"
    std::cout << GetName() << " reached running mode" << std::endl;
    return fep::ERR_NOERROR;
}

fep::Result cEmptyElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    // note : Initializing can only be reached from "IDLE"
    std::cout << "Initializing " << GetName() << std::endl;

    a_util::system::sleepMilliseconds(1000);

    // note: this event can only to be triggered internally
    GetStateMachine()->InitDoneEvent();

    return fep::ERR_NOERROR;
}

fep::Result cEmptyElement::ProcessStartupEntry(const fep::tState eOldState)
{
    // Filling the Element header properly
    double fVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR)  +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, fVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION, "Example: empty FEP Element");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, fVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, 
        "20b9f0c8-57bf-465c-b690-7db0d1f94b0f");

    std::cout << "Startup " << GetName() << std::endl;
    a_util::system::sleepMilliseconds(1000);
    // note: this event can only to be triggered internally
    GetStateMachine()->StartupDoneEvent();

    return fep::ERR_NOERROR;
}

fep::Result cEmptyElement::ProcessErrorEntry(const fep::tState eOldState)
{
    std::cout << GetName() << " threw an error! Something went wrong!" << std::endl;
    a_util::system::sleepMilliseconds(1000);
    return fep::ERR_NOERROR;
}

fep::Result cEmptyElement::ProcessShutdownEntry(const fep::tState eOldState)
{
    std::cout << "Shutting down " << GetName() << " entirely" << std::endl;
    return fep::ERR_NOERROR;
}

fep::Result cEmptyElement::CleanUp(const fep::tState eOldState)
{
    std::cout << "Cleaning up " << GetName() << std::endl;

    return fep::ERR_NOERROR;
}
