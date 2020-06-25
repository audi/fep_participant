/**
* Implementation of the driver for demo timing
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

#include "driver_participant.h"
#include "common.h"

#include <math.h>
#include <iostream>

using namespace fep;

cExampleDriver::cExampleDriver(const std::string& strTimingConfiguration, bool bExtrapolate, bool bVerbose)
    : m_hFrontHandle(nullptr)
    , m_hBackHandle(nullptr)
    , m_hDriverCtrl(nullptr)
    , m_pFrontSample(nullptr)
    , m_pBackSample(nullptr)
    , m_pDriverCtrlSample(nullptr)
    , m_f64dist_x_front(0.0)
    , m_f64dist_y_front(0.0)
    , m_f64rel_vel_front(0.0)
    , m_f64dist_x_back(0.0)
    , m_f64dist_y_back(0.0)
    , m_f64rel_vel_back(0.0)
    , m_tmlast_step(0)
    , m_strTimingConfiguration(strTimingConfiguration)
    , m_bExtrapolate(bExtrapolate)
    , m_bVerbose(bVerbose)
{
}

cExampleDriver::~cExampleDriver()
{
}

fep::Result cExampleDriver::ProcessStartupEntry(const fep::tState eOldState)
{
    fep::Result nResult = fep::ERR_NOERROR;
    double fFepVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;

    // set participant header information
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION, "Timing Demo Element Driver");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
    GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, "b33a2517-e7b4-4ae5-825c-c064ba40b362");

    // set default properties
    GetPropertyTree()->SetPropertyValue(FEP_TIMING_CLIENT_CONFIGURATION_FILE, m_strTimingConfiguration.c_str());
    GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, "TimingMaster");

    GetStateMachine()->StartupDoneEvent();

    return nResult;
}

Result cExampleDriver::ProcessIdleEntry(const tState eOldState)
{
    Result nResult = fep::ERR_NOERROR;

    if (eOldState != FS_STARTUP)
    {
        // unregister step listener
        GetTimingInterface()->UnregisterStepListener("CheckDistanceAndDecide");

        // unregister signals
        nResult |= GetSignalRegistry()->UnregisterSignal(m_hFrontHandle);
        nResult |= GetSignalRegistry()->UnregisterSignal(m_hBackHandle);
        nResult |= GetSignalRegistry()->UnregisterSignal(m_hDriverCtrl);

        // delete samples
        if (m_pFrontSample)
        {
            delete m_pFrontSample;
            m_pFrontSample = nullptr;
        }
        if (m_pBackSample)
        {
            delete m_pBackSample;
            m_pBackSample = nullptr;
        }
        if (m_pDriverCtrlSample)
        {
            delete m_pDriverCtrlSample;
            m_pDriverCtrlSample = nullptr;
        }
    }

    return nResult;
}


fep::Result cExampleDriver::ProcessInitializingEntry(const fep::tState eOldState)
{
    fep::Result nResult = fep::ERR_NOERROR;
    
    // registering signals
    fep::cUserSignalOptions oFrontSignalOptions("FrontDistance", fep::SD_Input, "tSensorInfo");
    fep::cUserSignalOptions oBackSignalOptions("BackDistance", fep::SD_Input, "tSensorInfo" );
    fep::cUserSignalOptions oCtrlSignalOptions("DriverCommand", fep::SD_Output, "tDriverCtrl");
    nResult = GetSignalRegistry()->RegisterSignal(oFrontSignalOptions, m_hFrontHandle);
    nResult |= GetSignalRegistry()->RegisterSignal(oBackSignalOptions, m_hBackHandle);
    nResult |= GetSignalRegistry()->RegisterSignal(oCtrlSignalOptions, m_hDriverCtrl);
    if (isFailed(nResult))
    {
        std::stringstream strMessage;
        strMessage << "\"RegisterSignal\" failed with error " << nResult.getErrorLabel();

        RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nResult.getErrorCode()),
            fep::SL_Critical_Global, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
    }
    else
    {
        // create samples
        nResult = GetUserDataAccess()->CreateUserDataSample(m_pFrontSample, m_hFrontHandle);
        nResult |= GetUserDataAccess()->CreateUserDataSample(m_pBackSample, m_hBackHandle);
        nResult |= GetUserDataAccess()->CreateUserDataSample(m_pDriverCtrlSample, m_hDriverCtrl);
        if (isFailed(nResult))
        {
            std::stringstream strMessage;
            strMessage << "Could not allocate data samples for signals! Result code = " << nResult.getErrorLabel();

            RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nResult.getErrorCode()),
                fep::SL_Critical_Global, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
        }
    }

    if (isOk(nResult))
    {
        // now registering the step listener (only cycle time is required - more configuration possible if needed)
        nResult = GetTimingInterface()->RegisterStepListener("CheckDistanceAndDecide", 
            StepConfig(10 * 1000), &cExampleDriver::CheckSensorDataAndDecide_caller, this);
        if (isFailed(nResult))
        {
            std::stringstream strMessage;
            strMessage << "\"RegisterStepListener\" failed! Result code = " << nResult.getErrorLabel();

            RETURN_IF_FAILED(GetIncidentHandler()->InvokeIncident(static_cast<int16_t>(nResult.getErrorCode()),
                fep::SL_Critical_Global, strMessage.str().c_str(), "ProcessInitializingEntry", __LINE__, __FILE__));
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

fep::Result cExampleDriver::ProcessShutdownEntry(const fep::tState eOldState)
{
    if (eOldState == FS_ERROR)
    {
        GetTimingInterface()->UnregisterStepListener("CheckDistanceAndDecide");

        {
            // unregistering signals
            GetSignalRegistry()->UnregisterSignal(m_hFrontHandle);
            GetSignalRegistry()->UnregisterSignal(m_hBackHandle);
            GetSignalRegistry()->UnregisterSignal(m_hDriverCtrl);
        }

        if (m_pFrontSample)
        {
            delete m_pFrontSample;
            m_pFrontSample = nullptr;
        }
        if (m_pBackSample)
        {
            delete m_pBackSample;
            m_pBackSample = nullptr;
        }
        if (m_pDriverCtrlSample)
        {
            delete m_pDriverCtrlSample;
            m_pDriverCtrlSample = nullptr;
        }
    }

    return ERR_NOERROR;
}

void cExampleDriver::CheckSensorDataAndDecide(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
{   
    double diffTime = static_cast<double>(tmSimulation - m_tmlast_step) / 1000000.0;

    // check distance and velocity of front obstacle
    fep::Result nResult = pStepDataAccess->CopyRecentData(m_hFrontHandle, m_pFrontSample);
    tSensorInfo* pSensor = reinterpret_cast<tSensorInfo*>(m_pFrontSample->GetPtr());
    if (isOk(nResult))
    {
        // decision whether extrapolation has to be carried out
        if (!m_bExtrapolate || m_pFrontSample->GetTime() > m_tmlast_step)
        {
            // filtering of invalid data (sensors are transmitting (-1) value if nothing is measured)
            if (pSensor->x_dist > 0 && m_f64dist_x_front > 0)
            {
                // calculate realtive speed of the front vehicle
                m_f64rel_vel_front = (pSensor->x_dist - m_f64dist_x_front) / diffTime;
            }
            // save current value for the next step
            m_f64dist_x_front = pSensor->x_dist;
            if (m_bVerbose)
            {
                std::cout << tmSimulation << ": " << m_f64dist_x_front << std::endl;
            }
        }
        else 
        {
            // we extrapolate
            double f64X_latest_available = pSensor->x_dist;
            // time given at CopyDataAt acts as upper simulation time bound thus we get the sample prior to the most recent
            nResult = pStepDataAccess->CopyDataBefore(m_hFrontHandle, m_pFrontSample->GetTime(), m_pFrontSample);
            if (isOk(nResult))
            {
                // filter valid data (sensor is transmitting (-1) if nothing is measured)
                if (pSensor->x_dist > 0)
                {
                    double f64X_Before = pSensor->x_dist;
                    // since we have equidistant steps this is our linear extrapolation
                    double m_f64dist_x_front_extrapolated = f64X_latest_available + (f64X_latest_available - f64X_Before);
                    m_f64rel_vel_front = (m_f64dist_x_front_extrapolated - m_f64dist_x_front) / diffTime;
                    m_f64dist_x_front = m_f64dist_x_front_extrapolated;
                    if (m_bVerbose)
                    {
                        std::cout << tmSimulation << ": " << m_f64dist_x_front << " (Extrapolated as input sample is missing)" << std::endl;
                    }
                }
            }
            else
            {
                if (m_bVerbose)
                {
                    std::cout << tmSimulation << ": " << m_f64dist_x_front << " (Can not extrapolate. Using old)" << std::endl;
                }
            }
        }
    }
    
    // check distance and velocity of back obstacle
    nResult = pStepDataAccess->CopyRecentData(m_hBackHandle, m_pBackSample);
    pSensor = reinterpret_cast<tSensorInfo*>(m_pBackSample->GetPtr());
    if (isOk(nResult))
    {
        // filtering of invalid data (sensors transmit (-1) value if nothing is measured)
        if (pSensor->x_dist > 0 && m_f64dist_x_back > 0)
        {
            // calculate relative speed of the back vehicle
            m_f64rel_vel_back = (pSensor->x_dist - m_f64dist_x_back) / diffTime;
        }
        // save current value for the next step
        m_f64dist_x_back = pSensor->x_dist;
    }

    if (isOk(nResult))
    {
        // computing acceleration / deceleration
        tDriverCtrl* pCtrl = reinterpret_cast<tDriverCtrl*>(m_pDriverCtrlSample->GetPtr());
        if (m_f64dist_x_back >= 0 && m_f64dist_x_front >= 0)
        { 
            if (m_f64dist_x_front < 30)
            {
                pCtrl->x_acc = m_f64rel_vel_front - (m_f64dist_x_front / 2.0);
            }
            else if (m_f64dist_x_back < 30)
            {
                pCtrl->x_acc = fabs(m_f64rel_vel_back) + (m_f64dist_x_back / 2.0);
            }
            else
            {
                pCtrl->x_acc = 0;
            }
        }
        else
        {
            pCtrl->x_acc = 0;
        }
    }
    else
    {
        tDriverCtrl* pCtrl = reinterpret_cast<tDriverCtrl*>(m_pDriverCtrlSample->GetPtr());
        pCtrl->x_acc = 0;
        pCtrl->y_acc = 0;
    }

    pStepDataAccess->TransmitData(m_pDriverCtrlSample);
    m_tmlast_step = tmSimulation;
}

int main(int nArgc, const char* pArgv[])
{
    fep::cModuleOptions oModuleOptions("Driver");

    std::string strTimingConfiguration = "default_timing_configuration.xml";
    oModuleOptions.SetAdditionalOption(strTimingConfiguration, "-c", "--conf",
        "(Optional Argument) configuration file for timing (default: default_timing_configuration.xml)", "file");

    bool bExtrapolate = false;
    oModuleOptions.SetAdditionalOption(bExtrapolate, "-e", "--extrapolate",
        "(Optional Argument) flag to enable extrapolation.");

    bool bVerbose = false;
    oModuleOptions.SetAdditionalOption(bVerbose, "-V", "--verbose",
        "(Optional Argument) flag to enable verbose output.");


    if (fep::isFailed(oModuleOptions.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }

    // create the elements
    cExampleDriver oElement(strTimingConfiguration, bExtrapolate, bVerbose);
    if (fep::isFailed(oElement.Create(oModuleOptions)))
    {
        return 1;
    }

    return fep::isFailed(oElement.WaitForShutdown()) ? 1 : 0;
}
