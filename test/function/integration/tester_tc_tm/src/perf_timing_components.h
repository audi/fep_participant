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

#ifndef _TESTER_TC_TM_PERF_TIMING_COMPONENTS_H_INC_
#define _TESTER_TC_TM_PERF_TIMING_COMPONENTS_H_INC_

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

#include "timing_client_mock.h"
#include "timing_master_mock.h"
#include "timing_connection.h"


class PerfTimingComponents : public ::testing::Test 
{
private:
    static void initialize();

public:
    PerfTimingComponents();

protected:
    template <int NUM_ELEMENTS, int NUM_STEPS> inline void RunTest()
    {
        TimingMix<NUM_ELEMENTS, NUM_STEPS> m_oMx;

        m_oMx.Initialize();
        m_oMx.Configure();
        m_oMx.Start();

        a_util::system::sleepMilliseconds(2 * 1000);

        m_oMx.Stop();
        m_oMx.Reset();
        m_oMx.Finalize();

        TimingClientMock& x_oTC = m_oMx.m_voTC[0];
        std::cerr 
            << " Steps: " << x_oTC.getStepCount() << ""
            << " Runtime: " << x_oTC.getRunTime() << " us"
            << " Speed: " << x_oTC.getSpeed() << " steps/s"
            << std::endl;

        summary_os
            << NUM_ELEMENTS
            << ";" << NUM_STEPS
            << ";" << x_oTC.getStepCount()
            << ";" << x_oTC.getSpeed()
            << std::endl;
    }

protected:
    static std::ostringstream summary_os;
};



#endif // _TESTER_TC_TM_PERF_TIMING_COMPONENTS_H_INC_