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

#ifndef _TESTER_TC_TM_TIMING_CONNECTION_H_INC_
#define _TESTER_TC_TM_TIMING_CONNECTION_H_INC_

#include <gtest/gtest.h>

#include "fep3/components/legacy/timing/locked_step_legacy/timing_master.h"
#include "fep3/components/legacy/timing/locked_step_legacy/timing_client.h"

using namespace fep;

#include "fep_my_mock_property_tree_client.h"
#include "fep_my_mock_property_tree_master.h"
#include "fep_my_mock_transmission_adapter_private.h"
#include "fep_my_mock_user_data_access.h"
#include "fep_my_mock_signal_registry.h"
#include <fep3/components/legacy/timing/common_timing.h>

#include "function/_common/fep_mock_incident_handler.h"
#include "function/_common/fep_mock_state_machine.h"

template <int NUM_ELEMENTS, int NUM_STEPS> class TimingMix
{
public:
    void Initialize()
    {
        int QUEUE_SIZE = NUM_ELEMENTS * NUM_STEPS;
        if (QUEUE_SIZE > 4096)
        {
            QUEUE_SIZE = 4096;
        }
        m_oTM.Initialize(QUEUE_SIZE);
        for (int i = 0; i < NUM_ELEMENTS; ++i)
        {
            m_voTC[i].Initialize(NUM_STEPS);
        }

        for (int i = 0; i < NUM_ELEMENTS; ++i)
        {
            TimingClientMock& x_oTC = m_voTC[i];
            ASSERT_EQ(ERR_NOERROR, m_oTM.m_oMockTransmissionAdapterPrivate.RegisterCommandListener(&x_oTC));
            ASSERT_EQ(ERR_NOERROR, x_oTC.m_oMockTransmissionAdapterPrivate.RegisterNotificationListener(&m_oTM));
        }
    }

    void Finalize()
    {
        for (int i = 0; i < NUM_ELEMENTS; ++i)
        {
            TimingClientMock& x_oTC = m_voTC[i];
            ASSERT_EQ(ERR_NOERROR, m_oTM.m_oMockTransmissionAdapterPrivate.UnregisterCommandListener(&x_oTC));
            ASSERT_EQ(ERR_NOERROR, x_oTC.m_oMockTransmissionAdapterPrivate.UnregisterNotificationListener(&m_oTM));
        }

        m_oTM.Finalize();
        for (int i = 0; i < NUM_ELEMENTS; ++i)
        {
            m_voTC[i].Finalize();
        }
    }

public:
    void Configure()
    {
        for (int i = 0; i < NUM_ELEMENTS; ++i)
        {
            m_voTC[i].Configure();
        }
        m_oTM.Configure();

        for (int i = 0; i < NUM_ELEMENTS; ++i)
        {
            TimingClientMock& x_oTC = m_voTC[i];
            ASSERT_EQ(ERR_NOERROR, x_oTC.m_oMockDataAccessPrivate.RegisterDataListener(&m_oTM, x_oTC.m_oMockTransmissionAdapterPrivate.LookupSignal(fep::timing::s_trigger_ack_signal_name)));
            ASSERT_EQ(ERR_NOERROR, m_oTM.m_oMockDataAccessPrivate.RegisterDataListener(&x_oTC, m_oTM.m_oMockTransmissionAdapterPrivate.LookupSignal(fep::timing::s_trigger_tick_signal_name)));
        }
    }

    void Reset()
    {
        for (int i = 0; i < NUM_ELEMENTS; ++i)
        {
            TimingClientMock& x_oTC = m_voTC[i];
            ASSERT_EQ(ERR_NOERROR, m_oTM.m_oMockDataAccessPrivate.UnregisterDataListener(&x_oTC, m_oTM.m_oMockTransmissionAdapterPrivate.LookupSignal(fep::timing::s_trigger_tick_signal_name)));
            ASSERT_EQ(ERR_NOERROR, x_oTC.m_oMockDataAccessPrivate.UnregisterDataListener(&m_oTM, x_oTC.m_oMockTransmissionAdapterPrivate.LookupSignal(fep::timing::s_trigger_ack_signal_name)));
        }

        m_oTM.Reset();
        for (int i = 0; i < NUM_ELEMENTS; ++i)
        {
            m_voTC[i].Reset();
        }
    }

public:
    void Start()
    {
        for (int i = 0; i < NUM_ELEMENTS; ++i)
        {
            m_voTC[i].Start();
        }
        m_oTM.Start();
    }

    void Stop()
    {
        m_oTM.Stop();
        for (int i = 0; i < NUM_ELEMENTS; ++i)
        {
            m_voTC[i].Stop();
        }
    }


public:
    TimingMasterMock m_oTM;
    TimingClientMock m_voTC[NUM_ELEMENTS];
};



#endif // _TESTER_TC_TM_TIMING_CONNECTION_H_INC_
