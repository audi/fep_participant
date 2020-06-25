/************************************************************************
 * Snippets hosting FEP Participant ... nothing else. :P
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
 *
 * @file
 *
 */
#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <fstream>
#include <ctime>

#define FEP_PREP_CMD_VERSION 1.0

#ifdef WIN32
    // only required for sleep()
    #include "Windows.h"
    #define sleep(x) Sleep(static_cast<tUInt32>(x) * 1000);
#endif

#include "snippet_template.h"

cMyElement::cMyElement()
{
    // nothing to do here...
}

cMyElement::~cMyElement()
{
    // nothing to do here...
}

fep::Result cMyElement::ProcessIdleEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessIdleEntry(eOldState));

    if (FS_STARTUP == eOldState)
    {
        std::cout << "Startup Done " << GetName() << std::endl;
    }

    return fep::ERR_NOERROR;
}

fep::Result cMyElement::ProcessStartupEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessStartupEntry(eOldState));

    return fep::ERR_NOERROR;
}

fep::Result cMyElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessInitializingEntry(eOldState));
    std::cout << "Initializing " << GetName() << std::endl;

    GetStateMachine()->InitDoneEvent();

    return fep::ERR_NOERROR;
}

fep::Result cMyElement::ProcessReadyEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessReadyEntry(eOldState));
    std::cout << "Ready " << GetName() << std::endl;
    return fep::ERR_NOERROR;
}

fep::Result cMyElement::ProcessRunningEntry(const fep::tState eOldState)
{
   
    return fep::ERR_NOERROR;
}

fep::Result cMyElement::ProcessErrorEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessErrorEntry(eOldState));
    std::cout << GetName() << " threw an error! Something went wrong!" << std::endl;
    return fep::ERR_NOERROR;
}
