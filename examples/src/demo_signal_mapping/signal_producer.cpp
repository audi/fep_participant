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

#include "signal_producer.h"
#include "example_ddl_types.h"
#include <iostream>

cSignalProducer::~cSignalProducer()
{
    Destroy();
    if (m_pSampleOrientation)
    {
        delete m_pSampleOrientation;
        m_pSampleOrientation = NULL;
    }
    if (m_pSamplePosition)
    {
        delete m_pSamplePosition;
        m_pSamplePosition = NULL;
    }
    if (m_pSampleObject)
    {
        delete m_pSampleObject;
        m_pSampleObject = NULL;
    }
}

fep::Result cSignalProducer::ProcessStartupEntry(const fep::tState eOldState)
{
    double fFepVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION, "Signal Mapping Receiver");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, "8aa96c91-b268-4264-ac55-c00746b3bd70");

    // here we register any signal descriptions that persists during the entire
    // Participant life cycle
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
        GetStateMachine()->StartupDoneEvent();
    }

    return fep::ERR_NOERROR;
}

fep::Result cSignalProducer::ProcessIdleEntry(const fep::tState eOldState)
{
    std::cout << "cSignalProducer idle\n";
    if (fep::FS_STARTUP != eOldState)
    {
        m_oTimer.stop();

        //Unregister all signals
        if (NULL != m_hObject)
        {
            GetSignalRegistry()->UnregisterSignal(m_hObject);
        }
        if (NULL != m_hPosition)
        {
            GetSignalRegistry()->UnregisterSignal(m_hPosition);
        }
        if (NULL != m_hOrientation)
        {
            GetSignalRegistry()->UnregisterSignal(m_hOrientation);
        }
    }

    return fep::ERR_NOERROR;
}

fep::Result cSignalProducer::ProcessInitializingEntry(const fep::tState eOldState)
{
    // register the output signal 'LightOrientation'
    fep::cUserSignalOptions oLightOrientationOptions("LightOrientation", fep::SD_Output, "tFEP_Examples_Coord");
    if (fep::isFailed(GetSignalRegistry()->RegisterSignal(oLightOrientationOptions, m_hOrientation)))
    {
        GetIncidentHandler()->InvokeIncident(-1501, fep::SL_Critical,
            "Couldn't register signal LightOrientation",NULL,0,NULL);
        GetStateMachine()->ErrorEvent();
        return fep::ERR_NOERROR;
    }

    // register the output signal 'LightPos'
    fep::cUserSignalOptions oLightPosOptions("LightPos", fep::SD_Output, "tFEP_Examples_PointCartesian");
    if (fep::isFailed(GetSignalRegistry()->RegisterSignal(oLightPosOptions, m_hPosition)))
    {
        GetIncidentHandler()->InvokeIncident(-1501, fep::SL_Critical,
            "Couldn't register signal LightPos",NULL,0,NULL);
        GetStateMachine()->ErrorEvent();
        return fep::ERR_NOERROR;
    }

    // register the output signal 'SourceObject'
    fep::cUserSignalOptions oSourceObjectOptions("SourceObject", fep::SD_Output, "tObject");
    if (fep::isFailed(GetSignalRegistry()->RegisterSignal(oSourceObjectOptions, m_hObject)))
    {
        GetIncidentHandler()->InvokeIncident(-1501, fep::SL_Critical,
            "Couldn't register signal SourceObject",NULL,0,NULL);
        GetStateMachine()->ErrorEvent();
        return fep::ERR_NOERROR;
    }

    // register the timer that will send out the samples
    m_oTimer.setCallback(&cSignalProducer::RunCyclic, *this);
    m_oTimer.setPeriod(1000 * 1000);
    m_nSampleCounter = 0;

    // preallocate the samples
    GetUserDataAccess()->CreateUserDataSample(m_pSampleOrientation, m_hOrientation);
    GetUserDataAccess()->CreateUserDataSample(m_pSamplePosition, m_hPosition);
    GetUserDataAccess()->CreateUserDataSample(m_pSampleObject, m_hObject);

    GetStateMachine()->InitDoneEvent();
    return fep::ERR_NOERROR;
}

fep::Result cSignalProducer::ProcessRunningEntry(const fep::tState eOldState)
{
    m_oTimer.start();
    std::cout << "cSignalProducer running\n";
    return fep::ERR_NOERROR;
}

fep::Result cSignalProducer::ProcessErrorEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(-1501, fep::SL_Critical,
        "Entered state FS_ERROR",NULL,0,NULL);
    return fep::ERR_NOERROR;
}

void cSignalProducer::RunCyclic()
{
    fep_examples::tFEP_Examples_Coord* pOrient = (fep_examples::tFEP_Examples_Coord*)m_pSampleOrientation->GetPtr();
    fep_examples::tFEP_Examples_PointCartesian* pPosition = (fep_examples::tFEP_Examples_PointCartesian*)m_pSamplePosition->GetPtr();

    // fill samples (with dummy data)
    pOrient->f64H = m_nSampleCounter;
    pOrient->f64P = m_nSampleCounter * 1.1;
    pOrient->f64R = m_nSampleCounter * 1.2;
    pPosition->f64X = m_nSampleCounter * 1.3;
    pPosition->f64Y = m_nSampleCounter * 1.4;
    pPosition->f64Z = m_nSampleCounter * 1.5;

    *(int32_t*)m_pSampleObject->GetPtr() = 7; // Animal

    // send samples
    GetUserDataAccess()->TransmitData(m_pSampleOrientation, true);
    GetUserDataAccess()->TransmitData(m_pSamplePosition, true);
    GetUserDataAccess()->TransmitData(m_pSampleObject, true);

    m_nSampleCounter++;
}
