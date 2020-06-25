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

#include "observer_participant.h"
#include "common.h"

#include <string.h>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

cElementObserver::cElementObserver(const std::string& strTimingConfiguration)
    : m_hSignalCarAIn(nullptr)
    , m_hSignalCarBIn(nullptr)
    , m_hSignalOwnIn(nullptr)
    , m_pDataSampleCarA(nullptr)
    , m_pDataSampleCarB(nullptr)
    , m_pDataSampleOwn(nullptr)
    , m_strTimingConfiguration(strTimingConfiguration)
{
}

cElementObserver::~cElementObserver()
{
    Destroy();
}

fep::Result cElementObserver::RegisterSignalAndCreateSample(const char* strSignalName, const char* strSignalType,
    const fep::tSignalDirection nSignalDirection, handle_t& oSignalHandle, fep::IUserDataSample*& pDataSample)
{
    fep::Result nExprResult = fep::ERR_NOERROR;
    
    fep::cUserSignalOptions oSignalOptions(strSignalName, nSignalDirection, strSignalType);
    nExprResult = GetSignalRegistry()->RegisterSignal(oSignalOptions, oSignalHandle);
    if (isFailed(nExprResult))
    {
        std::stringstream strMessage; \
            strMessage << "\"RegisterSignal\" failed with error " << nExprResult.getErrorLabel();
        RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nExprResult.getErrorCode()),
            fep::SL_Critical, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
    }
    else
    {
        nExprResult = GetUserDataAccess()->CreateUserDataSample(pDataSample, oSignalHandle);
        if (isFailed(nExprResult))
        {
            std::stringstream strMessage; \
                strMessage << "\"RegisterSignal\" failed with error " << nExprResult.getErrorLabel();
            RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nExprResult.getErrorCode()),
                fep::SL_Critical, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
        }
    }

    return nExprResult;
}

fep::Result cElementObserver::ProcessStartupEntry(const fep::tState eOldState)
{
    // set participant header information
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION, "Timing Demo Element Front Sensor");
    double fFepVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, "e8282682-c4cf-4726-832c-d48f7d8eb094");

    GetStateMachine()->StartupDoneEvent();

    return fep::ERR_NOERROR;
}

fep::Result cElementObserver::ProcessIdleEntry(const fep::tState eOldState)
{
    fep::Result nResult = fep::ERR_NOERROR;

    if (eOldState != fep::FS_STARTUP)
    {
        GetTimingInterface()->UnregisterStepListener("ObserveScene");

        // Delete samples
        if (m_pDataSampleCarA)
        {
            delete m_pDataSampleCarA;
            m_pDataSampleCarA = nullptr;
        }
        if (m_pDataSampleCarB)
        {
            delete m_pDataSampleCarB;
            m_pDataSampleCarB = nullptr;
        }
        if (m_pDataSampleOwn)
        {
            delete m_pDataSampleOwn;
            m_pDataSampleOwn = nullptr;
        }

        // Unregister signals
        if (m_hSignalCarAIn)
        {
            GetSignalRegistry()->UnregisterSignal(m_hSignalCarAIn);
            m_hSignalCarAIn = nullptr;
        }
        if (m_hSignalCarBIn)
        {
            GetSignalRegistry()->UnregisterSignal(m_hSignalCarBIn);
            m_hSignalCarBIn = nullptr;
        }
        if (m_hSignalOwnIn)
        {
            GetSignalRegistry()->UnregisterSignal(m_hSignalOwnIn);
            m_hSignalOwnIn = nullptr;
        }
    }

    return nResult;
}

