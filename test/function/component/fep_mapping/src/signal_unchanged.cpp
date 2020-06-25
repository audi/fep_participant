/**
 * Implementation of the tester for Mapping
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
/*
* Test Case:   TestSignalUnchangedFEPMapping
* Test ID:     1.12
* Test Title:  Test FEP Mapping unchanged signals
* Description: Tests mapping through the offical FEP SDK API
*               This has no effect on currently registered signals, 
*               only during signal registration is the mapping 
*               configuration considered
* Strategy:    
* Passed If:   no errors occur
* Ticket:      #32177
* Requirement: FEPSDK-1611
*/

#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include "fep_test_common.h"

#include "a_util/concurrency.h"
#include "mapping/fep_mapping.h"
#include "signal_registry/fep_signal_registry.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include <ddl.h>

using namespace mapping::oo;
using namespace mapping::rt;
using namespace ddl;

/**
 * @req_id "FEPSDK-1611"
 */
TEST(cTesterFEPMapping, TestSignalUnchangedFEPMapping)
{
   // Create signal mapping base module
    cTestBaseModule oReceiver;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.Create(cModuleOptions( "receiver")));
    oReceiver.GetStateMachine()->StartupDoneEvent(); ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, 5000));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignalDescription(
        "./files/engine.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));

    // Registered signal without mapping stay unchanged after mapping registration
    handle_t hNotMappedOutSignal = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("OutSignal",
        SD_Input, "OutStruct"), hNotMappedOutSignal));
    ASSERT_TRUE(hNotMappedOutSignal);
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalMapping()->RegisterMappingConfiguration(
        "./files/fep_test.map", ISignalMapping::MF_MAPPING_FILE | ISignalMapping::MF_REPLACE));

    fep::cSignalRegistry* pReceiverSignalRegistry = dynamic_cast<fep::cSignalRegistry*>(oReceiver.GetSignalRegistry());
    ASSERT_TRUE(pReceiverSignalRegistry);
    ASSERT_FALSE(pReceiverSignalRegistry->IsMappedSignal(hNotMappedOutSignal));
    const char * strSignalName = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->GetSignalNameFromHandle(hNotMappedOutSignal,strSignalName));
    ASSERT_TRUE(a_util::strings::isEqual(strSignalName, "OutSignal"));
    ASSERT_NE(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("OutSignal",
        SD_Input, "OutStruct"), hNotMappedOutSignal));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->UnregisterSignal(hNotMappedOutSignal));
}