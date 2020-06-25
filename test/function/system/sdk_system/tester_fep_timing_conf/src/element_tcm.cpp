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

#include "element_tcm.h"
#include "common.h"

#include <stdlib.h>

#include <iostream> // debug
#include "a_util/system.h"
#include "a_util/strings.h"

using namespace fep;

cTimingClientMasterElement::cTimingClientMasterElement()
    : m_nNumberOfSteps(0)
    , m_tmStartTimeStamp(0)
    , m_tmStopTimeStamp(0)
{
}

cTimingClientMasterElement::~cTimingClientMasterElement()
{
    Destroy();
}

Result cTimingClientMasterElement::ProcessStartupEntry(const tState eOldState)
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

    // This element is the timing master
    GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, GetName());

    nResult |= GetStateMachine()->StartupDoneEvent();

    return nResult;
}

Result cTimingClientMasterElement::ProcessIdleEntry(const tState eOldState)
{
    Result nResult = ERR_NOERROR;

    // Unregister Step Listeners ... ignore error as they might not be registered
    (void) GetTimingInterface()->UnregisterStepListener("DummyStep");

    return nResult;
}

Result cTimingClientMasterElement::ProcessInitializingEntry(const tState eOldState)
{
    Result nResult = ERR_NOERROR;
    
    // Register steps
    if (isOk(nResult))
    {
        int32_t nNumberOfSteps = 0;

        if (fep::isFailed(GetPropertyTree()->GetPropertyValue(
            DUMMY_ELEMENT_NUMBER_OF_STEPS_PROPERTY, nNumberOfSteps)))
        {
            nNumberOfSteps = 1;
        }

        for (int32_t i = 0; i < nNumberOfSteps; ++i)
        {
            std::string strStepName;
            
            if (i == 0)
            {
                strStepName = "DummyStep";
                nResult |= GetTimingInterface()->RegisterStepListener(strStepName.c_str(), StepConfig(100 * 1000), &cTimingClientMasterElement::CountStep_caller, this);
            }
            else
            {
                strStepName = a_util::strings::format("DummyStep%d", i);
                nResult |= GetTimingInterface()->RegisterStepListener(strStepName.c_str(), StepConfig(100 * 1000), &cTimingClientMasterElement::DummyStep_caller, this);
            }

            
            if (isFailed(nResult))
            {
                std::stringstream strMessage;
                strMessage << "\"RegisterStepListener\" failed! Result code = " << nResult.getErrorLabel();

                RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nResult.getErrorCode()),
                    SL_Critical_Global, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
            }
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

Result cTimingClientMasterElement::ProcessRunningEntry(const tState eOldState)
{
    m_nNumberOfSteps = 0;
    m_tmStartTimeStamp = 0; 

    return ERR_NOERROR;
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

void cTimingClientMasterElement::DummyStep(timestamp_t tmSimulation, IStepDataAccess* pStepDataAccess)
{
#ifdef DUMMY_STEP_WITH_WAIT_SLEEP
    const uint32_t my_random_sleep_us = rand() % 1000; // Maximum sleep is 50ms == Half of cycle time
    a_util::system::sleepMicroseconds(my_random_sleep_us);
#endif
#ifdef DUMMY_STEP_WITH_WAIT_NOP
    const uint32_t my_random_count = rand() % 1000; 
    for (uint32_t i= 0; i< my_random_count; ++i)
    {
        NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP();
    }
#endif
}

void cTimingClientMasterElement::CountStep(timestamp_t tmSimulation, IStepDataAccess* pStepDataAccess)
{
    m_tmStopTimeStamp = a_util::system::getCurrentMicroseconds();
    if (m_tmStartTimeStamp == 0)
    {
        m_tmStartTimeStamp = m_tmStopTimeStamp;
        // Should add at least the cycle time, but this is not known
    }

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
    ++m_nNumberOfSteps;
}

