/**
* Implementation of timing master used for integration testing
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

#include "element_tm.h"
#include "common.h"
#include <iostream>
#include <algorithm>

using namespace fep;


cTimingMasterElement::cTimingMasterElement()
{
}

cTimingMasterElement::~cTimingMasterElement()
{
    Destroy();
}

Result cTimingMasterElement::ProcessStartupEntry(const tState eOldState)
{
    GetPropertyTree()->SetPropertyValue(g_strElementHeaderPath_fElementVersion, 1.0);
    GetPropertyTree()->SetPropertyValue(g_strElementHeaderPath_strElementDescription, "Timing Demo Master");
    double fFepVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    GetPropertyTree()->SetPropertyValue(g_strElementHeaderPath_fFEPVersion, fFepVersion);
    GetPropertyTree()->SetPropertyValue(g_strElementHeaderPath_strElementContext, "FEP SDK");
    GetPropertyTree()->SetPropertyValue(g_strElementHeaderPath_fElementContextVersion, 1.0);
    GetPropertyTree()->SetPropertyValue(g_strElementHeaderPath_strElementVendor, "Audi Electronics Venture GmbH");
    GetPropertyTree()->SetPropertyValue(g_strElementHeaderPath_strTypeID, "412a41ac-0060-41b6-b74f-a3d20c51e6a6");

    // This element is the timing master
    GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, GetName());
 
    GetStateMachine()->StartupDoneEvent();
    return ERR_NOERROR;
}

Result cTimingMasterElement::ProcessInitializingEntry(const tState eOldState)
{
    GetStateMachine()->InitDoneEvent();
    return ERR_NOERROR;
}


