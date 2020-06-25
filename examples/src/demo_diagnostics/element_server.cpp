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
#include "element_server.h"
#include "common.h"

cElementServer::cElementServer()
{
    //initialize member variables 
    m_bTestDone=false;
    m_hPing = NULL;
    m_hPong = NULL;
    m_pSamplePong = NULL;
};

cElementServer::~cElementServer()
{
    Destroy();
}

fep::Result cElementServer::Create(const fep::cModuleOptions& oModuleOptions)
{
    //call parent class
    RETURN_IF_FAILED(cModule::Create(oModuleOptions));
    return fep::ERR_NOERROR;
}

const char* cElementServer::GetParticipantName() const
{
    //get identifier
    return m_strParticipantName.c_str();
}


fep::Result cElementServer::ProcessIdleEntry(const fep::tState eOldState)
{
    if (fep::FS_STARTUP == eOldState)
    {
        GetStateMachine()->InitializeEvent();
    }
    else
    {
        if (NULL != m_hPing)
        {
            GetUserDataAccess()->UnregisterDataListener(this, m_hPing);
            GetSignalRegistry()->UnregisterSignal(m_hPing);
        }
        if (NULL != m_hPong)
        {
            GetSignalRegistry()->UnregisterSignal(m_hPong);
        }
        if (m_pSamplePong != NULL)
        {
            delete m_pSamplePong;
        }
    }
    return fep::ERR_NOERROR;
}

fep::Result cElementServer::ProcessInitializingEntry(const fep::tState eOldState)
{
    // register incoming signal
    fep::cUserSignalOptions oPingOptions(PING_SIGNAL_NAME, fep::SD_Input, "tPing");
    if (fep::isFailed(GetSignalRegistry()->RegisterSignal(oPingOptions,m_hPing)))
    {
        GetIncidentHandler()->InvokeIncident(-1501, fep::SL_Critical,
            "Could not register signal: Ping",NULL,0,NULL);
        GetStateMachine()->ErrorEvent();
        return fep::ERR_NOERROR;
    }

    GetUserDataAccess()->RegisterDataListener(this,m_hPing);

    // register outgoing signal
    fep::cUserSignalOptions oPongOptions(PONG_SIGNAL_NAME, fep::SD_Output, "tPing");
    if (fep::isFailed(GetSignalRegistry()->RegisterSignal(oPongOptions,m_hPong)))
    {
        GetIncidentHandler()->InvokeIncident(-1501, fep::SL_Critical,
            "Could not register signal: Pong",NULL,0,NULL);
        GetStateMachine()->ErrorEvent();
        return fep::ERR_NOERROR;
    }
    //allocate signal sample (for outgoing signal)
    GetUserDataAccess()->CreateUserDataSample(m_pSamplePong, m_hPong);
    GetStateMachine()->InitDoneEvent();
    return fep::ERR_NOERROR;
}

fep::Result cElementServer::ProcessReadyEntry(const fep::tState oldState)
{
    GetStateMachine()->StartEvent();
    return fep::ERR_NOERROR;
}


fep::Result cElementServer::ProcessStartupEntry(const fep::tState oldState)
{
    //fill in headers 
    double fVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR)  +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, fVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION,
        "Server FEP-Element:Part of the extended Diagnostics example.");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, fVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, 
        "f6c4273e-fa84-48ee-88fe-bfe27068957f");
    //set test property that will be attemted to be 
    //retrieved by client
     m_strTestProperty = "This is a test string";
    RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(ELEMENT_TEST_PROPERTY, m_strTestProperty));

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
    GetStateMachine()->StartupDoneEvent();
    return fep::ERR_NOERROR;
}

fep::Result cElementServer::ProcessErrorEntry(fep::tState oldState)
{
    //In this Demo we go directly to shutdown when
    //entering FS_IDLE from any other state than FS_STARTUP
    //therfore we go to IDLE with ErrorFixedEvent and
    // make use of the CleanUps that are implemented 
    // there.
    GetStateMachine()->ErrorFixedEvent();
    return fep::ERR_NOERROR;
}

fep::Result cElementServer::ProcessRunningEntry(const fep::tState eOldState)
{
    GetIncidentHandler()->InvokeIncident(-111, fep::SL_Info,
        "FEP-Server is in state \"Running\".\nServer-element is now ready for measurement\n",NULL,0,NULL);
    return fep::ERR_NOERROR;
}

fep::Result cElementServer::Update(const fep::IUserDataSample* poSample)
{
    if(poSample->GetSignalHandle() == m_hPing)
    {
        //indicate received signal from client
        GetIncidentHandler()->InvokeIncident(-111, fep::SL_Info,
            "Received Signal from Client!",NULL,0,NULL);
        *(int64_t*)m_pSamplePong->GetPtr() = 1;
        //answer ping signal with pong signal
        GetUserDataAccess()->TransmitData(m_pSamplePong,true);
        //set flag 
        m_bTestDone = true;
    }
    return fep::ERR_NOERROR;
}
bool cElementServer::IsDone()
{
    return m_bTestDone;
}

fep::Result cElementServer::TerminateServer()
{
    GetStateMachine()->StopEvent();
    return fep::ERR_NOERROR;
}
