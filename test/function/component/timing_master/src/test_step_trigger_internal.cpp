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
#include "fep_my_mock_property_tree.h"

using namespace fep;

/*
* Test Case:   TestStepTrigger.InternalStepTrigger
* Test ID:     2.1
* Test Title:  Test Internal Step Trigger 
* Description: The internal step trigger is checked if triggering 
*              is correct.
* Strategy:    Create step trigger.
*              Let the step trigger work for some time.
*              Compare done triggers with expectations.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1752
*/

/**
 * @req_id "FEPSDK-1752 FEPSDK-1753 FEPSDK-1754"
 */
TEST(TestStepTrigger, InternalStepTrigger)
{
    using namespace fep;
    using namespace fep::timing;

    TestCallbackClass test_callback_class;

    cInternalStepTriggerStrategy oInternalStepTrigger;

    cMyMockPropertyTree oMockPropertyTree;
    oMockPropertyTree.SetSpeedFactor(1.0);
    oInternalStepTrigger.initialize(&oMockPropertyTree);

    IStepTriggerStrategy* clock = &oInternalStepTrigger;

    const timestamp_t cycle_time = 10 * 1000;

    // Register Trigger
    ASSERT_EQ(fep::ERR_NOERROR, clock->RegisterStrategyTrigger(cycle_time, &test_callback_class));
    
    // Run for some time
    clock->Start();
    a_util::system::sleepMilliseconds(1000);
    clock->Stop();

    // Unregister Trigger
    ASSERT_EQ(fep::ERR_NOERROR, clock->UnregisterStrategyTrigger(&test_callback_class));

    // Do Checks
    {
        std::list<timestamp_t>::const_iterator it = test_callback_class.received_ticks.begin();
        ASSERT_NE(it, test_callback_class.received_ticks.end());
        timestamp_t expected_ti = *it;
        for (++it; it != test_callback_class.received_ticks.end(); ++it)
        {
            expected_ti += cycle_time;
            EXPECT_EQ(*it, expected_ti);
        }
    }
}
