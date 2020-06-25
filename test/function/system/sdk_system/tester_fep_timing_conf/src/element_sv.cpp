/**
* Implementation of timing client / server element used for integration testing
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

#include "element_sv.h"
#include "common.h"

#include <stdlib.h>

// For Debug purposes only
#include <iostream>
//#define DBG_ONLY(x) x
#define DBG_ONLY(x)
#define ERR_OUT(x) x


using namespace fep;

cServerElement::cServerElement()
    : m_hRequestInput(nullptr)
    , m_hResponseOutput(nullptr)
    , m_pSampleRequestInput(nullptr)
    , m_pSampleResponseOutput(nullptr)
    , m_errorCount(0)
{
}

cServerElement::~cServerElement()
{
    Destroy();
}

Result cServerElement::ProcessStartupEntry(const tState eOldState)
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


    // Register Descripotion
    nResult |= GetSignalRegistry()->RegisterSignalDescription("files/signals.description",
        ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_MERGE);
    if (isFailed(nResult))
    {
        // common reaction is to publish an incident
        std::stringstream strMessage;
        strMessage << "\"RegisterSignalDescription\" failed with error " << nResult.getErrorLabel();

        RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nResult.getErrorCode()),
            SL_Critical_Global, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
    }

    nResult |= GetStateMachine()->StartupDoneEvent();

    return nResult;
}

Result cServerElement::ProcessIdleEntry(const tState eOldState)
{
    Result nResult = ERR_NOERROR;
    if (m_pSampleRequestInput)
    {
        delete m_pSampleRequestInput;
        m_pSampleRequestInput = NULL;
    }
    if (m_pSampleResponseOutput)
    {
        delete m_pSampleResponseOutput;
        m_pSampleResponseOutput = NULL;
    }

    if (m_hRequestInput)
    {
        nResult |= GetSignalRegistry()->UnregisterSignal(m_hRequestInput);
    }
    if (m_hResponseOutput)
    {
        nResult |= GetSignalRegistry()->UnregisterSignal(m_hResponseOutput);
    }

    // Unregister Step Listeners ... ignore error as they might not be registered
    (void) GetTimingInterface()->UnregisterStepListener("ServRequest");

    return nResult;
}

Result cServerElement::ProcessInitializingEntry(const tState eOldState)
{
    Result nResult = ERR_NOERROR;
    
    // Register signals
    if (isOk(nResult))
    {
        // get signal names
        std::string strRequestSignalName = "Request";
        std::string strResponseSignalName = "Response";
        {
            const char* strTempValue;
            if (fep::isOk(GetPropertyTree()->GetPropertyValue(
                REQUEST_SIGNAL_NAME_PROPERTY, strTempValue)))
            {
                strRequestSignalName = strTempValue;
            }
            if (fep::isOk(GetPropertyTree()->GetPropertyValue(
                RESPONSE_SIGNAL_NAME_PROPERTY, strTempValue)))
            {
                strResponseSignalName = strTempValue;
            }
        }

        // registering signals
        nResult |= GetSignalRegistry()->RegisterSignal(cUserSignalOptions(strRequestSignalName.c_str(), SD_Input, "tRequest"), m_hRequestInput);
        nResult |= GetSignalRegistry()->RegisterSignal(cUserSignalOptions(strResponseSignalName.c_str(), SD_Output, "tResponse"), m_hResponseOutput);
        if (isFailed(nResult))
        {
            std::stringstream strMessage;
            strMessage << "\"RegisterSignal\" failed with error " << nResult.getErrorLabel();

            RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nResult.getErrorCode()),
                SL_Critical_Global, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
        }
    }

    // Register samples
    if (isOk(nResult))
    {
        // create samples
        nResult |= GetUserDataAccess()->CreateUserDataSample(m_pSampleRequestInput, m_hRequestInput);
        nResult |= GetUserDataAccess()->CreateUserDataSample(m_pSampleResponseOutput, m_hResponseOutput);
        if (isFailed(nResult))
        {
            std::stringstream strMessage;
            strMessage << "Could not allocate data samples for signals! Result code = " << nResult.getErrorLabel();

            RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nResult.getErrorCode()),
                SL_Critical_Global, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
        }
    }

    // Register steps
    if (isOk(nResult))
    {
        // now registering the timing configuration (only cycle time is required - more configuration possible if wanted)
        nResult |= GetTimingInterface()->RegisterStepListener("ServRequest", StepConfig(10 * 1000), &cServerElement::ServRequest_caller, this);
        if (isFailed(nResult))
        {
            std::stringstream strMessage;
            strMessage << "\"RegisterStepListener\" failed! Result code = " << nResult.getErrorLabel();

            RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nResult.getErrorCode()),
                SL_Critical_Global, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
        }
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

inline void NOP()
{
#ifdef _MSC_VER
    // Microsoft compiler
    __nop();
#else
#ifdef  __GNUC__
    // GNU Compiler
    asm("nop");
#else
    // Do not know how to do nops
    ;
#endif
#endif
}

void cServerElement::ServRequest(timestamp_t tmSimulation, IStepDataAccess* pStepDataAccess)
{
#ifdef DUMMY_STEP_WITH_WAIT_SLEEP
    const uint32_t my_random_sleep_us = rand() % 1000; // Maximum sleep is 50ms == Half of cycle time
    a_util::system::sleepMicroseconds(my_random_sleep_us);
#endif
#ifdef DUMMY_STEP_WITH_WAIT_NOP
    const uint32_t my_random_count = rand() % 1000;
    for (uint32_t i = 0; i< my_random_count; ++i)
    {
        NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP();
    }
#endif

    Result nResult = pStepDataAccess->CopyRecentData(m_hRequestInput, m_pSampleRequestInput);

    if (isOk(nResult))
    {
        tResponse* pResponse = reinterpret_cast<tResponse*>(m_pSampleResponseOutput->GetPtr());
        tRequest* pRequest = reinterpret_cast<tRequest*>(m_pSampleRequestInput->GetPtr());
        pResponse->response_value = pRequest->request_value;
        DBG_ONLY(std::cerr << "SERVER BACK: " << pResponse->response_value << std::endl;)
        nResult = pStepDataAccess->TransmitData(m_pSampleResponseOutput);
    }
    else if (nResult == ERR_OUT_OF_SYNC)
    {
        // Nothing to send
    }
    else if (nResult == ERR_NOT_FOUND)
    {
        ERR_OUT(std::cerr << "ServRequest: NO PACKET FOUND" << std::endl;)
        ++m_errorCount;
    }
    else
    {
        ERR_OUT(std::cerr << "ServRequest: UNEXPECTED RESPONSE VALUE" << std::endl;)
        ++m_errorCount;
    }
 
    // No way to report error
    (void)nResult;
}

