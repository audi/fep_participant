/**
 * Implementation of the tester for the integration of FEP Module with the FEP Property Tree.
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
* Test Case:   TestShutdownWhileMirroring
* Test ID:     1.1
* Test Title:  Test shutdown of element while properties are being mirrored
* Description: Tests the correctly working shut down of FEP elements which properties are being mirrored
* Strategy:    Initate 2 FEP elements. Fill the property tree of the first with 100 properties. Let the second
*              mirror those properties and verify the shutdown of the first element.
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1558
*/

#include <cstring>

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

#include <ddl.h>

/**
 * @req_id "FEPSDK-1558"
 */
TEST(cTesterModulePropertyTree, TestShutdownWhileMirroring)
{
    cTestBaseModule oModule;
    cTestBaseModule oMirrorModule;

    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions("TestModule")));
    ASSERT_EQ(a_util::result::SUCCESS, oMirrorModule.Create(cModuleOptions("TestMirrorModule")));

    oModule.GetStateMachine()->StartupDoneEvent();
    oMirrorModule.GetStateMachine()->StartupDoneEvent();
    oModule.WaitForState(fep::FS_IDLE);
    oMirrorModule.WaitForState(fep::FS_IDLE);

    for (uint32_t i = 0; i < 100; ++i)
    {
        std::string strProp = a_util::strings::format("%d", i);
        ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->SetPropertyValue(strProp.c_str(), static_cast<double>(100.00)));
        ASSERT_EQ(a_util::result::SUCCESS, oMirrorModule.GetPropertyTree()->MirrorRemoteProperty(oModule.GetName(), strProp.c_str(), strProp.c_str(), 10000));
    }

    a_util::system::sleepMilliseconds(10);

    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->ShutdownEvent());
    oModule.WaitForState(fep::FS_SHUTDOWN);

    ASSERT_EQ(a_util::result::SUCCESS, oModule.Destroy());
}