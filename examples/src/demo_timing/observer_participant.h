/**
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

#ifndef _EXAMPLE_TIMING_ELEMENT_OBSERVER_H_
#define _EXAMPLE_TIMING_ELEMENT_OBSERVER_H_

#include <fep_participant_sdk.h>

/**
 * Element Observer
 * The Observer observes the scene and creates ascii art
 */
class cElementObserver : public fep::cModule
{
public:
    cElementObserver(const std::string& strTimingConfiguration);
    ~cElementObserver();

private:
    fep::Result RegisterSignalAndCreateSample(const char* strSignalName, const char* strSignalType,
        const fep::tSignalDirection nSignalDirection, handle_t& oSignalHandle, fep::IUserDataSample*& pDataSample);

public: // overrides cModule / cStateEntryListener
    fep::Result ProcessStartupEntry(const fep::tState eOldState) override;
    fep::Result ProcessIdleEntry(const fep::tState eOldState) override;
    fep::Result ProcessInitializingEntry(const fep::tState eOldState) override;
    fep::Result ProcessShutdownEntry(const fep::tState eOldState) override;

private:
    void ObserveScene(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    static void ObserveScene_caller(void* _this, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<cElementObserver*>(_this)->ObserveScene(tmSimulation, pStepDataAccess);
    }

private:
    /// Stores the handle for the signal
    handle_t m_hSignalCarAIn;
    /// Stores the handle for the signal
    handle_t m_hSignalCarBIn;
    /// Stores the handle for the signal
    handle_t m_hSignalOwnIn;
    /// Data Sample
    fep::IUserDataSample* m_pDataSampleCarA;
    /// Data Sample
    fep::IUserDataSample* m_pDataSampleCarB;
    /// Data Sample
    fep::IUserDataSample* m_pDataSampleOwn;
    /// Timing configuration set by command line
    std::string m_strTimingConfiguration;
};

#endif
