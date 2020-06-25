/**
* Implementation of the stm change notifier test
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

#include <gtest/gtest.h>

#include <fep3/components/legacy/timing/common_timing.h>
#include "fep3/components/legacy/timing/locked_step_legacy/schedule_map.h"
#include "fep3/components/legacy/timing/locked_step_legacy/timing_master.h"

#include "messages/fep_notification_schedule.h"
#include "transmission_adapter/fep_data_sample.h"
#include "_common/fep_schedule_list.h"
#include "_common/fep_timestamp.h"

#include "timing_master_fixture.h"
#include "timing_master_support.h"

#include <a_util/system.h>


using namespace fep;
using namespace fep::timing;

#pragma pack(push,1)
/// External clock trigger
/// Also described in timing.description
struct tFEP_StepTrigger
{
    /// current time (wall clock time)
    timestamp_t currentTime_us;
    /// validity of the current time
    timestamp_t validity_us;
};
#pragma pack(pop)


/*
* Test Case:   TestTimingMaster.ExternalStepTrigger
* Test ID:     1.4
* Test Title:  Test Timing Master with Test External Step Trigger 
* Description: The External Step Trigger uses a external clock to
*              get trigger. This external clock is connected using
*              FEP signals.
* Strategy:    Test if the mocked timing client is receiving the
*              expected triggering.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id "FEPSDK-1553 FEPSDK-1757 FEPSDK-1752 FEPSDK-1751 FEPSDK-1755 FEPSDK-1762"
 */
TEST_F(TestTimingMaster, ExternalStepTrigger)
{
    using namespace fep;
    using namespace fep::timing;

    const timestamp_t timeout = 100 * 1000;

    // Speciyl settings for this test
    m_oMockTransmissionAdapterPrivate._received_triggers.clear();
    m_oMockTransmissionAdapterPrivate.m_bLoggingMode = true;

    m_oMockPropertyTree.SetModuleName("TestMaster");
    m_oMockPropertyTree.SetMasterName("TestMaster");
    m_oMockPropertyTree.SetTriggerMode("EXTERNAL_CLOCK");

    ASSERT_EQ(ERR_NOERROR, oTM.configure());

    // Fake some results
    cScheduleList oScheduleList;
    {
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(), 20 * 1000));
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(), 30 * 1000));
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(), 50 * 1000));
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(), 100 * 1000));

        cScheduleNotification oScheduleNotification(&oScheduleList, "A", GetModuleName(), GetTimeStampMicrosecondsUTC(), 0);

        oTM.Update(&oScheduleNotification);
    }

    {
        // Create a schedule map for Testing
        ScheduleMap oScheduleMap;
        {
            // Initialize the Map
            std::set<ScheduleConfig> oScheduleSet;
            for (cScheduleList::const_iterator it = oScheduleList.begin(); it != oScheduleList.end(); ++it)
            {
                oScheduleSet.insert(*it);
            }

            oScheduleMap.configure(oScheduleSet);
        }

        ASSERT_EQ(ERR_NOERROR, oTM.start());
 
        for (int i = 0; i < 3; ++i)
        {

            // Simulate external trigger
            {
                tFEP_StepTrigger clock_tick;
                cDataSample oDataSample;
                oDataSample.Attach(&clock_tick, sizeof(tFEP_StepTrigger));
                oDataSample.SetSignalHandle(cMyMockTransmissionAdapterPrivate::s_tClockSignalHandle);

                clock_tick.currentTime_us = (2010 + i) * 1000 * 1000; // 2010 s
                clock_tick.validity_us = 1 * 1000 * 1000; // 1 s

                 m_oMockTransmissionAdapterPrivate.TransmitData(&oDataSample);
            }

            // Let it run !
            a_util::system::sleepMilliseconds(1 * 1000);
        }

        ASSERT_EQ(ERR_NOERROR, oTM.stop());

        timestamp_t expected_timestamp = -1;
        timestamp_t expected_duration = 0;
        
        // Verify results
        for (std::list<fep::timing::TriggerTick>::iterator it = m_oMockTransmissionAdapterPrivate._received_triggers.begin(); it != m_oMockTransmissionAdapterPrivate._received_triggers.end(); )
        {
            // Do we expect some trigger value
            if (oScheduleMap.isStepInCurrentSchedule())
            {
                timestamp_t last_sim_time_step = it->simTimeStep;
                timestamp_t last_current_time = it->currentTime;
                if (expected_timestamp < 0)
                {
                    expected_timestamp = last_current_time;
                }

                EXPECT_EQ(expected_duration, last_sim_time_step);
                EXPECT_EQ(last_current_time, expected_timestamp);

                expected_duration = 0;

                ++it;
            }

            expected_duration += oScheduleMap.getCycleTime();
            expected_timestamp += oScheduleMap.getCycleTime();

            // Increase current_index
            oScheduleMap.incrementCurrentSchedule();
        }
    }

    ASSERT_EQ(ERR_NOERROR, oTM.reset());
}
