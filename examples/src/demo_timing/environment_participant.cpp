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

#include "environment_participant.h"
#include "common.h"

#include <cmath>

// Base car model
cCarModel::cCarModel(const char* strSignalName, const char* strStepName)
    : m_strSignalName(strSignalName)
    , m_strStepName(strStepName)
    , m_hSignalOut(nullptr)
    , m_pDataSample(nullptr)
{
}

cCarModel::~cCarModel()
{
}

fep::Result cCarModel::Initialize(fep::ISignalRegistry* pSignalRegistry,
    fep::IUserDataAccess* pUserDataAccess, fep::ITiming* pTiming)
{
    fep::Result nResult = fep::ERR_NOERROR;

    m_pSignalRegistry = pSignalRegistry;
    m_pUserDataAccess = pUserDataAccess;
    m_pTiming = pTiming;

    return nResult;
}

fep::Result cCarModel::Finalize()
{
    fep::Result nResult = fep::ERR_NOERROR;

    m_pSignalRegistry = nullptr;
    m_pUserDataAccess = nullptr;
    m_pTiming = nullptr;

    return nResult;
}

fep::Result cCarModel::Configure()
{
    fep::Result nResult = fep::ERR_NOERROR;

    if (fep::isOk(nResult) && (nullptr == m_hSignalOut))
    {
        // Registering Step Listener output signal
        fep::cUserSignalOptions oObjectStateOptions(m_strSignalName.c_str(), fep::SD_Output, "tFEP_Examples_ObjectState");
        nResult = m_pSignalRegistry->RegisterSignal(oObjectStateOptions, m_hSignalOut);
    }

    if (fep::isOk(nResult) && (nullptr == m_pDataSample))
    {
        nResult = m_pUserDataAccess->CreateUserDataSample(m_pDataSample, m_hSignalOut);

        if (fep::isOk(nResult))
        {
            nResult |= m_pTiming->RegisterStepListener(m_strStepName.c_str(), fep::StepConfig(10 * 1000), &cCarModel::Simulate_caller, this);
        }
    }

    m_tmLastStep = 0;

    return nResult;
}

fep::Result cCarModel::Reset()
{
    fep::Result nResult = fep::ERR_NOERROR;

    nResult|= m_pTiming->UnregisterStepListener(m_strStepName.c_str());

    if (m_pDataSample)
    {
        delete m_pDataSample;
        m_pDataSample = nullptr;
    }

    // Unregister signals
    if (m_hSignalOut)
    {
        nResult |= m_pSignalRegistry->UnregisterSignal(m_hSignalOut);
        m_hSignalOut = nullptr;
    }

    return nResult;
}