fep::Result cElementObserver::ProcessInitializingEntry(const fep::tState eOldState)
{
    fep::Result nResult = fep::ERR_NOERROR;

    {
        if (isOk(nResult))
        {
            // Register signals and create samples
            nResult |= RegisterSignalAndCreateSample("PositionCarA", "tFEP_Examples_ObjectState", fep::SD_Input, m_hSignalCarAIn, m_pDataSampleCarA);
            nResult |= RegisterSignalAndCreateSample("PositionCarB", "tFEP_Examples_ObjectState", fep::SD_Input, m_hSignalCarBIn, m_pDataSampleCarB);
            nResult |= RegisterSignalAndCreateSample("PositionOwn", "tFEP_Examples_ObjectState", fep::SD_Input, m_hSignalOwnIn, m_pDataSampleOwn);
        }

        if (isOk(nResult))
        {
            // register step listener, where all cyclic work is done
            fep::StepConfig step_config(100 * 1000);

            nResult = GetTimingInterface()->RegisterStepListener("ObserveScene", step_config, &cElementObserver::ObserveScene_caller, this);
            if (isFailed(nResult))
            {
                std::stringstream strMessage; \
                    strMessage << "\"RegisterStepListener\" failed with error " << nResult.getErrorLabel();
                RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nResult.getErrorCode()),
                    fep::SL_Critical, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
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
            fep::SL_Critical, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
    }

    return nResult;
}

fep::Result cElementObserver::ProcessShutdownEntry(const fep::tState eOldState)
{
    fep::Result nResult = fep::ERR_NOERROR;

    if (eOldState == fep::FS_ERROR)
    {
        // Unregister listener
        GetTimingInterface()->UnregisterStepListener("ObserveScene");

        // Delete samples
        if (m_pDataSampleCarA)
        {
            delete m_pDataSampleCarA;
            m_pDataSampleCarA = nullptr;
        }
        if (m_pDataSampleCarB)
        {
            delete m_pDataSampleCarB;
            m_pDataSampleCarB = nullptr;
        }
        if (m_pDataSampleOwn)
        {
            delete m_pDataSampleOwn;
            m_pDataSampleOwn = nullptr;
        }

        // Unregister signals
        if (m_hSignalCarAIn)
        {
            GetSignalRegistry()->UnregisterSignal(m_hSignalCarAIn);
            m_hSignalCarAIn = nullptr;
        }
        if (m_hSignalCarBIn)
        {
            GetSignalRegistry()->UnregisterSignal(m_hSignalCarBIn);
            m_hSignalCarBIn = nullptr;
        }
        if (m_hSignalOwnIn)
        {
            GetSignalRegistry()->UnregisterSignal(m_hSignalOwnIn);
            m_hSignalOwnIn = nullptr;
        }
    }

    return fep::ERR_NOERROR;
}

void cElementObserver::ObserveScene(std::int64_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
{
    pStepDataAccess->CopyRecentData(m_hSignalCarAIn, m_pDataSampleCarA);
    pStepDataAccess->CopyRecentData(m_hSignalCarBIn, m_pDataSampleCarB);
    pStepDataAccess->CopyRecentData(m_hSignalOwnIn,  m_pDataSampleOwn);

    auto pCarA = reinterpret_cast<fep_examples::tFEP_Examples_ObjectState*>(m_pDataSampleCarA->GetPtr());
    auto pCarB = reinterpret_cast<fep_examples::tFEP_Examples_ObjectState*>(m_pDataSampleCarB->GetPtr());
    auto pOwn  = reinterpret_cast<fep_examples::tFEP_Examples_ObjectState*>(m_pDataSampleOwn->GetPtr());

    double x_dist_A = pCarA->sPosInertial.f64X - pOwn->sPosInertial.f64X;
    double x_dist_B = pOwn->sPosInertial.f64X - pCarB->sPosInertial.f64X;

    // ASCII Art Visualize:
    //   O: Own Car
    int i_dist_A = static_cast<int>(x_dist_A) / 2; 
    bool is_crash = false;
    if (i_dist_A < 0)
    {
        is_crash = true;
        i_dist_A = 0;
    }
    if (i_dist_A > 50) i_dist_A = 50;
    int i_dist_B = static_cast<int>(x_dist_B) / 2;
    if (i_dist_B < 0) i_dist_B = 0;
    if (i_dist_B > 50) i_dist_B = 50;

    static char buffer[103];
    if (is_crash)
    {
        memset(buffer, '_', sizeof(buffer));
        buffer[48] = 'B';
        buffer[49] = 'O';
        buffer[50] = 'O';
        buffer[51] = 'M';
        buffer[52] = '!';
    }
    else
    {
        memset(buffer, '_', sizeof(buffer));
        buffer[101] = '\n';
        buffer[102] = '\0';
        buffer[50 - i_dist_B] = 'B';
        buffer[50 + i_dist_A] = 'A';
        buffer[50] = 'O';
    }

#ifdef WIN32
    _write
#else
    write
#endif
        (1, buffer, sizeof(buffer) - 1);
}

int main(int nArgc, const char* pArgv[])
{
    fep::cModuleOptions oModuleOptions("Observer");

    std::string strTimingConfiguration = "default_timing_configuration.xml";
    oModuleOptions.SetAdditionalOption(strTimingConfiguration, "-c", "--conf",
        "(Optional Argument) configuration file for timing (default: default_timing_configuration.xml)", "file");

    if (isFailed(oModuleOptions.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }

    // create the elements
    cElementObserver oElement(strTimingConfiguration);
    if (isFailed(oElement.Create(oModuleOptions)))
    {
        return 1;
    }

    return isFailed(oElement.WaitForShutdown()) ? 1 : 0;
}
