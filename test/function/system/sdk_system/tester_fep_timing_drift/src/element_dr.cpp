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

#define NOMINMAX

#include "element_dr.h"
#include "common.h"

#include <stdlib.h>

//#include <iostream> // debug
#include "a_util/system.h"
#include "a_util/strings.h"

#include <algorithm>
#include <iostream>
#include <iomanip> 
#include "a_util/system.h"

// For Debug purposes only
#include <iostream>
//#define DBG_ONLY(x) x
#define DBG_ONLY(x) 
#define ERR_OUT(x) x

using namespace fep;

//#define REAL_TIME_DRIFT_VERBOSE_MODE

static void printimestamp_t(std::ostream& os, timestamp_t ts)
{
    if (ts < 0)
    {
        os << "-";
        ts = -ts;
    }
    os
        << ts / 1000000
        << "."
        << std::setfill('0') << std::setw(6)
        << ts % 1000000;
}

#ifndef WIN32
// avoid issues with ambiguous overloading
static inline timestamp_t abs(timestamp_t val){
#ifndef __QNX__
    return static_cast<timestamp_t>(std::abs(val));
#else
    return static_cast<timestamp_t>(llabs(val));
#endif
}
#endif // !WIN32

cDriftElement::cDriftElement()
    : m_tmSimTimePeriod(100 * 1000)
    , m_tmRealTimePeriod(100 * 1000)
{
}

cDriftElement::~cDriftElement()
{
    Destroy();
}

Result cDriftElement::ProcessStartupEntry(const tState eOldState)
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

Result cDriftElement::ProcessIdleEntry(const tState eOldState)
{
    Result nResult = ERR_NOERROR;

    // Unregister Step Listeners ... ignore error as they might not be registered
    (void) GetTimingInterface()->UnregisterStepListener("DriftStep");

    return nResult;
}

