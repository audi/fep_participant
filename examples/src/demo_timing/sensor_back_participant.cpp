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

#include "sensor_back_participant.h"
#include "common.h"

cElementSensorBack::cElementSensorBack(const std::string& strTimingConfiguration)
    : m_hSignalCarBIn(nullptr)
    , m_hSignalOwnIn(nullptr)
    , m_pDataSampleCarB(nullptr)
    , m_pDataSampleOwn(nullptr)
    , m_tmLastStep(0)
    , m_strTimingConfiguration(strTimingConfiguration)
{
}

cElementSensorBack::~cElementSensorBack()
{
    Destroy();
}

fep::Result cElementSensorBack::RegisterSignalAndCreateSample(const char* strSignalName, const char* strSignalType,
    const fep::tSignalDirection nSignalDirection, handle_t& oSignalHandle, fep::IUserDataSample*& pDataSample)
{
    fep::Result nExprResult = fep::ERR_NOERROR;

     fep::cUserSignalOptions oSignalOptions(strSignalName, nSignalDirection, strSignalType);
    nExprResult = GetSignalRegistry()->RegisterSignal(oSignalOptions, oSignalHandle);
    if (fep::isFailed(nExprResult))
    {
        std::stringstream strMessage; \
            strMessage << "\"RegisterSignal\" failed with error " << nExprResult.getErrorLabel();
        RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nExprResult.getErrorCode()),
            fep::SL_Critical, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
    }
    else
    {
        nExprResult = GetUserDataAccess()->CreateUserDataSample(pDataSample, oSignalHandle);
        if (fep::isFailed(nExprResult))
        {
            std::stringstream strMessage; \
                strMessage << "\"RegisterSignal\" failed with error " << nExprResult.getErrorLabel();
            RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nExprResult.getErrorCode()),
                fep::SL_Critical, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
        }
    }

    return nExprResult;
}

fep::Result cElementSensorBack::ProcessStartupEntry(const fep::tState eOldState)
{
    // set participant header information
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION, "Timing Demo Element Back Sensor");
    double fFepVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, "0016a57b-8eef-4b88-b3b9-83872315d3a7");

    GetStateMachine()->StartupDoneEvent();

    return fep::ERR_NOERROR;
}

fep::Result cElementSensorBack::ProcessIdleEntry(const fep::tState eOldState)
{
    fep::Result nResult = fep::ERR_NOERROR;

    if (eOldState != fep::FS_STARTUP)
    {
        // Unregister listener
        GetTimingInterface()->UnregisterStepListener("CalculateBackDistance");

        // Delete samples
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
        if (m_pDataSampleOut)
        {
            delete m_pDataSampleOut;
            m_pDataSampleOut = nullptr;
        }

        // Unregister signals
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
        if (m_hSignalOut)
        {
            GetSignalRegistry()->UnregisterSignal(m_hSignalOut);
            m_hSignalOut = nullptr;
        }
    }

    return nResult;
}

fep::Result cElementSensorBack::ProcessInitializingEntry(const fep::tState eOldState)
{
    fep::Result nResult = fep::ERR_NOERROR;

    {
        if (fep::isOk(nResult))
        {
            // Register signals and create samples
            nResult |= RegisterSignalAndCreateSample("PositionCarB", "tFEP_Examples_ObjectState", fep::SD_Input, m_hSignalCarBIn, m_pDataSampleCarB);
            nResult |= RegisterSignalAndCreateSample("PositionOwn", "tFEP_Examples_ObjectState", fep::SD_Input, m_hSignalOwnIn, m_pDataSampleOwn);
            nResult |= RegisterSignalAndCreateSample("BackDistance", "tSensorInfo", fep::SD_Output, m_hSignalOut, m_pDataSampleOut);
        }

        if (fep::isOk(nResult))
        {
            // register step listener, where all cyclic work is done
            fep::StepConfig step_config(100 * 1000);

            nResult = GetTimingInterface()->RegisterStepListener("CalculateBackDistance", step_config, &cElementSensorBack::CalculateBackDistance_caller, this);
            if (fep::isFailed(nResult))
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

fep::Result cElementSensorBack::ProcessShutdownEntry(const fep::tState eOldState)
{
    if (eOldState == fep::FS_ERROR)
    {
        // Unregister listener
        GetTimingInterface()->UnregisterStepListener("CalculateBackDistance");

        // Delete samples
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
        if (m_pDataSampleOut)
        {
            delete m_pDataSampleOut;
            m_pDataSampleOut = nullptr;
        }

        // Unregister signals
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
        if (m_hSignalOut)
        {
            GetSignalRegistry()->UnregisterSignal(m_hSignalOut);
            m_hSignalOut = nullptr;
        }
    }

    return fep::ERR_NOERROR;
}

void cElementSensorBack::CalculateBackDistance(std::int64_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
{
    fep::Result nResult = fep::ERR_NOERROR;

    // copy recent data from buffer
    nResult |= pStepDataAccess->CopyRecentData(m_hSignalCarBIn, m_pDataSampleCarB);
    nResult |= pStepDataAccess->CopyRecentData(m_hSignalOwnIn,  m_pDataSampleOwn);

    tSensorInfo* pSensorInfo = reinterpret_cast<tSensorInfo*>(m_pDataSampleOut->GetPtr());

    if (fep::isOk(nResult))
    {
        // read car position data
        auto pCarB = reinterpret_cast<fep_examples::tFEP_Examples_ObjectState*>(m_pDataSampleCarB->GetPtr());
        auto pOwn = reinterpret_cast<fep_examples::tFEP_Examples_ObjectState*>(m_pDataSampleOwn->GetPtr());
        double x_pos_B;
        double x_pos_Own;
        x_pos_B = pCarB->sPosInertial.f64X;
        x_pos_Own = pOwn->sPosInertial.f64X;

        //calculate distance
        double x_dist_B = x_pos_Own - x_pos_B;

        // write distance into output sample
        pSensorInfo->x_dist = x_dist_B;
        pSensorInfo->y_dist = 0.0;
    }
    else
    {
        pSensorInfo->x_dist = -1.0; // Nothing detected
        pSensorInfo->y_dist = -1.0; // Nothing detected
    }

    pStepDataAccess->TransmitData(m_pDataSampleOut);
    m_tmLastStep = tmSimulation;
}

int main(int nArgc, const char* pArgv[])
{
    fep::cModuleOptions oModuleOptions("BackSensor");

    std::string strTimingConfiguration = "default_timing_configuration.xml";
    oModuleOptions.SetAdditionalOption(strTimingConfiguration, "-c", "--conf",
        "(Optional Argument) configuration file for timing (default: default_timing_configuration.xml)", "file");

    if (fep::isFailed(oModuleOptions.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }
    
    // create the elements
    cElementSensorBack oElement(strTimingConfiguration);
    if (fep::isFailed(oElement.Create(oModuleOptions)))
    {
        return 1;
    }

    return fep::isFailed(oElement.WaitForShutdown()) ? 1 : 0;
}
