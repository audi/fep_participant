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
//#include  "components/timing_legacy/step_trigger_strategy.h"

#include "messages/fep_notification_schedule.h"
#include "transmission_adapter/fep_data_sample.h"
#include "_common/fep_schedule_list.h"
#include "_common/fep_timestamp.h"

#include "perf_timing_components.h"

#include <a_util/system.h>

#include <iostream> // For Debug purposes

using namespace fep;
using namespace fep::timing;

std::ostringstream PerfTimingComponents::summary_os;

void PerfTimingComponents::initialize()
{
    static bool init_done = false;
    if (!init_done)
    {
        summary_os
            << "Elements"
            << ";" << "Steps"
            << ";" << "Total"
            << ";" << "Speed"
            << std::endl;
        init_done = true;
    }
}

PerfTimingComponents::PerfTimingComponents()
{
    initialize();
}