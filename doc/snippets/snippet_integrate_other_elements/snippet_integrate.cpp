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

#include "snippet_integrate.h"
// Indention due to usage in doxygen
fep::Result cMyAI::ExampleMethod()
{
    //![GetAvailableElements]
// [...]
std::vector<std::string> poVec;

// collect availability replies for 5 seconds
timestamp_t tmTimeout = static_cast<timestamp_t>(5 * 1000 * 1000);

// broadcast a request for availability information

RETURN_IF_FAILED(GetAvailableParticipants(poVec, tmTimeout));

// iterate over all available FEP Participants that replied to the request
for (size_t szIdx = 0; poVec.size() > szIdx; ++szIdx)
{
    std::string strFEPElement = poVec[szIdx];
    // [...]
}

// [...]
    //![GetAvailableElements]

    //![RegisterMonitoring]
cMyMonitor oMonitor;
// register monitor for all elements
RegisterMonitoring("*", &oMonitor);
// unregister monitor
UnregisterMonitoring(&oMonitor);
// register monitor for RemoteElement
RegisterMonitoring("RemoteElement", &oMonitor);
// unregister monitor
UnregisterMonitoring(&oMonitor);
    //![RegisterMonitoring]
    return fep::ERR_NOERROR;
}