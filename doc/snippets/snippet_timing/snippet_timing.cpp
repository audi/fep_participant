/**
*  Implementation of an exemplary FEP Timing Client Participant
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

#include "snippet_timing.h"

using namespace fep;

fep::Result cTimingSnippetElement::ProcessStartupEntry(const fep::tState eOldState)
{
    return ERR_NOERROR;
}


fep::Result cTimingSnippetElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    fep::Result nResult = ERR_NOERROR;

//! [StepListenerRegistration]
// A Step Listener can be registered by providing the name of the Step Listener, a task cycle time, the callback function pointer and a pointer to the instance of the callback
nResult = GetTimingInterface()->RegisterStepListener("SimpleListener", StepConfig(100 * 1000), 
    cTimingSnippetElement::SimpleListenerCallbackFunc_caller, this);
if (isFailed(nResult))
{
// Error handling - for example publishing an error incident
GetIncidentHandler()->InvokeIncident(1, fep::SL_Critical_Global, "Error during Step Listener registration", "ExampleElement", 32, "snippet_timing.cpp");
}
//! [StepListenerRegistration]

    if (isOk(nResult))
    {
//! [StepListenerConfiguration]
// Possible configuration properties to parameterize the Step Listener

// The simulation time period with which the Step Listener callback will be triggered
StepConfig oConfiguration(100 * 1000);
oConfiguration.m_cycleTime_sim_us = 100 * 1000;
// The maximum real waiting time the Step Listener will wait for valid input data of its configured inputs
oConfiguration.m_maxInputWaittime_us = 50 * 1000;
// The maxmimum real operational time the Step Listener may take to complete its computation
oConfiguration.m_maxRuntime_us = 50 * 1000;
// The strategy to be used when the Step Listener takes longer than the configured real operational time for computation
oConfiguration.m_runtimeViolationStrategy = TS_WARN_ABOUT_RUNTIME_VIOLATION;

// Input signals required by the Step Listener may be configured
InputConfig oInputConfig;
// Simulated delay in simulation time that will be applied to received samples
oInputConfig.m_delay_sim_us = 0;
// Maximum simulation time age that the input will be considered valid
oInputConfig.m_validAge_sim_us = 200 * 1000;
// The strategy to be used when no valid input data exists for the input
oInputConfig.m_inputViolationStrategy = IS_WARN_ABOUT_INPUT_VALIDITY_VIOLATION;

// Create entry with name of the input signal
std::pair<std::string, InputConfig> oInputEntry;
oInputEntry.first = "InputSignal";
oInputEntry.second = oInputConfig;
// Insert into configuration map of inputs
oConfiguration.m_inputs.insert(oInputEntry);

// Output signals published by the Step Listener may be configured
std::pair<std::string, OutputConfig> oOutputEntry;
oOutputEntry.first = "OutputSignal";
oConfiguration.m_outputs.insert(oOutputEntry);
        
nResult = GetTimingInterface()->RegisterStepListener("ConfiguredListener", oConfiguration, 
    cTimingSnippetElement::ConfiguredListenerCallbackFunc_caller, this);
if (isFailed(nResult))
{
// Error handling - for example publishing an error incident
GetIncidentHandler()->InvokeIncident(2, fep::SL_Critical_Global, "Error during Step Listener registration", "ExampleElement", 77, "snippet_timing.cpp");
}
//! [StepListenerConfiguration]
    }

    return ERR_NOERROR;
}

//![UserCallback]
void cTimingSnippetElement::SimpleListenerCallbackFunc_caller(void* pInstance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
{
    // Redirection from static const method to implementation
    reinterpret_cast<cTimingSnippetElement*>(pInstance)->SimpleListenerCallbackFunc(tmSimulation, pStepDataAccess);
}
void cTimingSnippetElement::SimpleListenerCallbackFunc(timestamp_t tmSimulation, fep::IStepDataAccess * pStepDataAccess)
{
    // Implementation
}
//![UserCallback]

void cTimingSnippetElement::ConfiguredListenerCallbackFunc_caller(void* pInstance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
{
    reinterpret_cast<cTimingSnippetElement*>(pInstance)->ConfiguredListenerCallbackFunc(tmSimulation, pStepDataAccess);
}

void cTimingSnippetElement::ConfiguredListenerCallbackFunc(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
{

}

fep::Result cTimingSnippetElement::ProcessIdleEntry(const fep::tState eOldState)
{
    return ERR_NOERROR;
}


