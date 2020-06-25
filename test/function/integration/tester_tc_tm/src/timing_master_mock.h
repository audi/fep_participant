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

#ifndef _TESTER_TC_TM_TIMING_MASTER_MOCK_H_INC_
#define _TESTER_TC_TM_TIMING_MASTER_MOCK_H_INC_

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
#include "function/_common/fep_mock_timing_client_private.h"

#define TIMING_MASTER_MOCK_USE_ASYNC_FORWARD 1

#ifdef TIMING_MASTER_MOCK_USE_ASYNC_FORWARD
#include "_common/fep_waitable_queue.h"
#endif

class TimingMasterMock : public fep::cNotificationListener, fep::IUserDataListener
{
    template <int NUM_ELEMENTS, int NUM_STEPS> friend class TimingMix;

    public:
    void Initialize(int num_samples_alloc);
    void Finalize();

public:
    void Configure();
    void Reset();

public:
    void Start();
    void Stop();

protected: // implements/overwrites INotificationListener
    fep::Result Update(IScheduleNotification const * poScheduleNotification);

    fep::Result Update(const IUserDataSample* poSample);

#ifdef TIMING_MASTER_MOCK_USE_ASYNC_FORWARD
private:
    void AsyncForwardThreadFunc();
#endif

private:
    int m_config_num_samples_alloc;
    fep::timing::TimingMaster m_oTimingMaster;
    cMyMockPropertyTreeMaster m_oMockPropertyTree;
    cMyMockDataAccessPrivate m_oMockDataAccessPrivate;
    cMyMockTransmissionAdapterPrivate m_oMockTransmissionAdapterPrivate;
    cMockUpIncidentHandler m_oIncidentHandlerMock;
    cMockTimingClientPrivate m_oMockTimingClientPrivate;
    cMockStateMachine m_oMockStateMachine;

#ifdef TIMING_MASTER_MOCK_USE_ASYNC_FORWARD
private:
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pAsyncForwardThread;
    a_util::concurrency::semaphore m_oShutdownAsyncForward;
    fep::cWaitableQueue<fep::cDataSample*> m_oAsyncForwardQueue;
    fep::cWaitableQueue<fep::cDataSample*> m_oAsyncUnusedQueue;
    std::vector<fep::cDataSample> m_AllocatedSamples;
#endif

};


#endif // _TESTER_TC_TM_TIMING_MASTER_MOCK_H_INC_