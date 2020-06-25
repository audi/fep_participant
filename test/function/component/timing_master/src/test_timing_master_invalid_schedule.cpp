/**
* Implementation of the timing master invalid schedule test
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
#include "fep3/components/legacy/timing/locked_step_legacy/timing_master.h"

#include "timing_master_fixture.h"
#include "timing_master_support.h"

#include <a_util/system.h>


using namespace fep;
using namespace fep::timing;

/*
* Test Case:   TestTimingMaster.InvalidSchedule
* Test ID:     1.x
* Test Title:  Test Timing Master Error on invalid schedule
* Description: 
* Strategy:    
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1758
*/

/**
 * @req_id "FEPSDK-1553 FEPSDK-1760 FEPSDK-1757 FEPSDK-1758"
 */
TEST_F(TestTimingMaster, InvalidSchedule)
{
    // configuration for test
    m_oMockPropertyTree.SetModuleName("TestMaster");
    m_oMockPropertyTree.SetMasterName("TestMaster");
    m_oMockPropertyTree.SetTriggerMode("AFAP");
    ASSERT_EQ(a_util::result::SUCCESS, oTM.configure());
    
    // Step Listeners
    cScheduleList* pScheduleList = new cScheduleList; // Notification takes ownership
    pScheduleList->push_back(make_Schedule(a_util::system::generateUUIDv4(), 16667));
    pScheduleList->push_back(make_Schedule(a_util::system::generateUUIDv4(), 33333));
    cScheduleNotification oScheduleNotification(pScheduleList, "A", GetModuleName(), GetTimeStampMicrosecondsUTC(), 0);
    oTM.Update(&oScheduleNotification);

    // starting and checking whether timeout after 1 second is reached i.e. state machine is set to error
    ASSERT_EQ(fep::ERR_FAILED, oTM.start());
    ASSERT_TRUE(m_oMockStateMachine.m_oErrorNotif.wait_for(a_util::chrono::milliseconds(1000)));

    // resetting the Timing Master
    ASSERT_EQ(a_util::result::SUCCESS, oTM.reset());
    ASSERT_EQ(a_util::result::SUCCESS, oTM.configure());

    // Step Listeners
    pScheduleList = new cScheduleList;
    pScheduleList->push_back(make_Schedule(a_util::system::generateUUIDv4(), 100));
    pScheduleList->push_back(make_Schedule(a_util::system::generateUUIDv4(), 1000));
    cScheduleNotification oScheduleNotification2(pScheduleList, "B", GetModuleName(), GetTimeStampMicrosecondsUTC(), 0);
    oTM.Update(&oScheduleNotification2);

    ASSERT_EQ(a_util::result::SUCCESS, oTM.start());
    ASSERT_EQ(a_util::result::SUCCESS, oTM.stop());
    ASSERT_EQ(a_util::result::SUCCESS, oTM.reset());
}