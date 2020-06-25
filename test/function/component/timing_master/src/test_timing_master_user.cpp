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
#include "components/timing_legacy/locked_step_legacy//timing_master.h"

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
* Test Case:   TestTimingMaster.UserStepTrigger
* Test ID:     1.3
* Test Title:  Test Timing Master with Test Internal Step Trigger using a user supplied trigger
* Description: A manually triggered step trigger is used as
*              user supplied step trigger.
* Strategy:    Test if the mocked timing client is receiving the
*              expected triggering.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

class ManualStepTrigger : public IStepTrigger
{
public:
    ManualStepTrigger()
        : _step_trigger_listener(nullptr)
    {
    }

public:
    fep::Result RegisterTrigger(const timestamp_t cycle_time, IStepTriggerListener* pStepTriggerListener)
    {
        // Valid arguments ?
        if ((cycle_time <= 0) || (nullptr == pStepTriggerListener))
        {
            return ERR_INVALID_ARG;
        }

        // Already registered ?
        if (_step_trigger_listener)
        {
            return ERR_UNEXPECTED;
        }

        _step_trigger_listener = pStepTriggerListener;

        return ERR_NOERROR;
    }

    fep::Result UnregisterTrigger(IStepTriggerListener* pStepTriggerListener)
    {
        // Valid arguments ?
        if (nullptr == pStepTriggerListener)
        {
            return ERR_INVALID_ARG;
        }

        // Registered ?
        if (_step_trigger_listener != pStepTriggerListener)
        {
            return ERR_UNEXPECTED;
        }

        _step_trigger_listener = nullptr;

        return ERR_NOERROR;
    }


protected:
    void doCall()
    {
        if (_step_trigger_listener)
        {
            _step_trigger_listener->Trigger();
        }
    }

public: // Support
    void setStartTime(const timestamp_t ti)
    {
        _step_trigger_listener->SetInitialSimulationTime(ti);
    }

    void manualTrigger()
    {
        doCall();
    }

private:
    IStepTriggerListener* _step_trigger_listener;
};

/**
 * @req_id "FEPSDK-1553 FEPSDK-1757 FEPSDK-1752 FEPSDK-1751 FEPSDK-1756 FEPSDK-1763"
 */
TEST_F(TestTimingMaster, UserStepTrigger)
{
    using namespace fep;
    using namespace fep::timing;

    const timestamp_t timeout = 100 * 1000;

    m_oMockPropertyTree.SetModuleName("TestMaster");
    m_oMockPropertyTree.SetMasterName("TestMaster");
    m_oMockPropertyTree.SetTriggerMode("USER_IMPLEMENTATION");

    ManualStepTrigger* pManualStepTrigger = new ManualStepTrigger();
    ASSERT_EQ(ERR_NOERROR, oTM.SetStepTrigger(pManualStepTrigger));


    ASSERT_EQ(ERR_NOERROR, oTM.configure());

    // Fake some results
    cScheduleList oScheduleList;
    {
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(), 200));
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(), 300));
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(), 500));
        oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(), 1000));

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
 
        timestamp_t expected_timestamp = 2010;
        timestamp_t expected_duration = 0;
        pManualStepTrigger->setStartTime(expected_timestamp);

        for (int i = 0; i < 100; ++i)
        {
            // Trigger 
            pManualStepTrigger->manualTrigger();

            // Do we expect some trigger value
            if (oScheduleMap.isStepInCurrentSchedule())
            {
                timestamp_t last_sim_time_step;
                timestamp_t last_current_time;
                m_oMockTransmissionAdapterPrivate.GetAndResetTimes(last_sim_time_step, last_current_time);
 
                EXPECT_EQ(expected_duration, last_sim_time_step);
                EXPECT_EQ(last_current_time, expected_timestamp);

                expected_duration = 0;
            }

            expected_duration += oScheduleMap.getCycleTime();
            expected_timestamp += oScheduleMap.getCycleTime();

            // Increase current_index
            oScheduleMap.incrementCurrentSchedule();
        }
    }

    ASSERT_EQ(ERR_NOERROR, oTM.stop());
    ASSERT_EQ(ERR_NOERROR, oTM.reset());
}


