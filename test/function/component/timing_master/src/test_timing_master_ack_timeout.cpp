/**
* Implementation of the timing master ack wait timeout test
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
* Test Case:   TestTimingMaster.AckWaitTimeout
* Test ID:     1.5
* Test Title:  Test Timing Master Acknowledgement Wait Timeout
* Description: Test whether the Timing Master will push an error
*              event if acknowledgements are not received in time.
* Strategy:    Configure the Timing Master. Start it and check whether 
*              an error event was pushed into stm in time.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id ""
 */
TEST_F(TestTimingMaster, AckWaitTimeout)
{
    // configuration for test
    m_oMockPropertyTree.SetModuleName("TestMaster");
    m_oMockPropertyTree.SetMasterName("TestMaster");
    m_oMockPropertyTree.SetTriggerMode("AFAP");

    // timeout set to 1 second
    m_oMockPropertyTree.SetAckWaitTimeout(1);

    // prepration of acknowledgement sample
    fep::cDataSample oAckSample;
    oAckSample.SetSize(sizeof(TriggerAck));
    ASSERT_EQ(a_util::result::SUCCESS, oTM.configure());
    oAckSample.SetSignalHandle(oTM.GetAckInputSignalHandle());
    TriggerAck* pAck = reinterpret_cast<TriggerAck*>(oAckSample.GetPtr());
    
    // faking one Step Listener A
    cScheduleList* pScheduleList= new cScheduleList;
    std::string testUuid = a_util::system::generateUUIDv4();
    memcpy(pAck->uuid_str, testUuid.c_str(), 36);
    pScheduleList->push_back(make_Schedule(testUuid, 200));
    cScheduleNotification oScheduleNotification(pScheduleList, "A", GetModuleName(), GetTimeStampMicrosecondsUTC(), 0);
    pScheduleList = nullptr;
    oTM.Update(&oScheduleNotification);

    // starting and checking whether timeout after 1 second is reached i.e. state machine is set to error
    ASSERT_EQ(a_util::result::SUCCESS, oTM.start());
    ASSERT_FALSE(m_oMockStateMachine.m_oErrorNotif.wait_for(a_util::chrono::milliseconds(500)));
    ASSERT_TRUE(m_oMockStateMachine.m_oErrorNotif.wait_for(a_util::chrono::milliseconds(1500)));

    // resetting the Timing Master
    ASSERT_EQ(a_util::result::SUCCESS, oTM.stop());
    ASSERT_EQ(a_util::result::SUCCESS, oTM.reset());
    ASSERT_EQ(a_util::result::SUCCESS, oTM.configure());

    // faking Step Listener B
    pScheduleList = new cScheduleList;
    pScheduleList->push_back(make_Schedule(testUuid, 200));
    cScheduleNotification oScheduleNotification2(pScheduleList, "B", GetModuleName(), GetTimeStampMicrosecondsUTC(), 0);
    pScheduleList = nullptr;
    oTM.Update(&oScheduleNotification2);
    ASSERT_EQ(a_util::result::SUCCESS, oTM.start());

    // check whether no error event is triggered if acknowledgements are received in time
    for (int32_t i = 0; i < 10; ++i)
    {
        ASSERT_FALSE(m_oMockStateMachine.m_oErrorNotif.wait_for(a_util::chrono::milliseconds(800)));
        pAck->currSimTime = i * 200;
        pAck->operationalTime = 1;
        oTM.Update(&oAckSample);
    }

    // reconfiguration of timeout to 2 seconds
    ASSERT_EQ(a_util::result::SUCCESS, oTM.stop());
    m_oMockPropertyTree.SetAckWaitTimeout(2);
    ASSERT_EQ(a_util::result::SUCCESS, oTM.reset());
    ASSERT_EQ(a_util::result::SUCCESS, oTM.configure());

    // faking Step Listener C
    pScheduleList = new cScheduleList;
    pScheduleList->push_back(make_Schedule(testUuid, 200));
    cScheduleNotification oScheduleNotification3(pScheduleList, "C", GetModuleName(), GetTimeStampMicrosecondsUTC(), 0);
    pScheduleList = nullptr;
    oTM.Update(&oScheduleNotification3);

    // check whether timeout is received after 2 seconds
    ASSERT_EQ(a_util::result::SUCCESS, oTM.start());
    ASSERT_FALSE(m_oMockStateMachine.m_oErrorNotif.wait_for(a_util::chrono::milliseconds(1500)));
    ASSERT_TRUE(m_oMockStateMachine.m_oErrorNotif.wait_for(a_util::chrono::milliseconds(800)));

    ASSERT_EQ(a_util::result::SUCCESS, oTM.stop());
    ASSERT_EQ(a_util::result::SUCCESS, oTM.reset());
}