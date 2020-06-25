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

#ifndef _TEST_TIMING_CLIENT_DRIFT_H_
#define _TEST_TIMING_CLIENT_DRIFT_H_

#include <fep_participant_sdk.h>

class cDriftElement : public fep::cModule
{
public:
    cDriftElement();
    ~cDriftElement();

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

    void setStepCyclePeriode(const timestamp_t tmSimTimePeriod)
    {
        m_tmSimTimePeriod = tmSimTimePeriod;
    }

    timestamp_t getStepCyclePeriode() const
    {
        return m_tmSimTimePeriod;
    }

    void setRealCyclePeriode(const timestamp_t tmRealTimePeriod)
    {
        m_tmRealTimePeriod = tmRealTimePeriod;
    }

    timestamp_t getRealCyclePeriode() const
    {
        return m_tmRealTimePeriod;
    }

public:
    void printResults(std::ostream& os);
    timestamp_t getAverageDrift() const;
    timestamp_t getMaximumDrift() const;

private:
    void DriftStep(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    static void DriftStep_caller(void* _instance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<cDriftElement*>(_instance)->DriftStep(tmSimulation, pStepDataAccess);
    }

private:
    void CountStep(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    static void CountStep_caller(void* _instance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<cDriftElement*>(_instance)->CountStep(tmSimulation, pStepDataAccess);
    }


private:
    int64_t m_nNumberOfSteps;
    timestamp_t m_tmStartTimeStamp;
    timestamp_t m_tmStopTimeStamp;

private:
    timestamp_t m_tmSimTimeExpected;
    timestamp_t m_tmRealTimeExpected;
    timestamp_t m_tmSimTimePeriod;
    timestamp_t m_tmRealTimePeriod;
    timestamp_t m_tmSimTimeStart;
    timestamp_t m_tmRealTimeStart;

private:
    timestamp_t m_tmResMinDiffSimTime;
    timestamp_t m_tmResSumDiffSimTime;
    double      m_tmResQSumDiffSimTime;
    timestamp_t m_tmResMaxDiffSimTime;

    timestamp_t m_tmResMinDiffRealTime;
    timestamp_t m_tmResSumDiffRealTime;
    double      m_tmResQSumDiffRealTime;
    timestamp_t m_tmResMaxDiffRealTime;

    timestamp_t m_tmResMinDiffRealSimDrift;
    timestamp_t m_tmResSumDiffRealSimDrift;
    double      m_tmResQSumDiffRealSimDrift;
    timestamp_t m_tmResMaxDiffRealSimDrift;
};

#endif // _TEST_TIMING_CLIENT_DRIFT_H_