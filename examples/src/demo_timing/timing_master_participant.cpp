/************************************************************************
 * Implementation of the timing demo
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
 */

#include "timing_master_participant.h"
#include "common.h"
#include <iostream>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cManualStepTrigger::cManualStepTrigger()
    : _step_trigger_listener(nullptr)
{
}

fep::Result cManualStepTrigger::RegisterTrigger(const timestamp_t cycle_time, fep::IStepTriggerListener* pStepTriggerListener)
{
    // Valid arguments ?
    if ((cycle_time <= 0) || (nullptr == pStepTriggerListener))
    {
        return fep::ERR_INVALID_ARG;
    }

    // Already registered ?
    if (_step_trigger_listener)
    {
        return fep::ERR_UNEXPECTED;
    }

    _step_trigger_listener = pStepTriggerListener;

    return fep::ERR_NOERROR;
}

fep::Result cManualStepTrigger::UnregisterTrigger(fep::IStepTriggerListener* pStepTriggerListener)
{
    // Valid arguments ?
    if (nullptr == pStepTriggerListener)
    {
        return fep::ERR_INVALID_ARG;
    }

    // Registered ?
    if (_step_trigger_listener != pStepTriggerListener)
    {
        return fep::ERR_UNEXPECTED;
    }

    _step_trigger_listener = nullptr;

    return fep::ERR_NOERROR;
}

timestamp_t cManualStepTrigger::triggerNextStep()
{
    timestamp_t result;

    if (_step_trigger_listener)
    {
        result= _step_trigger_listener->Trigger();
    }
    else
    {
        result = -1;
    }

    return result;
}

cTimingMasterElement::cTimingMasterElement(MasterMode eMode, double fScale)
    : m_eMode(eMode)
    , m_fScale(fScale)
{
}

cTimingMasterElement::~cTimingMasterElement()
{
    Destroy();
}

fep::Result cTimingMasterElement::ProcessStartupEntry(const fep::tState eOldState)
{
    fep::Result result = fep::ERR_NOERROR;

    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION, "Timing Demo Master");
    double fFepVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, "412a41ac-0060-41b6-b74f-a3d20c51e6a6");

    // This element is the timing master
    GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, GetName());

    // set time scale for internal clock
    GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, m_fScale);

    // Set modes
    switch (m_eMode)
    {
    case  UNDEFINED_MODE:
        result = fep::ERR_UNEXPECTED;
        break;
    case AFAP_MODE:
        GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, "AFAP");
        break;
    case SYSTEM_TIME_MODE:
        GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, "SYSTEM_TIME");
        GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, m_fScale);
        break;
    case EXTERNAL_CLOCK_MODE:
        GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, "EXTERNAL_CLOCK");
        break;
    case USER_IMPLEMENTATION_MODE:
        GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, "USER_IMPLEMENTATION");
        GetTimingMaster()->SetStepTrigger(&m_oManualStepTrigger);
        GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_CLIENT_TIMEOUT, 0);
        break;
    }

    GetStateMachine()->StartupDoneEvent();

    return result;
}

fep::Result cTimingMasterElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    GetStateMachine()->InitDoneEvent();

    return fep::ERR_NOERROR;
}


timestamp_t cTimingMasterElement::triggerNextStep()
{
    return m_oManualStepTrigger.triggerNextStep();
}

int main(int nArgc, const char* pArgv[])
{
    fep::cModuleOptions oModuleOptions("TimingMaster");

    std::string strMode = std::string();
    oModuleOptions.SetAdditionalOption(strMode, "-m", "--mode",
        "(Required Argument) set timing master mode. Available modes are: \n" 
        "     0: AFAP (As Fast As Possible) mode.\n"
        "     1: System Time mode.\n"
        "     2: External Clock Mode.\n"
        "     3: User Implementation (using Manual Trigger).", "int");


    std::string strScale = std::string();
    oModuleOptions.SetAdditionalOption(strScale, "-s", "--scale",
        "(Optional Argument) Only for \"System Time Mode\"!\n"
        "Scale simulation execution relative to wall clock.\n" 
        "e.g. 2.0 will double the simulation speed (\"faster\").\n"
        "e.g. 0.5 will run the the simulation at half speed (\"slower\").", "float");

    if (fep::isFailed(oModuleOptions.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }

    MasterMode eMode = UNDEFINED_MODE;
    double fScale = 1.0;

    if (!strMode.empty())
    {
        const int nMode = atoi(strMode.c_str());
        switch (nMode)
        {
        case 0:
            eMode = AFAP_MODE;
            break;
        case 1:
            eMode = SYSTEM_TIME_MODE;
            break;
        case 2:
            eMode = EXTERNAL_CLOCK_MODE;
            break;
        case 3:
            eMode = USER_IMPLEMENTATION_MODE;
            break;
        default:
            eMode = UNDEFINED_MODE;
            break;
        }
    }
    if (!strScale.empty())
    {
        fScale = atof(strScale.c_str());
    }

    if (eMode == UNDEFINED_MODE)
    {
        std::cerr << "Error: Invalid mode value. See usage for allowed values." << std::endl;
        oModuleOptions.PrintHelp();
        return 1;
    }

    if (eMode == SYSTEM_TIME_MODE)
    {
        if (fScale <= 0.0)
        {
            std::cerr << "Error: Invalid scale value: " << fScale << ". See usage for allowed values." << std::endl;
            oModuleOptions.PrintHelp();
            return 1;
        }
    }

    // create the elements
    cTimingMasterElement oElement(eMode, fScale);
    fep::Result result = oElement.Create(oModuleOptions);
    if (fep::isFailed(result))
    {
        std::cerr << "Internal Error: Failed to create Element." << std::endl;
        std::cerr << result.getErrorLabel() << ": " << result.getDescription() << std::endl;
        return 1;
    }

    if (eMode == USER_IMPLEMENTATION_MODE)
    {
        // Run
        std::cout << std::endl;
        std::cout << "!!! Manual mode is activated. Please press enter to progress. !!!" << std::endl;
        std::cout << std::endl;

        while (oElement.GetStateMachine()->GetState() != fep::FS_SHUTDOWN)
        {
            if (oElement.GetStateMachine()->GetState() == fep::FS_RUNNING)
            {
#ifdef WIN32
                std::getchar();
#else
                getchar();
#endif
                std::cout << "Sending Trigger ... ";
                std::cout << "SimTime=oElement=" << oElement.triggerNextStep();
                std::cout << ". Done." << std::endl;
            }
        }
    }

    return fep::isFailed(oElement.WaitForShutdown()) ? 1 : 0;
}
