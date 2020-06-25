/**
 *
 * Bus Compat Stimuli: Base Module Source 
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
#include "module_base.h"

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
#include "a_utils.h"
#else
#include "a_util/system.h"
#endif


using namespace fep;

cModuleBase::cModuleBase()
{
}

fep::Result cModuleBase::ProcessStartupEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessStartupEntry(eOldState));

    fep::Result nResult= ERR_NOERROR;
     // Standadlone Mode
    nResult|= GetPropertyTree()->SetPropertyValue(fep::component_config::g_strStateMachineStandAloneModePath_bEnable, true);


    // Filling the Element header properly
    double fVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR)  +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fElementVersion, fVersion);
    GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementDescription, "Bus Check Element");
    GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fFEPVersion, fVersion);
    GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementContext, "FEP SDK");
    GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fElementContextVersion, fVersion);
    GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementVendor, "Audi Electronics Venture GmbH");
    GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strTypeID, 
        "20b9f0c8-57bf-465c-b690-7db0d1f94b1f");

	// Do not log incidents on console
	GetPropertyTree()->SetPropertyValue(fep::component_config::g_strIncidentConsoleLogPath_bEnable, false);

    return nResult;
}
