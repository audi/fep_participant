/**

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
#include <gtest/gtest.h>

#include <fep3/components/legacy/timing/common_timing.h>
#include "fep3/components/legacy/timing/locked_step_legacy/schedule_map.h"
#include "fep3/components/legacy/timing/locked_step_legacy/timing_master.h"

#include "messages/fep_notification_schedule.h"
#include "transmission_adapter/fep_data_sample.h"
#include "_common/fep_schedule_list.h"
#include "_common/fep_timestamp.h"

#include "perf_timing_components.h"

#include <a_util/system.h>
#include <a_util/strings.h>

#include <iostream> // For Debug purposes

using namespace fep;
using namespace fep::timing;

void TimingClientMock::Initialize(int num_taks)
{
    m_config_num_tasks = num_taks;

    m_nNumberOfSteps = 0;
    m_tmStartTimeStamp = 0;
    m_tmStopTimeStamp = 0;

    ASSERT_EQ(a_util::result::SUCCESS, m_oMockTransmissionAdapterPrivate.initialize(
        &m_oMockDataAccessPrivate));
    ASSERT_EQ(a_util::result::SUCCESS, m_oTimingClient.initialize(
        &m_oMockDataAccessPrivate,
        &m_oMyMockSignalRegistryPrivate,
        &m_oMockTransmissionAdapterPrivate,
        &m_oMockStateMachine,
        &m_oIncidentHandlerMock,
        &m_oMockPropertyTree));
}

void TimingClientMock::Finalize()
{
    ASSERT_EQ(a_util::result::SUCCESS, m_oTimingClient.finalize());
    ASSERT_EQ(a_util::result::SUCCESS, m_oMockTransmissionAdapterPrivate.finalize());
}

void TimingClientMock::Configure()
{

    // Client configuration
    m_oMockPropertyTree.SetModuleName("TestClient");
    m_oMockPropertyTree.SetMasterName("TestMaster");

    StepConfig oStepConfiguration(
        100 * 1000,         //  cycle_time_us
        50 * 1000,          // max_operational_real_us
        10 * 1000 * 1000,   //max_waiting_time_real_us
        TS_IGNORE_RUNTIME_VIOLATION);

    ASSERT_EQ(ERR_NOERROR, m_oTimingClient.RegisterStepListener("CountStep", oStepConfiguration, &TimingClientMock::CountStep_caller, this));
    for (int i= 1; i< m_config_num_tasks; ++i)
    {
        ASSERT_EQ(ERR_NOERROR, m_oTimingClient.RegisterStepListener(a_util::strings::format("DummyStep%d", i).c_str() , oStepConfiguration, &TimingClientMock::DummyStep_caller, this));
    }

    ASSERT_EQ(ERR_NOERROR, m_oTimingClient.configure());
}

void TimingClientMock::Reset()
{
    ASSERT_EQ(ERR_NOERROR, m_oTimingClient.UnregisterStepListener("CountStep"));
    for (int i = 1; i< m_config_num_tasks; ++i)
    {
        ASSERT_EQ(ERR_NOERROR, m_oTimingClient.UnregisterStepListener(a_util::strings::format("DummyStep%d", i).c_str()));
    }

    ASSERT_EQ(ERR_NOERROR, m_oTimingClient.reset());
}

void TimingClientMock::Start()
{
    m_oAsyncForwardSample.SetSize(sizeof(TriggerTick));
    m_pAsyncForwardThread.reset(new a_util::concurrency::thread(&TimingClientMock::AsyncForwardThreadFunc, this));

    ASSERT_EQ(ERR_NOERROR, m_oTimingClient.start());
}

void TimingClientMock::Stop()
{
    if (m_pAsyncForwardThread)
    {
        m_oShutdownAsyncForward.notify();
        m_pAsyncForwardThread->join();
        m_pAsyncForwardThread.reset();
        m_oShutdownAsyncForward.reset();
    }

    ASSERT_EQ(ERR_NOERROR, m_oTimingClient.stop());
}

void TimingClientMock::CountStep(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
{
    m_tmStopTimeStamp = a_util::system::getCurrentMicroseconds();
    if (m_tmStartTimeStamp == 0)
    {
        m_tmStartTimeStamp = m_tmStopTimeStamp;
        // Should add at least the cycle time, but this is not known
    }

    ++m_nNumberOfSteps;
}


void TimingClientMock::DummyStep(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
{
    // Do nothing
}

fep::Result TimingClientMock::Update(IGetScheduleCommand const * poCommand)
{
    return static_cast<cCommandListener*>(&m_oTimingClient)->Update(poCommand);
}

fep::Result TimingClientMock::Update(const IUserDataSample* poSample)
{
    a_util::concurrency::unique_lock<a_util::concurrency::mutex> locker(m_oSignalAsyncForwardLOCK);
    m_oAsyncForwardSample.SetSignalHandle(poSample->GetSignalHandle());
    m_oAsyncForwardSample.CopyFrom(poSample->GetPtr(), poSample->GetSize());
    m_oSignalAsyncForwardCV.notify_one();

    return ERR_NOERROR;
}

void TimingClientMock::AsyncForwardThreadFunc()
{
    static const a_util::chrono::microseconds wait_time(100 * 1000);

    while (!m_oShutdownAsyncForward.is_set())
    {
        a_util::concurrency::unique_lock<a_util::concurrency::mutex> locker(m_oSignalAsyncForwardLOCK);
        if (m_oSignalAsyncForwardCV.wait_for(locker, wait_time) == a_util::concurrency::cv_status::no_timeout)
        {
            static_cast<IUserDataListener*>(&m_oTimingClient)->Update(&m_oAsyncForwardSample);
        }
    }
}
