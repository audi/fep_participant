/**
* Implementation of the a test for checking the maximum size of the schedule map
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

#include "timing_master_fixture.h"
#include "timing_master_support.h"

#include "_common/fep_schedule_list.h"

#include "timing_master_schedule_support.h"

#include <a_util/system.h>

using namespace fep;
using namespace fep::timing;

/*
* Test Case:   TimingMasterScheduleConfig
* Test Title:  Test of Timing Master with unlevelled frequencies when computing the schedule map.
* Description: The timing master calculates the lcm and gcd values from the participants' cycle time.
* If the calculated value is getting too large, the timing master sets the size of the schedule map to zero
* and goes to the error state. Otherwise the size of the schedule map could increase to several GB or more.
* Strategy:    Test if the schedule map fails if the timing frequencies are unlevelled.
* Passed If:   No errors occur
* Ticket:      FEPSDK-882
* Requirement: -
*/

/**
 * @req_id "FEPSDK-1553 FEPSDK-1760 FEPSDK-1757 FEPSDK-1758"
 */
TEST_F(TestTimingMaster, TimingMasterScheduleMap)
{
    {
        // Fake some participants for scheduling
        fep::cScheduleList oScheduleList;
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(), 16667)); // cycle_time 60 Hz to be equal to 16667 microseconds
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(), 33333)); // cycle_time 30 Hz to be equal to 33333 microseconds

        std::set<ScheduleConfig> oScheduleSet;
        for (cScheduleList::const_iterator it = oScheduleList.begin(); it != oScheduleList.end(); ++it)
        {
            oScheduleSet.insert(*it);
        }

        // Check if the schedule map will NOT be created with unlevelled timing frequencies
        ScheduleMap oScheduleMap;
        ASSERT_EQ(false, oScheduleMap.configure(oScheduleSet));
    }

    {
        fep::cScheduleList oScheduleList;
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(), 10000));
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(), 100000));

        ScheduleMap oScheduleMap;

        // Initialize the schedule configuration
        std::set<ScheduleConfig> oScheduleSet;
        for (cScheduleList::const_iterator it = oScheduleList.begin(); it != oScheduleList.end(); ++it)
        {
            oScheduleSet.insert(*it);
        }

        // Check if the schedule map will be created with fine timing frequencies
        ASSERT_EQ(true, oScheduleMap.configure(oScheduleSet));
    }
}