Result cDriftElement::ProcessInitializingEntry(const tState eOldState)
{
    Result nResult = ERR_NOERROR;
    
    // Register steps
    if (isOk(nResult))
    {
        int32_t nNumberOfSteps = 0;

        if (fep::isFailed(GetPropertyTree()->GetPropertyValue(
            DRIFT_ELEMENT_NUMBER_OF_STEPS_PROPERTY, nNumberOfSteps)))
        {
            nNumberOfSteps = 1;
        }

        for (int32_t i = 0; i < nNumberOfSteps; ++i)
        {
            std::string strStepName;
            
            if (i == 0)
            {
                strStepName = "DriftStep";
                nResult |= GetTimingInterface()->RegisterStepListener(strStepName.c_str(), StepConfig(m_tmSimTimePeriod), &cDriftElement::CountStep_caller, this);
            }
            else
            {
                strStepName = a_util::strings::format("DriftStep%d", i);
                nResult |= GetTimingInterface()->RegisterStepListener(strStepName.c_str(), StepConfig(m_tmSimTimePeriod), &cDriftElement::DriftStep_caller, this);
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

Result cDriftElement::ProcessRunningEntry(const tState eOldState)
{
    m_nNumberOfSteps = 0;
    m_tmStartTimeStamp = 0; 
    m_tmStopTimeStamp = 0;


    m_tmSimTimeExpected = 0;
    m_tmRealTimeExpected = 0;
    m_tmSimTimeStart = 0;
    m_tmRealTimeStart = 0;

    m_tmResMinDiffSimTime = 999999;
    m_tmResSumDiffSimTime = 0;
    m_tmResQSumDiffSimTime = 0.0;
    m_tmResMaxDiffSimTime = 0;

    m_tmResMinDiffRealTime = 999999;
    m_tmResSumDiffRealTime = 0;
    m_tmResQSumDiffRealTime = 0.0;
    m_tmResMaxDiffRealTime = 0;

    m_tmResMinDiffRealSimDrift = 999999;
    m_tmResSumDiffRealSimDrift = 0;
    m_tmResQSumDiffRealSimDrift = 0.0;
    m_tmResMaxDiffRealSimDrift = 0;

    return ERR_NOERROR;
}

void cDriftElement::printResults(std::ostream& os)
{
    timestamp_t tmResAvgDiffRealTime = m_tmResSumDiffRealTime / m_nNumberOfSteps;
    //timestamp_t tmResDstDiffRealTime = static_cast<timestamp_t>(m_tmResQSumDiffRealTime / static_cast<double>(m_nNumberOfSteps));  

    os << "min/avg/max="; //<< "Result: min/avg/dst/max=";
    printimestamp_t(std::cout, m_tmResMinDiffRealTime); os << "/";
    printimestamp_t(std::cout, tmResAvgDiffRealTime); os << "/";
    //printimestamp_t(std::cout, tmResDstDiffRealTime); os << "/";
    printimestamp_t(std::cout, m_tmResMaxDiffRealTime); os << std::endl;
}

timestamp_t cDriftElement::getAverageDrift() const
{
    return m_tmResSumDiffRealTime / m_nNumberOfSteps;
}

timestamp_t cDriftElement::getMaximumDrift() const
{
    return m_tmResMaxDiffRealTime;
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

void cDriftElement::DriftStep(timestamp_t tmSimulation, IStepDataAccess* pStepDataAccess)
{
#ifdef DRIFT_STEP_WITH_WAIT_SLEEP
    const uint32_t my_random_sleep_us = rand() % 1000; // Maximum sleep is 50ms == Half of cycle time
    a_util::system::sleepMicroseconds(my_random_sleep_us);
#endif
#ifdef DRIFT_STEP_WITH_WAIT_NOP
    const uint32_t my_random_count = rand() % 1000; 
    for (uint32_t i= 0; i< my_random_count; ++i)
    {
        NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP();
    }
#endif
    DBG_ONLY(std::cerr << "Drift Step in " << GetName() << std::endl;)
}

#include <iostream>
void cDriftElement::CountStep(timestamp_t tmSimulation, IStepDataAccess* pStepDataAccess)
{
    m_tmStopTimeStamp = a_util::system::getCurrentMicroseconds();
    if (m_tmStartTimeStamp == 0)
    {
        m_tmStartTimeStamp = m_tmStopTimeStamp;
        // Should add at least the cycle time, but this is not known
    }

#ifdef DRIFT_STEP_WITH_WAIT_SLEEP
    const uint32_t my_random_sleep_us = rand() % 1000; // Maximum sleep is 50ms == Half of cycle time
    a_util::system::sleepMicroseconds(my_random_sleep_us);
#endif
#ifdef DRIFT_STEP_WITH_WAIT_NOP
    const uint32_t my_random_count = rand() % 1000;
    for (uint32_t i = 0; i< my_random_count; ++i)
    {
        NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP();
    }
#endif

    ++m_nNumberOfSteps;

#if 0
    std::cerr << GetName() << "/" << "CountStep:"
        << " m_tmStopTimeStamp=" << m_tmStopTimeStamp
        << " m_tmStartTimeStamp=" << m_tmStartTimeStamp
        << " m_nNumberOfSteps=" << m_nNumberOfSteps
        << std::endl;
#endif

    timestamp_t tmSimTimeCurrent = tmSimulation; // GetTimingInterface()->GetTime();
    timestamp_t tmRealTimeCurrent = m_tmStopTimeStamp;

    // If it the first call, initialize
    if (tmSimulation == 0)
    {
        m_tmRealTimeExpected = tmRealTimeCurrent;
        m_tmSimTimeExpected = tmSimTimeCurrent;
        m_tmRealTimeStart = tmRealTimeCurrent;
        m_tmSimTimeStart = tmSimTimeCurrent;
    }

    timestamp_t tmRealTimeDiff = tmRealTimeCurrent - m_tmRealTimeExpected;
    timestamp_t tmSimTimeDiff = tmSimTimeCurrent - m_tmSimTimeExpected;

    timestamp_t tmRealTimeSinceStart = tmRealTimeCurrent - m_tmRealTimeStart;
    timestamp_t tmSimTimeSinceStart = tmSimTimeCurrent - m_tmSimTimeStart;

    timestamp_t tmRealSimDrift = tmRealTimeSinceStart - tmSimTimeSinceStart;

#ifdef REAL_TIME_DRIFT_VERBOSE_MODE
    std::cout << " === SimulationTime="; printimestamp_t(std::cout, tmSimulation);
    std::cout << " === RealTimeCurrent="; printimestamp_t(std::cout, tmRealTimeCurrent);
    std::cout << " === RealTimeExpected="; printimestamp_t(std::cout, m_tmRealTimeExpected);
    std::cout << " === RealTimeDiff="; printimestamp_t(std::cout, tmRealTimeDiff);
    std::cout << " === SimTimeCurrent="; printimestamp_t(std::cout, tmSimTimeCurrent);
    std::cout << " === SimTimeExpected="; printimestamp_t(std::cout, m_tmSimTimeExpected);
    std::cout << " === SimTimeDiff="; printimestamp_t(std::cout, m_tmResMinDiffSimTime);
    //std::cout << " === RealSimDrift="; printimestamp_t(std::cout, tmRealSimDrift);
    std::cout << std::endl;
#endif

    if (tmSimulation > 0)
    {
        m_tmResMinDiffRealSimDrift = std::min(m_tmResMinDiffRealSimDrift, abs(tmRealSimDrift));
    }
    m_tmResSumDiffRealSimDrift += abs(tmRealSimDrift);
    m_tmResQSumDiffRealSimDrift += static_cast<double>(tmRealTimeDiff*tmRealTimeDiff);
    m_tmResMaxDiffRealSimDrift = std::max(m_tmResMaxDiffRealSimDrift, abs(tmRealSimDrift));

    if (tmSimulation > 0)
    {
        m_tmResMinDiffSimTime = std::min(m_tmResMinDiffSimTime, abs(tmSimTimeDiff));
    }
    m_tmResSumDiffSimTime += abs(tmSimTimeDiff);
    m_tmResQSumDiffSimTime += static_cast<double>(tmSimTimeDiff*tmSimTimeDiff);
    m_tmResMaxDiffSimTime = std::max(m_tmResMaxDiffSimTime, abs(tmSimTimeDiff));

    if (tmSimulation > 0)
    {
        m_tmResMinDiffRealTime = std::min(m_tmResMinDiffRealTime, abs(tmRealTimeDiff));
    }
    m_tmResSumDiffRealTime += abs(tmRealTimeDiff);
    m_tmResQSumDiffRealTime += static_cast<double>(tmRealTimeDiff*tmRealTimeDiff);
    m_tmResMaxDiffRealTime = std::max(m_tmResMaxDiffRealTime, abs(tmRealTimeDiff));
#ifdef REAL_TIME_DRIFT_VERBOSE_MODE_XXX
    std::cout << " === tmRealTimeDiff="; printimestamp_t(std::cout, tmRealTimeDiff);
    std::cout << " --> m_tmResMinDiffRealTime="; printimestamp_t(std::cout, m_tmResMinDiffRealTime);
    std::cout << " --> m_tmResSumDiffRealTime="; printimestamp_t(std::cout, m_tmResSumDiffRealTime);
    std::cout << " --> m_tmResMaxDiffRealTime="; printimestamp_t(std::cout, m_tmResMaxDiffRealTime);
    std::cout << std::endl;
#endif


    m_tmRealTimeExpected += m_tmRealTimePeriod;
    m_tmSimTimeExpected += m_tmSimTimePeriod;
}
