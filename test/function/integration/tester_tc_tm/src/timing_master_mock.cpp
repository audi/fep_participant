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
//#include  "components/timing_legacy/step_trigger_strategy.h"

#include "messages/fep_notification_schedule.h"
#include "transmission_adapter/fep_data_sample.h"
#include "_common/fep_schedule_list.h"
#include "_common/fep_timestamp.h"

#include "perf_timing_components.h"

#include <a_util/system.h>

#include <iostream> // For Debug purposes

using namespace fep;
using namespace fep::timing;

void TimingMasterMock::Initialize(int num_samples_alloc)
{
    m_config_num_samples_alloc = num_samples_alloc;

    ASSERT_EQ(a_util::result::SUCCESS, m_oMockTransmissionAdapterPrivate.initialize(
        &m_oMockDataAccessPrivate));
    ASSERT_EQ(a_util::result::SUCCESS, m_oTimingMaster.initialize(
        &m_oMockDataAccessPrivate,
        &m_oMockTransmissionAdapterPrivate,
        &m_oIncidentHandlerMock,
        &m_oMockPropertyTree,
        &m_oMockTimingClientPrivate,
        &m_oMockStateMachine));

    // Master configuration
    m_oMockPropertyTree.SetTriggerMode("AFAP");
    m_oMockPropertyTree.SetSpeedFactor(-1.0);
    m_oMockPropertyTree.SetModuleName("TestMaster");
    m_oMockPropertyTree.SetMasterName("TestMaster");
}

void TimingMasterMock::Finalize()
{
    ASSERT_EQ(a_util::result::SUCCESS, m_oTimingMaster.finalize());
    ASSERT_EQ(a_util::result::SUCCESS, m_oMockTransmissionAdapterPrivate.finalize());
}

void TimingMasterMock::Configure()
{
    ASSERT_EQ(ERR_NOERROR, m_oTimingMaster.configure());
}

void TimingMasterMock::Reset()
{
    ASSERT_EQ(ERR_NOERROR, m_oTimingMaster.reset());
}

void TimingMasterMock::Start()
{
#ifdef TIMING_MASTER_MOCK_USE_ASYNC_FORWARD
    m_AllocatedSamples.resize(m_config_num_samples_alloc);
    for (std::size_t i= 0; i< m_AllocatedSamples.size(); ++i)
    {
        m_AllocatedSamples[i].SetSize(sizeof(TriggerTick));
    }
    for (std::size_t i = 0; i < m_AllocatedSamples.size(); ++i)
    {
        m_oAsyncUnusedQueue.Enqueue(&(m_AllocatedSamples[i]));
    }
    m_pAsyncForwardThread.reset(new a_util::concurrency::thread(&TimingMasterMock::AsyncForwardThreadFunc, this));
#endif

    ASSERT_EQ(ERR_NOERROR, m_oTimingMaster.start());
}

void TimingMasterMock::Stop()
{
#ifdef TIMING_MASTER_MOCK_USE_ASYNC_FORWARD
    if (m_pAsyncForwardThread)
    {
        m_oShutdownAsyncForward.notify();
        m_pAsyncForwardThread->join();
        m_pAsyncForwardThread.reset();
        m_oShutdownAsyncForward.reset();
    }
#endif

    ASSERT_EQ(ERR_NOERROR, m_oTimingMaster.stop());
}

fep::Result TimingMasterMock::Update(IScheduleNotification const * poScheduleNotification)
{
    return m_oTimingMaster.Update(poScheduleNotification);
}

fep::Result TimingMasterMock::Update(const IUserDataSample* poSample)
{
#ifdef TIMING_MASTER_MOCK_USE_ASYNC_FORWARD
    fep::cDataSample* xSample;
    
    m_oAsyncUnusedQueue.Dequeue(xSample);
    xSample->SetSignalHandle(poSample->GetSignalHandle());
    xSample->CopyFrom(poSample->GetPtr(), poSample->GetSize());
    m_oAsyncForwardQueue.Enqueue(xSample);

    return ERR_NOERROR;
#else
    return m_oTimingMaster.Update(poSample);
#endif
}

#ifdef TIMING_MASTER_MOCK_USE_ASYNC_FORWARD
void TimingMasterMock::AsyncForwardThreadFunc()
{
    fep::cDataSample* xSample;

    static const timestamp_t wait_time= 100 * 1000;

    while (!m_oShutdownAsyncForward.is_set())
    {
        if (m_oAsyncForwardQueue.TryDequeue(xSample, wait_time))
        {
            m_oTimingMaster.Update(xSample);
            m_oAsyncUnusedQueue.Enqueue(xSample);
        }
    }
}
#endif



