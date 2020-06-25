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

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include "fep3/components/legacy/timing/global_scheduler_configuration.h"
#include "fep3/components/legacy/timing/common_timing.h"

using namespace fep::timing;

/*
* Test Case:   TimingConfig.ReadConfig
* Test ID:     2.0
* Test Title:  Test Reading and Writing Timing Configurations.
* Description: Test reading and writing of timing configurations
*              using a sample configuration.
* Strategy:    Read sample configuration. 
*              Write configuration.
*              Read sample configuration from newly written configuration.
*              Compare results
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1769
*/

/**
 * @req_id "FEPSDK-1769"
 */
TEST(TimingConfig, ReadConfig)
{
    TimingConfiguration timing_configuration;
    ASSERT_EQ(fep::ERR_NOERROR, TimingConfig::readTimingConfigFromFile("files/timing_configuration.xml", timing_configuration));

    std::string timing_config_str;
    ASSERT_EQ(fep::ERR_NOERROR, TimingConfig::writeTimingConfigToString(timing_config_str, timing_configuration));

    TimingConfiguration timing_configuration_again;
    ASSERT_EQ(fep::ERR_NOERROR, TimingConfig::readTimingConfigFromString(timing_config_str, timing_configuration_again));
 
    std::string timing_config_str_again;
    ASSERT_EQ(fep::ERR_NOERROR, TimingConfig::writeTimingConfigToString(timing_config_str_again, timing_configuration_again));
    
    EXPECT_EQ(timing_config_str, timing_config_str_again);
}
