/**
 * Implementation of the tester for the integration of FEP PropertyTree with FEP Transmission Adapter
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
/**
* Test Case:   TestRemotePropertiesMemoryLeak
* Test ID:     1.3
* Test Title:  Test memory leak when requesting remote properties
* Description: Tests if the memoryleak when requesting remote properties is fixed
* 
* Strategy:    Initates 2 modules where one of them repeatedly requests a remote property of the other."
*              If functioning correctly, no memory leak should be observable in Valgrind
* Passed If:   no errors occur
*              
* Ticket:      #30812
* Requirement: FEPSDK-1606
*/
#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::component_config;

#include "messages/fep_command_set_property.h"
#include "helper_functions.h"

/**
 * @req_id "FEPSDK-1606"
 */
TEST(cTesterPropertyTreeTransmissionAdapter, TestRemotePropertiesMemoryLeak)
{
    cTestBaseModule oMaster;
    ASSERT_TRUE(fep::isOk(oMaster.Create(cModuleOptions( "Master"))));
    cTestBaseModule oSlave;
    ASSERT_TRUE(fep::isOk(oSlave.Create(cModuleOptions( "Slave"))));

    IProperty * poPtr = NULL;
    fep::Result nRemPropErr = ERR_NOERROR;

    // with the memory leak, this loop should increase the memory consumption
    for (unsigned int idx = 0; idx < 100; idx++)
    {

        nRemPropErr = oSlave.GetPropertyTree()->GetRemoteProperty(oMaster.GetName(),
            g_strTxAdapterPath_nNumberOfWorkerThreads,
            &poPtr,
            10000);

        delete poPtr;
        poPtr = NULL;

        a_util::system::sleepMilliseconds(1); // To make it easier to see the memory usage increasing...

    }
    ASSERT_TRUE(fep::isOk(nRemPropErr)) <<
                    a_util::strings::format("Error at remote property request: %d",
                                            nRemPropErr.getDescription()).c_str();
}