void cCarModel::Simulate(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
{
    auto pData = reinterpret_cast<fep_examples::tFEP_Examples_ObjectState*>(m_pDataSample->GetPtr());

    if (m_tmLastStep > 0)
    {
        double diffTime = static_cast<double>(tmSimulation - m_tmLastStep) / 1000000.0;
        m_vel_x += m_acc_x * diffTime;
        m_vel_y += m_acc_y * diffTime;
        m_pos_x += m_vel_x * diffTime;
        m_pos_y += m_vel_y * diffTime;
    }

    pData->sPosInertial.f64X = m_pos_x;
    pData->sPosInertial.f64Y = m_pos_y;
    pData->sSpeedInertial.f64X = m_vel_x;
    pData->sSpeedInertial.f64Y = m_vel_y;

    // set simulation = time in sample
    pStepDataAccess->TransmitData(m_pDataSample);

    // Rember last call time
    m_tmLastStep = tmSimulation;
}


// Car model that accelerates/decelerates with sine function
cCarSin::cCarSin(const char* strSignalName, const char* strStepName)
    : cCarModel(strSignalName, strStepName)
{
}

cCarSin::~cCarSin()
{

}

void cCarSin::Simulate(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
{
    if (m_tmLastStep > 0)
    {
        double simTime = static_cast<double>(tmSimulation) / 1000000.0;
        m_acc_x = std::sin(simTime) * 5.0;
        m_acc_y = 0;
    }

    cCarModel::Simulate(tmSimulation, pStepDataAccess);
}

// EGO car model that uses driver ctrl signal for acceleration
cOwnCar::cOwnCar(const char* strSignalName, const char* strStepName)
    : cCarModel(strSignalName, strStepName)
    , m_pInDataSample(nullptr)
    , m_hSignalIn(nullptr)
{
}

cOwnCar::~cOwnCar()
{
}

void cOwnCar::Simulate(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
{
    fep::Result nResult = pStepDataAccess->CopyRecentData(m_hSignalIn, m_pInDataSample);

    if (fep::isOk(nResult))
    {
        tDriverCtrl* pCtrl = reinterpret_cast<tDriverCtrl*>(m_pInDataSample->GetPtr());

        m_acc_x = pCtrl->x_acc;
        m_acc_y = pCtrl->y_acc;
    }
    else
    {
        // No input yet 
        m_acc_x = 0;
        m_acc_y = 0;
    }

    cCarModel::Simulate(tmSimulation, pStepDataAccess);
}

fep::Result cOwnCar::Configure()
{
    fep::Result nResult = fep::ERR_NOERROR;

    if (fep::isOk(nResult) && (nullptr == m_hSignalIn))
    {
        fep::cUserSignalOptions oDriverCtrlOptions("DriverCommand", fep::SD_Input, "tDriverCtrl");
        nResult = m_pSignalRegistry->RegisterSignal(oDriverCtrlOptions, m_hSignalIn);
    }

    if (fep::isOk(nResult) && (nullptr == m_pInDataSample))
    {
        nResult |= m_pUserDataAccess->CreateUserDataSample(m_pInDataSample, m_hSignalIn);
    }

    if (fep::isOk(nResult))
    {
        nResult |= cCarModel::Configure();
    }

    return nResult;
}

fep::Result cOwnCar::Reset()
{
    fep::Result nResult = fep::ERR_NOERROR;

    if (m_pInDataSample)
    {
        delete m_pInDataSample;
        m_pInDataSample = nullptr;
    }

    // Unregister signals
    if (m_hSignalIn)
    {
        nResult |= m_pSignalRegistry->UnregisterSignal(m_hSignalIn);
        m_hSignalIn = nullptr;
    }

    if (fep::isOk(nResult))
    {
        nResult |= cCarModel::Reset();
    }

    return nResult;
}

// The environment participant publishing simulated car model data
cElementEnvironment::cElementEnvironment(const std::string& strTimingConfiguration)
    : oCarA("PositionCarA", "SimulateCarA")
    , oCarB("PositionCarB", "SimulateCarB")
    , oOwn("PositionOwn", "SimulateOwn")
    , m_strTimingConfiguration(strTimingConfiguration)
{
}

cElementEnvironment::~cElementEnvironment()
{
    Destroy();
}


fep::Result cElementEnvironment::ProcessStartupEntry(const fep::tState eOldState)
{
    fep::Result nResult = fep::ERR_NOERROR;

    // Set participant header information
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION, "Timing Demo Element Environment");
    double fFepVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, "1a5ec649-b45f-4d4c-9e08-163c612a5639");

    // Participants proceed to FS_IDLE
    GetStateMachine()->StartupDoneEvent();

    return nResult;
}

fep::Result cElementEnvironment::ProcessIdleEntry(const fep::tState eOldState)
{
    fep::Result nResult = fep::ERR_NOERROR;

    if (eOldState == fep::FS_STARTUP)
    {
        if (fep::isOk(nResult))
        {
            nResult |= oCarA.Initialize(GetSignalRegistry(), GetUserDataAccess(), GetTimingInterface());
            nResult |= oCarB.Initialize(GetSignalRegistry(), GetUserDataAccess(), GetTimingInterface());
            nResult |= oOwn.Initialize(GetSignalRegistry(), GetUserDataAccess(), GetTimingInterface());
        }
    }
    else
    {
        if (fep::isOk(nResult))
        {
            nResult |= oCarA.Reset();
            nResult |= oCarB.Reset();
            nResult |= oOwn.Reset();
        }
    }

    return nResult;
}

fep::Result cElementEnvironment::ProcessInitializingEntry(const fep::tState eOldState)
{
    fep::Result nResult = fep::ERR_NOERROR;

    {
        if (fep::isOk(nResult))
        {

            // Set up models
            nResult |= oOwn.Configure();
            nResult |= oCarA.Configure();
            nResult |= oCarB.Configure();

            if (fep::isFailed(nResult))
            {
                std::stringstream strMessage;
                strMessage
                    << "\"RegisterSignal\" or \"CreateUserDataSample\" or \"RegisterStepListener\" failed with error "
                    << nResult.getErrorLabel();
                RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nResult.getErrorCode()),
                    fep::SL_Critical_Global, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
            }
        }

        if (fep::isOk(nResult))
        {
            // Set initial data values
            oCarA.SetPosition(200, 0);
            oCarA.SetVelocity(20, 0);
            oCarA.SetAcceleration(0, 0);

            oCarB.SetPosition(100, 0);
            oCarB.SetVelocity(20, 0);
            oCarB.SetAcceleration(0, 0);

            // Own car is between and faster
            oOwn.SetPosition(170, 0);
            oOwn.SetVelocity(30, 0);
            oOwn.SetAcceleration(0, 0);
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
            fep::SL_Critical_Global, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
    }

    return nResult;
}

fep::Result cElementEnvironment::ProcessShutdownEntry(const fep::tState eOldState)
{
    fep::Result nResult = fep::ERR_NOERROR;

    if (eOldState == fep::FS_ERROR)
    {
        nResult |= oCarA.Reset();
        nResult |= oCarB.Reset();
        nResult |= oOwn.Reset();

        nResult |= oCarA.Finalize();
        nResult |= oCarB.Finalize();
        nResult |= oOwn.Finalize();
    }

    return nResult;
}

int main(int nArgc, const char* pArgv[])
{
    fep::cModuleOptions oModuleOptions("Environment");

    std::string strTimingConfiguration = "default_timing_configuration.xml";
    oModuleOptions.SetAdditionalOption(strTimingConfiguration, "-c", "--conf",
        "(Optional Argument) configuration file for timing (default: default_timing_configuration.xml)", "file");

    if (fep::isFailed(oModuleOptions.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }

    // create the elements
    cElementEnvironment oElement(strTimingConfiguration);
    if (fep::isFailed(oElement.Create(oModuleOptions)))
    {
        return 1;
    }

    return fep::isFailed(oElement.WaitForShutdown()) ? 1 : 0;
}
