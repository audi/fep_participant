/**
* Implementation of timing client / client element used for FS_ERROR 
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

#include "element_clnt.h"
#include "common.h"

#include <stdlib.h>

// For Debug purposes only
#include <iostream>
//#define DBG_ONLY(x) x
#define DBG_ONLY(x)
#define ERR_OUT(x) x

using namespace fep;

cClientElement::cClientElement()
    : m_hRequestOutput(nullptr)
    , m_hResponseInput(nullptr)
    , m_pSampleRequestOutput(nullptr)
    , m_pSampleResponseInput(nullptr)
    , m_nNextRequestValue(1)
    , m_nExpectedResponseValue(1)
    , m_nNumberOfSteps(0)
    , m_errorCount(0)
{
}

cClientElement::~cClientElement()
{
    Destroy();
}

Result cClientElement::ProcessStartupEntry(const tState eOldState)
{
    Result nResult = ERR_NOERROR;

    nResult |= GetPropertyTree()->SetPropertyValue(g_strElementHeaderPath_fElementVersion, 1.0);
    nResult |= GetPropertyTree()->SetPropertyValue(g_strElementHeaderPath_strElementDescription, "Timing Demo Element Driver");
    double fFepVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    nResult |= GetPropertyTree()->SetPropertyValue(g_strElementHeaderPath_fFEPVersion, fFepVersion);
    nResult |= GetPropertyTree()->SetPropertyValue(g_strElementHeaderPath_strElementContext, "FEP SDK");
    nResult |= GetPropertyTree()->SetPropertyValue(g_strElementHeaderPath_fElementContextVersion, 1.0);
    nResult |= GetPropertyTree()->SetPropertyValue(g_strElementHeaderPath_strElementVendor, "Audi Electronics Venture GmbH");
    nResult |= GetPropertyTree()->SetPropertyValue(g_strElementHeaderPath_strTypeID, "ad4aa57e-54c7-411b-a440-667esf118adf");

    nResult |= GetStateMachine()->StartupDoneEvent();

    return nResult;
}

Result cClientElement::ProcessIdleEntry(const tState eOldState)
{
    Result nResult = ERR_NOERROR;
   
    // Unregister Step Listeners ... ignore error as they might not be registered
    (void) GetTimingInterface()->UnregisterStepListener("SendRequest");
    (void) GetTimingInterface()->UnregisterStepListener("ProcessResponse");

    return nResult;
}

Result cClientElement::ProcessInitializingEntry(const tState eOldState)
{
    Result nResult = ERR_NOERROR;
   
    // now registering the timing configuration (only cycle time is required - more configuration possible if wanted)
    nResult |= GetTimingInterface()->RegisterStepListener("SendRequest", StepConfig(10 * 1000), &cClientElement::SendRequest_caller, this);
    nResult |= GetTimingInterface()->RegisterStepListener("ProcessResponse", StepConfig(10 * 1000), &cClientElement::ProcessResponse_caller, this);
    if (isFailed(nResult))
    {
        std::stringstream strMessage;
        strMessage << "\"RegisterStepListener\" failed! Result code = " << nResult.getErrorLabel();
        RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nResult.getErrorCode()),
            SL_Critical_Global, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
    }
    if (isOk(nResult))
    {
        GetStateMachine()->InitDoneEvent();
    }
    else
    {
        std::stringstream strMessage;
        strMessage << GetName() << " : Initialization failed - check parameters and retry.";
        RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nResult.getErrorCode()),
            SL_Critical_Global, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
    }

    return nResult;
}
void cClientElement::SendRequest(timestamp_t tmSimulation, IStepDataAccess* pStepDataAccess)
{   
}

void cClientElement::ProcessResponse(timestamp_t tmSimulation, IStepDataAccess* pStepDataAccess)
{
}
