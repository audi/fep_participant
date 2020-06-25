/**
 *
 * Bus Compat Stimuli: Client Module Source 
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
#include "module_client.h"
#include "bus_check_mixed_signal.h"

#include <iostream>

using namespace fep;

cModuleClient::cModuleClient()
{
}

fep::Result cModuleClient::ProcessStartupEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModuleBase::ProcessStartupEntry(eOldState));
    fep::Result nResult= ERR_NOERROR;
    
            
    nResult|= GetStateMachine()->StartupDoneEvent();
    assert(fep::isOk(nResult));

    return nResult;
}

fep::Result cModuleClient::ProcessIdleEntry(const fep::tState eOldState)
{
    fep::Result nResult= ERR_NOERROR;

    nResult= cModule::ProcessIdleEntry(eOldState);
    assert(fep::isOk(nResult));

    if (eOldState == FS_INITIALIZING || eOldState == FS_READY || eOldState == FS_RUNNING)
    {
        nResult= UnregisterSignals();
        assert(fep::isOk(nResult));
    }

    nResult|= GetStateMachine()->InitializeEvent();
    assert(fep::isOk(nResult));

    return nResult;
}

fep::Result cModuleClient::ProcessInitializingEntry(const fep::tState eOldState)
{
    fep::Result nResult= ERR_NOERROR;

    nResult|= cModule::ProcessInitializingEntry(eOldState);
    assert(fep::isOk(nResult));

    nResult|= RegisterSignals();
    assert(fep::isOk(nResult));

    if (fep::isOk(nResult))
    {
        nResult= GetStateMachine()->InitDoneEvent();
    }

    return nResult;
}

fep::Result cModuleClient::ProcessReadyEntry(const fep::tState eOldState)
{
    fep::Result nResult= ERR_NOERROR;

    if (fep::isOk(nResult))
    {
        nResult= GetStateMachine()->StartEvent();
    }

    return nResult;
}

void cModuleClient::SetServerElementName(char const* const strServerElementName)
{
    m_strServerElementName= strServerElementName;
}

const char* cModuleClient::GetServerElementName() const
{
    return m_strServerElementName.c_str();
}

fep::Result cModuleClient::RegisterSignals()
{
    fep::Result nResult= ERR_NOERROR;

    {
        std::string strSignalType = "BusCheckMixedSignal";
        std::string strOutputSignalName = strSignalType + "Request";
        std::string strInputSignalName = strSignalType +  "Response";
 
        nResult|= GetSignalRegistry()->RegisterSignalDescription(s_strBusCheckMixedSignalDescription);
        assert(fep::isOk(nResult));

        nResult|= GetSignalRegistry()->RegisterSignal(cUserSignalOptions(strInputSignalName.c_str(), SD_Input, strSignalType.c_str())
            , m_hInputSignalHandle);
        assert(fep::isOk(nResult));

        nResult|= GetSignalRegistry()->RegisterSignal(cUserSignalOptions(strOutputSignalName.c_str(), SD_Output, strSignalType.c_str())
            , m_hOutputSignalHandle);
        assert(fep::isOk(nResult));

    }

    return nResult;
}

fep::Result cModuleClient::UnregisterSignals()
{
    fep::Result nResult= ERR_NOERROR;

    {
        nResult|= GetSignalRegistry()->UnregisterSignal(m_hOutputSignalHandle);
        assert(fep::isOk(nResult));

        nResult|= GetSignalRegistry()->UnregisterSignal(m_hInputSignalHandle);
        assert(fep::isOk(nResult));
    }

    return nResult;
}

