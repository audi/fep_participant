/**

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
#include <fep_participant_sdk.h>
#include <a_util/system.h>
#include "element_client.h"
#include "common.h"

cElementClient::cElementClient()
{
    //initialize member variables 
    m_bPropertytest = false;
    m_bSignaltest = false;
    m_bPropertytestDone = false;
    m_bSignaltestDone = false;
    m_bRemotePropertyReceived = false;
    m_nAttemptsPropertyTest = 0;
    m_nAttemptsSignalTest = 0;
    m_bSignaltestWasStarted = false;
    m_hPing = NULL;
    m_hPong = NULL;
    m_pSamplePing = NULL;
};

cElementClient::~cElementClient()
{
}

fep::Result cElementClient::Create(const fep::cModuleOptions& oModuleOptions)
{
    m_strParticipant = oModuleOptions.GetParticipantName();
    //call parent class
    RETURN_IF_FAILED(cModule::Create(oModuleOptions));
    return fep::ERR_NOERROR;
}

const char* cElementClient::GetParticipantName() const
{
    //get identifier
    return m_strParticipant.c_str();
}

fep::Result cElementClient::ProcessIdleEntry(const fep::tState eOldState)
{
    if (eOldState == fep::FS_STARTUP)
    {
        //register signal description
        if (fep::isFailed(GetSignalRegistry()->RegisterSignalDescription(DDL_DESCRIPTION,
            fep::ISignalRegistry::DF_DESCRIPTION_FILE)))
        {
            //notify failure and go to error state
            GetIncidentHandler()->InvokeIncident(-1501, fep::SL_Critical,
                "Could not register signal description file! The demo_diagnostics.description file needs "
                "to be placed in the descriptions-folder which needs to be placed in your working directory.", NULL, 0, NULL);
            GetStateMachine()->ErrorEvent();
            return fep::ERR_NOERROR;
        }
        GetStateMachine()->InitializeEvent();
    }
    else
    {
        m_oTimer.stop();
        if (NULL != m_hPong)
        {
            GetUserDataAccess()->UnregisterDataListener(this, m_hPong);
            GetSignalRegistry()->UnregisterSignal(m_hPong);
        }
        if (NULL != m_hPing)
        {
            GetSignalRegistry()->UnregisterSignal(m_hPing);
        }

        if (NULL != m_pSamplePing)
        {
            delete m_pSamplePing;
        }
    }

    return fep::ERR_NOERROR;
}

fep::Result cElementClient::ProcessInitializingEntry(const fep::tState eOldState){
    //Register Signal (outgoing ping)
    fep::cUserSignalOptions oPingOptions(PING_SIGNAL_NAME, fep::SD_Output, "tPing");
    if (fep::isFailed(GetSignalRegistry()->RegisterSignal(oPingOptions, m_hPing)))
    {
        GetIncidentHandler()->InvokeIncident(-1501, fep::SL_Critical,
            "Could not register signal: Ping",NULL,0,NULL);
        GetStateMachine()->ErrorEvent();
        return fep::ERR_NOERROR;
    }

    m_oTimer.setCallback(&cElementClient::RunCyclic, *this);
    m_oTimer.setPeriod(1000 * 100);

    //allocate signal sample
    fep::Result nResSampleCreate = GetUserDataAccess()->CreateUserDataSample(m_pSamplePing, m_hPing);

    //Register Signal (incoming ping)
    fep::cUserSignalOptions oPongOptions(PONG_SIGNAL_NAME, fep::SD_Input, "tPing");
    if (fep::isFailed(GetSignalRegistry()->RegisterSignal(oPongOptions, m_hPong)))
    {
        GetIncidentHandler()->InvokeIncident(-1501, fep::SL_Critical,
            "Could not register signal: Pong",NULL,0,NULL);
        GetStateMachine()->ErrorEvent();
        return fep::ERR_NOERROR;
    }
    GetUserDataAccess()->RegisterDataListener(this,m_hPong);
    GetStateMachine()->InitDoneEvent();
    return fep::ERR_NOERROR;
}

fep::Result cElementClient::ProcessReadyEntry(const fep::tState oldState)
{
    GetStateMachine()->StartEvent();
    return fep::ERR_NOERROR;
}

fep::Result cElementClient::ProcessRunningEntry(const fep::tState oldState)
{
    m_oTimer.start();
    return fep::ERR_NOERROR;
}

fep::Result cElementClient::ProcessStartupEntry(const fep::tState oldState)
{
    //fill headers 
    double fVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR)  +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, fVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION, 
        "Client FEP-Element:Part of the Diagnostics example.");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, fVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, "13b8c487-6196-44ce-a062-abb511a60ce4");

    GetStateMachine()->StartupDoneEvent();
    return fep::ERR_NOERROR;
}

fep::Result cElementClient::ProcessErrorEntry(fep::tState oldState)
{
    //In this Demo we go directly to shutdown when
    //entering FS_IDLE from any other state than FS_STARTUP
    //therfore we go to IDLE with ErrorFixedEvent and
    // make use of the CleanUps that are implemented 
    // there.
    GetStateMachine()->ErrorFixedEvent();
    return fep::ERR_NOERROR;
}

void cElementClient::RunCyclic()
{
    if (!m_bSignaltestDone)
    {
        if (m_bSignaltest)
        {
            //perform roundtrip time measurement
            GetIncidentHandler()->InvokeIncident(-111, fep::SL_Info,
                "Running signal test...",NULL,0,NULL);
            *(int64_t*)m_pSamplePing->GetPtr() = 1;
            m_tmSend = a_util::system::getCurrentMicroseconds();
            //send signal 
            GetUserDataAccess()->TransmitData(m_pSamplePing, true);
            //deactivate sending of further signals
            m_bSignaltest = false;
        }
        if (m_bSignaltestWasStarted)
        {
            //end test if server is not responding with pong signal
            //within a 100 cycles
            ++m_nAttemptsSignalTest;
            if(m_nAttemptsSignalTest > 100)
            {
                //set flag that signaltest is done 
                m_bSignaltestDone = true;
                m_tmReceived = 0;
            }
        }
    }
    if (m_bPropertytest)
    {
        //perform remote property retrieval test
        fep::IProperty* poPtr = NULL;
        fep::Result nRes = GetPropertyTree()->GetRemoteProperty(FEP_SERVER_IDENTIFIER,
            ELEMENT_TEST_PROPERTY, &poPtr, m_tmTimeout);
        if (fep::isFailed(nRes))
        {
            //indicate failure
            GetIncidentHandler()->InvokeIncident(-1502, fep::SL_Info,
                "Could not retrieve remote Property",NULL,0,NULL);
            ++m_nAttemptsPropertyTest;
            if (m_nAttemptsPropertyTest > 5)
            {
                //stop test after 5 attempts
                m_bPropertytest = false;
                m_bPropertytestDone = true;
            }
            return;
        }
        //indicate successful remote property retrieval
        GetIncidentHandler()->InvokeIncident(-111, fep::SL_Info,
            "Received remote property!",NULL,0,NULL);
        //set flags accordingly 
        m_bRemotePropertyReceived = true;
        m_bPropertytest = false;
        m_bPropertytestDone = true;
    }
}

fep::Result cElementClient::Update(const fep::IUserDataSample* poSample)
{
    //receives signal pong and measures roundtrip time
    if (poSample->GetSignalHandle() == m_hPong)
    {
        m_tmReceived = a_util::system::getCurrentMicroseconds();
        GetIncidentHandler()->InvokeIncident(-111, fep::SL_Info,
            "Received Signal from Server!",NULL,0,NULL);
        m_tmRoundtripTime = ( m_tmReceived - m_tmSend );
        std::ostringstream  rtt_string;
        rtt_string << "Roundtrip time for signal is "<< m_tmRoundtripTime << " micro-seconds.";
        GetIncidentHandler()->InvokeIncident(-111, fep::SL_Info,
            rtt_string.str().c_str(),NULL,0,NULL);
        //set flag that signaltest is done
        m_bSignaltestDone=true;
    }
    return fep::ERR_NOERROR;
}
fep::Result cElementClient::TerminateClient()
{
    GetStateMachine()->StopEvent();
    return fep::ERR_NOERROR;
}

fep::Result cElementClient::SetSignaltest(bool signaltest)
{
    if (signaltest)
    {
        m_bSignaltestWasStarted = true;
    }
    m_bSignaltest = signaltest;
    return fep::ERR_NOERROR;
}
bool cElementClient::GetPropertytestDone()
{
    return m_bPropertytestDone;
}
bool cElementClient::GetSignaltestDone()
{
    return m_bSignaltestDone;
}
bool cElementClient::GetRemotePropertyReceived()
{
    return m_bRemotePropertyReceived;
}
timestamp_t cElementClient::GetRoundTripTime()
{
    return m_tmRoundtripTime;
}
fep::Result cElementClient::SetPropertytest(bool propertytest, timestamp_t tmTimeout)
{
    m_bPropertytest = propertytest;
    m_tmTimeout = tmTimeout;
    return fep::ERR_NOERROR;
}
