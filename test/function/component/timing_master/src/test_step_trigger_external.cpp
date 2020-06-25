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

#include "fep_participant_sdk.h"

#include "fep3/components/legacy/timing/locked_step_legacy/timing_master.h"
#include "fep3/components/legacy/timing/locked_step_legacy/step_trigger_strategy.h"
#include "transmission_adapter/fep_data_sample.h"

#include "timing_master_support.h"

using namespace fep;

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
* Test Case:   TestStepTrigger.ExternalStepTrigger
* Test ID:     2.3
* Test Title:  Test External Step Trigger
* Description: The external step trigger is checked if triggering
*              is correct.
* Strategy:    Create step trigger.
*              Let the step trigger work for some time.
*              Compare done triggers with expectations.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK_654
*/

/**
 * @req_id "FEPSDK-1755"
 */
TEST(TestStepTrigger, ExternalStepTrigger)
{
    using namespace fep;
    using namespace fep::timing;

    TestCallbackClass test_callback_class;

    cExternalStepTriggerStrategy oExternalStepTrigger;
    IStepTriggerStrategy* clock = &oExternalStepTrigger;

    const timestamp_t cycle_time = 10 * 1000;

    // Register Trigger
    ASSERT_EQ(fep::ERR_NOERROR, clock->RegisterStrategyTrigger(cycle_time, &test_callback_class));

    // Run for some time
    clock->Start();

    tFEP_StepTrigger step_trigger;

    for (int i = 0; i < 3; ++i) 
    {
        test_callback_class.reset();

        // Simulate external trigger: Initial trigger
        {
            cDataSample oDataSample;
            oDataSample.Attach(&step_trigger, sizeof(tFEP_StepTrigger));

            step_trigger.currentTime_us = (2010 + i) * 1000 * 1000; // 2010 s
            step_trigger.validity_us = 1 * 1000 * 1000; // 1 s

            oExternalStepTrigger.Update(&oDataSample);
        }

        auto begintime = a_util::system::getCurrentMilliseconds();

        while (test_callback_class.received_ticks.size() < 100)
        {
            a_util::system::sleepMilliseconds(50);
#ifndef __QNX__
            ASSERT_LE(a_util::system::getCurrentMilliseconds() - begintime,  5000);
#else      // TODO: getting 5044 > 5000; but fails on Linux as well...
            EXPECT_LE(a_util::system::getCurrentMilliseconds() - begintime,  5000);
#endif
        }
        

        // Do Checks
        {
            std::list<timestamp_t>::const_iterator it = test_callback_class.received_ticks.begin();
            ASSERT_NE(it, test_callback_class.received_ticks.end());
            timestamp_t expected_ti = *it;

            // First tick must be exact
            std::cerr << "Received: First_time=" << *it << "(" << step_trigger.currentTime_us << ")" << std::endl;
            EXPECT_EQ(*it, step_trigger.currentTime_us);

            int count = 0;
            for (++it; it != test_callback_class.received_ticks.end(); ++it)
            {
                count++;
                expected_ti += cycle_time;
                std::cerr << "Received (index-" << count << "): current_time=" << *it << "(" << expected_ti << ")" << std::endl;
                
                // Check if matches
                EXPECT_EQ(*it, expected_ti);

                // All ticks must be inside margins
                EXPECT_LE(*it, step_trigger.currentTime_us + step_trigger.validity_us);
                EXPECT_GE(*it, step_trigger.currentTime_us);
            }

            EXPECT_GE(test_callback_class.received_ticks.size(), 100);
        }
    }

    clock->Stop();

    // Unregister Trigger
    ASSERT_EQ(fep::ERR_NOERROR, clock->UnregisterStrategyTrigger(&test_callback_class));
}

