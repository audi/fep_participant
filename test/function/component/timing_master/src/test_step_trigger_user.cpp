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


/*
* Test Case:   TestStepTrigger.UserStepTrigger
* Test ID:     2.2
* Test Title:  Test User Step Trigger
* Description: The user step trigger is checked if triggering
*              is correct. Sample implementation of user step 
*              trigger is used.
* Strategy:    Create step trigger.
*              Let the step trigger work for some time.
*              Compare done triggers with expectations.
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

class cMyStepTrigger : public IStepTrigger
{
public:
    fep::Result RegisterTrigger(const timestamp_t cycle_time, IStepTriggerListener* step_trigger_listener)
    {
        _step_trigger_listener = step_trigger_listener;

        return ERR_NOERROR;
    }

    fep::Result UnregisterTrigger(IStepTriggerListener* step_trigger_listener)
    {
        _step_trigger_listener = nullptr;

        return ERR_NOERROR;
    }

public:
    void SetInitialSimulationTime(const timestamp_t simulation_time)
    {
        _step_trigger_listener->SetInitialSimulationTime(simulation_time);
    }

    void DoStep()
    {
        _step_trigger_listener->Trigger();
    }

private:
    /// listener registered
    IStepTriggerListener* _step_trigger_listener;
};

/**
 * @req_id "FEPSDK-1756 FEPSDK-1763"
 */
TEST(TestStepTrigger, UserStepTrigger)
{
    using namespace fep;
    using namespace fep::timing;

    TestCallbackClass test_callback_class;

    cUserStepTriggerStrategy oUserStepTrigger;
    cMyStepTrigger oMyStepTrigger;
    oUserStepTrigger.initialize(&oMyStepTrigger);
    IStepTriggerStrategy* clock = &oUserStepTrigger;

    const timestamp_t cycle_time = 10 * 1000;

    // Register Trigger
    ASSERT_EQ(fep::ERR_NOERROR, clock->RegisterStrategyTrigger(cycle_time, &test_callback_class));

    // Run for some time
    clock->Start();

    for (int i = 0; i < 3; ++i)
    {
        timestamp_t expected_ti = (2010 + i) * 1000 * 1000; // 2010 s
        oMyStepTrigger.SetInitialSimulationTime(expected_ti);
        test_callback_class.received_ticks.clear();

        // Simulate trigger
        for (int j = 0; j < 100; ++j)
        {
            oMyStepTrigger.DoStep();
        }

        // Do Checks
        {
            std::list<timestamp_t>::const_iterator it = test_callback_class.received_ticks.begin();
            ASSERT_NE(it, test_callback_class.received_ticks.end());

            // First tick must be exact
            for (; it != test_callback_class.received_ticks.end(); ++it)
            {
                EXPECT_EQ(*it, expected_ti);

                expected_ti += cycle_time;
            }
        }
    }

    clock->Stop();

    // Unregister Trigger
    ASSERT_EQ(fep::ERR_NOERROR, clock->UnregisterStrategyTrigger(&test_callback_class));
}
