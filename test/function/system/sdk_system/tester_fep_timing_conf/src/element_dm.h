/**
* Implementation of timing client / server element used for integration testing
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

#ifndef _TEST_TIMING_CLIENT_DUMMY_H_
#define _TEST_TIMING_CLIENT_DUMMY_H_

#include <fep_participant_sdk.h>

class cDummyElement : public fep::cModule
{
public:
    cDummyElement();
    ~cDummyElement();

public: // overwrites state entry listener of cModule
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
        reinterpret_cast<cDummyElement*>(_instance)->DummyStep(tmSimulation, pStepDataAccess);
    }
private:
    void CountStep(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    static void CountStep_caller(void* _instance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<cDummyElement*>(_instance)->CountStep(tmSimulation, pStepDataAccess);
    }


private:
    int64_t m_nNumberOfSteps;
    timestamp_t m_tmStartTimeStamp;
    timestamp_t m_tmStopTimeStamp;
};

#endif // _TEST_TIMING_CLIENT_DUMMY_H_