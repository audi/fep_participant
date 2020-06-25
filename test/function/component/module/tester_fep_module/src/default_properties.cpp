/**
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

/*
 * Test Case:   TestModuleDefaultProperties
 * Test ID:     
 * Test Title:  Test the module default property timing configuration
 * Description: Test whether the module default property timing configuration exists after creation
 * Strategy:    Create a module and check the property
 * Passed If:   The timing configuration property exists
 * Ticket:      FEPSDK-2058
 * Requirement: FEPSDK-2058
 */

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;

/**
 * @req_id "FEPSDK-2058"
 */
TEST(TesterFepModule, TimingConfigurationPropertyExists)
{
    cTestBaseModule test_module;
    test_module.Create(cModuleOptions("test_module"));

    auto property_tree = test_module.GetComponents()->getComponent<IPropertyTree>();
    const char*  timing_config_property = "";
    ASSERT_TRUE(isOk(property_tree->GetPropertyValue(FEP_TIMING_CLIENT_CONFIGURATION_FILE, timing_config_property)));
    ASSERT_STREQ(timing_config_property, "");

    test_module.Destroy();
}