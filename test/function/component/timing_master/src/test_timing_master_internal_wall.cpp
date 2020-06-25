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

#include "aev_test/gtest.h"

#include <components/timing_legacy/common_timing.h>
#include "components/timing_legacy/locked_step_legacy/schedule_map.h"
#include "components/timing_legacy/locked_step_legacy/timing_master.h"

#include "messages/fep_notification_schedule.h"
#include "transmission_adapter/fep_data_sample.h"
#include "_common/fep_schedule_list.h"
#include "_common/fep_timestamp.h"

#include "timing_master_fixture.h"
#include "timing_master_support.h"

#include <a_util/system.h>


using namespace fep;
using namespace fep::timing;

/*
* Test Case:   TestTimingMaster.InternalStepTrigger_WallClockMode
* Test ID:     1.2
* Test Title:  Test Timing Master with Test Internal Step Trigger using wall clock progress
* Description: The timing master is using the internal step trigger to
*              trigger the timing client relative to wall clock progress.
* Strategy:    Test if the mocked timing client is receiving the
*              expected triggering.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1553 FEPSDK-1757 FEPSDK-1752 FEPSDK-1751 FEPSDK-1754 FEPSDK-1759
*/

/**
 * @req_id "FEPSDK-1553 FEPSDK-1757 FEPSDK-1752 FEPSDK-1751 FEPSDK-1754 FEPSDK-1759 FEPSDK-1556 FEPSDK-1555"
 */
TEST_F(TestTimingMaster, InternalStepTrigger_WallClockMode)
{
    using namespace fep;
    using namespace fep::timing;

    const timestamp_t timeout = 100 * 1000;

    m_oMockPropertyTree.SetModuleName("TestMaster");
    m_oMockPropertyTree.SetMasterName("TestMaster");
    m_oMockPropertyTree.SetTriggerMode("SYSTEM_TIME");
    m_oMockPropertyTree.SetSpeedFactor(1.0);

    ASSERT_EQ(ERR_NOERROR, oTM.configure());

    // Fake some results
    cScheduleList oScheduleList;
    {
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(),  20 * 1000));
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(),  30 * 1000));
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(),  50 * 1000));
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

        timestamp_t expected_current_time = -1;
        timestamp_t expected_duration = 0;

        for (int i = 0; i < 100; ++i)
        {
            if (oScheduleMap.isStepInCurrentSchedule())
            {
                timestamp_t last_sim_time_step;
                timestamp_t last_current_time;
                m_oMockTransmissionAdapterPrivate.GetAndResetTimes(last_sim_time_step, last_current_time);
                if (expected_current_time < 0)
                {
                    expected_current_time = last_current_time;
                }

                EXPECT_EQ(expected_duration, last_sim_time_step);
                EXPECT_EQ(last_current_time, expected_current_time);

                expected_duration = 0;
            }

            expected_duration += oScheduleMap.getCycleTime();
            expected_current_time += oScheduleMap.getCycleTime();

            // Send dummy responses from client
            const ScheduleItem& current_schedule_item = oScheduleMap.refCurrentScheduleItem();
            for (ScheduleItem::ScheduleReceivedMap::const_iterator it = current_schedule_item.refScheduleReceivedMap().begin();
                it != current_schedule_item.refScheduleReceivedMap().end(); ++it)
            {
                cDataSample oDataSample;
                fillAckDataSample(oDataSample, oTM, it->first);

                oTM.Update(&oDataSample);
            }

            // Increase current_index
            oScheduleMap.incrementCurrentSchedule();
        }

        ASSERT_EQ(ERR_NOERROR, oTM.stop());
    }

    ASSERT_EQ(ERR_NOERROR, oTM.reset());
}
