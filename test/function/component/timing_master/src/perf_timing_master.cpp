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

#include "messages/fep_notification_schedule.h"
#include "transmission_adapter/fep_data_sample.h"
#include "_common/fep_schedule_list.h"
#include "_common/fep_timestamp.h"

#include "timing_master_support.h"
#include "timing_master_fixture.h"

#include <a_util/system.h>

#include <iostream> 

using namespace fep;
using namespace fep::timing;

/*
* Test Case:   PerfTimingMaster.TestPerformance
* Test ID:     4.1
* Test Title:  Measure performance of timing master algorithm
* Description: To test performance (runtime and scalability)
*              the timing master is triggered as fast as possible.
* Strategy:    The Timing master is configured. Other components 
*              (timing clients with step listerners) are mocked
*              to only measure the timing master performance.
*              Test should pass. (The numbers must be checked by human.)
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

class PerformanceTimingMasterFixture
{
private:
    static void initialize()
    {
        static bool init_done = false;
        if (!init_done)
        {
            summary_os
                << "Steps"
                << ";" << "Total"
                << ";" << "Min"
                << ";" << "Max"
                << ";" << "Avg"
                << std::endl;
            init_done = true;
        }
    }
public:
    virtual void SetUp()
    {
        initialize();

        // Run as fast as possible
        m_oMockPropertyTree.SetTriggerMode("AFAP");
        m_oMockPropertyTree.SetSpeedFactor(-1.0);

        ASSERT_EQ(a_util::result::SUCCESS, m_oMockTransmissionAdapterPrivate.initialize(
            &m_oMockDataAccessPrivate));

        ASSERT_EQ(a_util::result::SUCCESS, oTM.initialize(
            &m_oMockDataAccessPrivate,
            &m_oMockTransmissionAdapterPrivate,
            &m_oIncidentHandlerMock,
            &m_oMockPropertyTree,
            &m_oMockTimingClientPrivate,
            &m_oMockStateMachine));
    }

    virtual void TearDown()
    {
        ASSERT_EQ(a_util::result::SUCCESS, oTM.finalize());
        ASSERT_EQ(a_util::result::SUCCESS, m_oMockTransmissionAdapterPrivate.finalize());
    }

    template <int NUM_STEPS> inline void DoRunTest();


protected: // Helper
    const char* GetModuleName() const
    {
        return m_oMockPropertyTree.GetModuleName();
    }

protected:
    fep::timing::TimingMaster oTM;
    cMyMockDataAccessPrivate m_oMockDataAccessPrivate;
    cMyMockTransmissionAdapterPrivate m_oMockTransmissionAdapterPrivate;
    cMockUpIncidentHandler m_oIncidentHandlerMock;
    cMyMockPropertyTree m_oMockPropertyTree;
    cMockTimingClientPrivate m_oMockTimingClientPrivate;
    cMockStateMachine m_oMockStateMachine;

public:
    static std::ostringstream summary_os;
};
std::ostringstream PerformanceTimingMasterFixture::summary_os;

template <int NUM_STEPS> inline void PerformanceTimingMasterFixture::DoRunTest()
{

    static const int number_of_iterations =
#ifdef _DEBUG
        1000
#else
        10000
#endif
        ;

    using namespace fep;
    using namespace fep::timing;

    m_oMockPropertyTree.SetModuleName("TestMaster");
    m_oMockPropertyTree.SetMasterName("TestMaster");
    m_oMockPropertyTree.SetTriggerMode("AFAP");

    ASSERT_EQ(ERR_NOERROR, oTM.configure());

    // Create a scehule map
    ScheduleMap oScheduleMap;
    {
        cScheduleList oScheduleList;
        for (int i = 0; i < NUM_STEPS; ++i)
        {
            oScheduleList.push_back(make_Schedule(a_util::system::generateUUIDv4(), 1000));
        }
    
        cScheduleNotification oScheduleNotification(&oScheduleList, "A", GetModuleName(), GetTimeStampMicrosecondsUTC(), 0);
        oTM.Update(&oScheduleNotification);

        // Initialize the Map
        std::set<ScheduleConfig> oScheduleSet;
        for (cScheduleList::const_iterator it = oScheduleList.begin(); it != oScheduleList.end(); ++it)
        {
            oScheduleSet.insert(*it);
        }

        oScheduleMap.configure(oScheduleSet);
    }


    // Some timing results
    timestamp_t start_overall_timestamp = 0;
    timestamp_t finish_overall_timestamp = 0;
    timestamp_t min_step_duration = 9999999999999;
    timestamp_t max_step_duration = 0;
    timestamp_t sum_step_duration = 0;

    // Run it
    {
        // Start timing master
        ASSERT_EQ(ERR_NOERROR, oTM.start());

        // Loop results
        timestamp_t expected_current_time = 0;
        timestamp_t expected_duration = 0;
        timestamp_t start_single_timestamp = 0;
        timestamp_t finish_single_timestamp = 0;

        // Start measurement
        start_overall_timestamp = a_util::system::getCurrentMicroseconds();

        for (int i = 0; i < number_of_iterations; ++i)
        {
            start_single_timestamp = a_util::system::getCurrentMicroseconds();

            if (oScheduleMap.isStepInCurrentSchedule())
            {
                timestamp_t last_sim_time_step;
                timestamp_t last_current_time;
                m_oMockTransmissionAdapterPrivate.GetAndResetTimes(last_sim_time_step, last_current_time);
                //std::cerr << "Received: current_time=" << last_current_time << " step=" << last_sim_time_step << std::endl;

                EXPECT_EQ(expected_duration, last_sim_time_step);
                EXPECT_EQ(expected_current_time, last_current_time);

                expected_duration = 0;
            }

            expected_duration += oScheduleMap.getCycleTime();
            expected_current_time += oScheduleMap.getCycleTime();

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
            finish_single_timestamp = a_util::system::getCurrentMicroseconds();

            {
                timestamp_t this_step_duration = finish_single_timestamp - start_single_timestamp;
                min_step_duration = std::min(min_step_duration, this_step_duration);
                max_step_duration = std::max(max_step_duration, this_step_duration);
                sum_step_duration += this_step_duration;
            }
        }

        // Stop measurement
        finish_overall_timestamp = a_util::system::getCurrentMicroseconds();
    }

    ASSERT_EQ(ERR_NOERROR, oTM.stop());
    ASSERT_EQ(ERR_NOERROR, oTM.reset());

    timestamp_t total_overall_duration = finish_overall_timestamp - start_overall_timestamp;
    std::cerr
        << " iterations: " << number_of_iterations
        << " steps: " << NUM_STEPS
        << " total: " << total_overall_duration
        << " min: " << min_step_duration
        << " max: " << max_step_duration
        << " avg: " << sum_step_duration / number_of_iterations
        << std::endl;
    summary_os
        << NUM_STEPS
        << ";" << total_overall_duration
        << ";" << min_step_duration
        << ";" << max_step_duration
        << ";" << sum_step_duration / number_of_iterations
        << std::endl; 
}

template <int NUM_STEPS> static void RunTest()
{
    PerformanceTimingMasterFixture timing_master_performance;

    timing_master_performance.SetUp();
    timing_master_performance.DoRunTest<NUM_STEPS>();
    timing_master_performance.TearDown();
}

/**
 * @req_id ""
 */
TEST(PerfTimingMaster, TestPerformance)
{
    RunTest<1>();
    RunTest<2>();
    RunTest<4>();
    RunTest<8>();
    RunTest<16>();
    RunTest<32>();
    RunTest<64>();
    RunTest<128>();
    RunTest<256>();
    RunTest<512>();
    RunTest<1024>();

    // Print summary as CSV for further investigation
    //std::cerr << PerformanceTimingMasterFixture::summary_os.str();
}
