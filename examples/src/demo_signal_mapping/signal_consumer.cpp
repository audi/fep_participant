/************************************************************************
 * Implementation of the signal mapping demo
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

#include "signal_consumer.h"
#include "example_ddl_types.h"
#include <iostream>

cSignalConsumer::~cSignalConsumer()
{
    Destroy();
}

fep::Result cSignalConsumer::ProcessStartupEntry(const fep::tState eOldState)
{
    double fFepVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION, "Signal Mapping Consumer");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, "643e3955-e9e6-492e-b82d-13c8ef613ac9");

    // we register any signal description here
    if (fep::isFailed(GetSignalRegistry()->RegisterSignalDescription("./mapping_example.description",
        fep::ISignalRegistry::DF_DESCRIPTION_FILE)))
    {
        GetIncidentHandler()->InvokeIncident(-1501, fep::SL_Critical,
            "Couldn't register signal description mapping_example.description. "
            "Please make sure it's found in the working directory.", NULL, 0, NULL);
        GetStateMachine()->ErrorEvent();
    }
    else
    {
        // the mapping configuration is also registered here since it 
        // needs to be registered before registering any of the mapped 
        // signals and persits over the entire participant life cycle
        if (fep::isFailed(GetSignalMapping()->RegisterMappingConfiguration("./mapping_example.map",
            fep::ISignalMapping::MF_MAPPING_FILE)))
        {
            GetIncidentHandler()->InvokeIncident(-1501, fep::SL_Critical,
                "Couldn't register mapping configuration mapping_example.map. "
                "Please make sure it's found in the working directory.", NULL, 0, NULL);
            GetStateMachine()->ErrorEvent();
        }
        else
        {
            GetStateMachine()->StartupDoneEvent();
        }      
    }

    return fep::ERR_NOERROR;
}

fep::Result cSignalConsumer::ProcessIdleEntry(const fep::tState eOldState)
{
    std::cout << "cSignalConsumer idle\n";
    if (fep::FS_STARTUP != eOldState)
    {
        //Unregister DataListeners (this should be done before unregistering the
        // according signals)
        GetUserDataAccess()->UnregisterDataListener(this, m_hObject);
        GetUserDataAccess()->UnregisterDataListener(this, m_hSignal);

        //Unregister all signals
        if (NULL != m_hObject)
        {
            GetSignalRegistry()->UnregisterSignal(m_hObject);
        }
        if (NULL != m_hSignal)
        {
            GetSignalRegistry()->UnregisterSignal(m_hSignal);
        }
    }

    return fep::ERR_NOERROR;
}

fep::Result cSignalConsumer::ProcessInitializingEntry(const fep::tState eOldState)
{
    // Here the input signal is registered. The element will recognize transparently that there
    // does exist valid mapping configuration for the signal and will therefore map it accordingly.
    fep::cUserSignalOptions oLightSourceOptions("LightSource", fep::SD_Input, "tFEP_Examples_LightSource");
    if (fep::isFailed(GetSignalRegistry()->RegisterSignal(oLightSourceOptions, m_hSignal)))
    {
        GetIncidentHandler()->InvokeIncident(-1501, fep::SL_Critical,
            "Couldn't register signal LightSource",NULL,0,NULL);
        GetStateMachine()->ErrorEvent();
        return fep::ERR_NOERROR;
    }

    fep::cUserSignalOptions oObjectOptions("Object", fep::SD_Input, "tObject");
    if (fep::isFailed(GetSignalRegistry()->RegisterSignal(oObjectOptions, m_hObject)))
    {
        GetIncidentHandler()->InvokeIncident(-1501, fep::SL_Critical,
            "Couldn't register signal Object",NULL,0,NULL);
        GetStateMachine()->ErrorEvent();
        return fep::ERR_NOERROR;
    }

    // Mapped signals behave just as if they were real, simply use the existing data listeners
    GetUserDataAccess()->RegisterDataListener(this, m_hSignal);
    GetUserDataAccess()->RegisterDataListener(this, m_hObject);

    GetStateMachine()->InitDoneEvent();
    return fep::ERR_NOERROR;
}

fep::Result cSignalConsumer::ProcessRunningEntry(const fep::tState eOldState)
{
    std::cout << "cSignalConsumer running\n";
    return fep::ERR_NOERROR;
}

fep::Result cSignalConsumer::ProcessErrorEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(-1501, fep::SL_Critical,
        "Entered state FS_ERROR",NULL,0,NULL);
    return fep::ERR_NOERROR;
}

fep::Result cSignalConsumer::Update(const fep::IUserDataSample* poSample)
{
    if(poSample->GetSignalHandle() == m_hSignal)
    {
        // simply output the received (mapped) sample, showing that the source signals were mapped correctly
        fep_examples::tFEP_Examples_LightSource* pLight = (fep_examples::tFEP_Examples_LightSource*)poSample->GetPtr();
        std::cout << "Mapped sample received:\n";
        std::cout << "  f64SimTime: " << pLight->f64SimTime << "\n";
        std::cout << "  ui32Id: "     << pLight->ui32Id << "\n";
        std::cout << "  ui8State: "   << (int)pLight->ui8State << "\n";
        std::cout << "  sPosIntertial.f64X: " << pLight->sPosIntertial.f64X << "\n";
        std::cout << "  sPosIntertial.f64Y: " << pLight->sPosIntertial.f64Y << "\n";
        std::cout << "  sPosIntertial.f64Z: " << pLight->sPosIntertial.f64Z << "\n";
        std::cout << "  sPosIntertial.f64H: " << pLight->sPosIntertial.f64H << "\n";
        std::cout << "  sPosIntertial.f64P: " << pLight->sPosIntertial.f64P << "\n";
        std::cout << "  sPosIntertial.f64R: " << pLight->sPosIntertial.f64R << "\n";
    }
    else if (poSample->GetSignalHandle() == m_hObject)
    {
        std::cout << "Mapped object received:\n";
        std::cout << "  Type: " << *(int32_t*)poSample->GetPtr() << "\n";
    }

    return fep::ERR_NOERROR;
}
