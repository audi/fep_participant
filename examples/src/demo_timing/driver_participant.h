/**
* Declaration of the driver for demo timing
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

#ifndef __EXAMPLE_TIMING_ELEMENT_DRIVER_H__
#define __EXAMPLE_TIMING_ELEMENT_DRIVER_H__

#include <fep_participant_sdk.h>

/**
* Element Driver
* Control vehicle based on sensor input
*/
class cExampleDriver : public fep::cModule
{
public:
    cExampleDriver(const std::string& strTimingConfiguration, bool bExtrapolate, bool bVerbose);
    ~cExampleDriver();

public: // overwrites state entry listener of cModule
    fep::Result ProcessStartupEntry(const fep::tState eOldState) override;
    fep::Result ProcessIdleEntry(const fep::tState eOldState) override;
    fep::Result ProcessInitializingEntry(const fep::tState eOldState) override;
    fep::Result ProcessShutdownEntry(const fep::tState eOldState) override;

public:
    // The step listener callback
    void CheckSensorDataAndDecide(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    static void CheckSensorDataAndDecide_caller(void* _instance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<cExampleDriver*>(_instance)->CheckSensorDataAndDecide(tmSimulation, pStepDataAccess);
    }

private:
    /// front radar signal handle
    handle_t m_hFrontHandle;
    /// back radar signal handle
    handle_t m_hBackHandle;
    /// driver ctrl signal handle
    handle_t m_hDriverCtrl;
    /// data sample for front radar data
    fep::IUserDataSample* m_pFrontSample;
    /// data sample for back radar data
    fep::IUserDataSample* m_pBackSample;
    /// data sample for driver ctrl
    fep::IUserDataSample* m_pDriverCtrlSample;

private: // simulation data members
    // front and back obstacles
    double m_f64dist_x_front;
    double m_f64dist_y_front;
    double m_f64rel_vel_front;
    double m_f64dist_x_back;
    double m_f64dist_y_back;
    double m_f64rel_vel_back;
    // last sim time
    timestamp_t m_tmlast_step;
    // the timing configuration given by commandline
    std::string m_strTimingConfiguration;
    // bool flag indicating whether extrapolation should be done
    bool m_bExtrapolate;
    // flag to enable verbose mode
    bool m_bVerbose;
};

#endif // __EXAMPLE_TIMING_ELEMENT_DRIVER_H__