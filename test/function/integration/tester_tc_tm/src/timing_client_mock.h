/*
*
* Implementation of the testfixture for the stm test
*
* @file

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
*/

#ifndef _TESTER_TC_TM_TIMING_CLIENT_MOCK_H_INC_
#define _TESTER_TC_TM_TIMING_CLIENT_MOCK_H_INC_

#include <gtest/gtest.h>

#include "fep3/components/legacy/timing/locked_step_legacy/timing_master.h"
#include "fep3/components/legacy/timing/locked_step_legacy/timing_client.h"

using namespace fep;

#include "fep_my_mock_property_tree_client.h"
#include "fep_my_mock_property_tree_master.h"
#include "fep_my_mock_transmission_adapter_private.h"
#include "fep_my_mock_user_data_access.h"
#include "fep_my_mock_signal_registry.h"

#include "function/_common/fep_mock_incident_handler.h"
#include "function/_common/fep_mock_state_machine.h"

template <int NUM_ELEMENTS, int NUM_STEPS> class TimingMix;
class TimingClientMock : public fep::cCommandListener, public fep::IUserDataListener
{
    template <int NUM_ELEMENTS, int NUM_STEPS> friend class TimingMix;

public:
    void Initialize(int num_tasks);
    void Finalize();

public:
    void Configure();
    void Reset();

public:
    void Start();
    void Stop();

protected: // implements/overwrites ICommandListener
    fep::Result Update(IGetScheduleCommand const * poCommand);
    fep::Result Update(const IUserDataSample* poSample);

private:
    void CountStep(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    static void CountStep_caller(void* _instance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<TimingClientMock*>(_instance)->CountStep(tmSimulation, pStepDataAccess);
    }
    void DummyStep(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    static void DummyStep_caller(void* _instance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<TimingClientMock*>(_instance)->DummyStep(tmSimulation, pStepDataAccess);
    }

public: // Test Support
    int64_t getStepCount() const
    {
        return m_nNumberOfSteps;
    }

    timestamp_t getRunTime() const
    {
        return m_tmStopTimeStamp - m_tmStartTimeStamp;
    }

    int64_t getSpeed() const
    {
        return getRunTime() > 0 ? (1000000 * getStepCount()) / getRunTime() : 0;
    }

private:
    void AsyncForwardThreadFunc();

private:
    int m_config_num_tasks;
    fep::timing::TimingClient m_oTimingClient;
    cMyMockPropertyTreeClient m_oMockPropertyTree;
    cMyMockDataAccessPrivate m_oMockDataAccessPrivate;
    cMockStateMachine m_oMockStateMachine;
    cMyMockSignalRegistryPrivate m_oMyMockSignalRegistryPrivate;
    cMyMockTransmissionAdapterPrivate m_oMockTransmissionAdapterPrivate;
    cMockUpIncidentHandler m_oIncidentHandlerMock;

private:
    int64_t m_nNumberOfSteps;
    timestamp_t m_tmStartTimeStamp;
    timestamp_t m_tmStopTimeStamp;

private:
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pAsyncForwardThread;
    a_util::concurrency::semaphore m_oShutdownAsyncForward;
    a_util::concurrency::condition_variable m_oSignalAsyncForwardCV;
    a_util::concurrency::mutex m_oSignalAsyncForwardLOCK;
    fep::cDataSample m_oAsyncForwardSample;
};


#endif // _TESTER_TC_TM_TIMING_CLIENT_MOCK_H_INC_