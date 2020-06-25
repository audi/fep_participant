/**
 * Implementation of timing master used for integration testing
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
 *
 * @file
 *
 */

#ifndef _TEST_TIMING_MASTER_H_
#define _TEST_TIMING_MASTER_H_

#include <fep_participant_sdk.h>


/// FEP Timing Client Master = Combined Client Master
class cTimingClientMasterElement : public fep::cModule
{
public:
    /// CTOR
    cTimingClientMasterElement();
    /// DTOR
    ~cTimingClientMasterElement();

public: // overrides cModule / cStateEntryListener
    fep::Result ProcessStartupEntry(const fep::tState eOldState);
    fep::Result ProcessIdleEntry(const fep::tState eOldState);
    fep::Result ProcessInitializingEntry(const fep::tState eOldState);
    fep::Result ProcessRunningEntry(const fep::tState eOldState);
public: // Test Support
    int64_t getStepCount() const
    {
        return m_nNumberOfSteps;
    }
    timestamp_t getRunTime() const
    {
        return m_tmStopTimeStamp - m_tmStartTimeStamp;
    }

private:
    void DummyStep(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    static void DummyStep_caller(void* _instance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<cTimingClientMasterElement*>(_instance)->DummyStep(tmSimulation, pStepDataAccess);
    }
private:
    void CountStep(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    static void CountStep_caller(void* _instance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<cTimingClientMasterElement*>(_instance)->CountStep(tmSimulation, pStepDataAccess);
    }


private:
    int64_t m_nNumberOfSteps;
    timestamp_t m_tmStartTimeStamp;
    timestamp_t m_tmStopTimeStamp;
};

#endif // _TEST_TIMING_MASTER_H_
