/**
* Implementation of the tester for the FEP Automation Interface
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
* Test Case:   TestShutdownHandler
* Test ID:     1.22
* Test Title:  Test AI shutdown handler
* Description: Tests usage of shutdown handler in AI
* Strategy:    Register a shutdown handler and check calls to the callback function
*              
* Passed If:   Shutdown handler was called
* Ticket:      -
* Requirement:
*/
#include <gtest/gtest.h>

#include <fep_test_common.h>
#include "helper_functions.h"


/**
 * @req_id ""
 */
TEST(cTesterFepAutomation, TestShutdownHandler)
{
    cModule oAiModule;
    oAiModule.Create("AI test module");
    oAiModule.GetStateMachine()->StartupDoneEvent();

    AutomationInterface oAi(oAiModule);

    bool bShutdownHandlerCalled = false;

    // register shutdown handler
    oAi.SetShutdownHandler([&bShutdownHandlerCalled]() { bShutdownHandlerCalled = true; });

    ASSERT_FALSE(bShutdownHandlerCalled);

    oAiModule.GetStateMachine()->ShutdownEvent();
    oAiModule.WaitForShutdown(REM_PROP_TIMEOUT * 4);

    ASSERT_TRUE(bShutdownHandlerCalled);
}

/**
 * @req_id ""
 */
TEST(cTesterFepAutomation, TestShutdownHandlerImplicitModule)
{
    AutomationInterface oAiControl; // AI to control the AI under test

    const char * strAiName = "AI with implicit Module";

    AutomationInterface oAi(strAiName);

    bool bShutdownHandlerCalled = false;

    // register shutdown handler
    oAi.SetShutdownHandler([&bShutdownHandlerCalled]() { bShutdownHandlerCalled = true; });

    ASSERT_FALSE(bShutdownHandlerCalled);

    oAiControl.SetPropertyValue(fep::component_config::g_strStateMachineStandAloneModePath_bEnable, false, strAiName, 1000);
    oAiControl.TriggerEventSync(fep::CE_Shutdown, strAiName, 1000);


    ASSERT_TRUE(bShutdownHandlerCalled);
}

/**
 * @req_id ""
 */
TEST(cTesterFepAutomation, TestEmptyShutdownHandler)
{
    // check if empty shutdown handlers lead to crashes
    {
        cModule oAiModule;
        oAiModule.Create("AI test module");
        oAiModule.GetStateMachine()->StartupDoneEvent();

        AutomationInterface oAi(oAiModule);
        // no handler set

        oAiModule.GetStateMachine()->ShutdownEvent();
        ASSERT_EQ(ERR_NOERROR, oAiModule.WaitForShutdown(REM_PROP_TIMEOUT * 4));
    }
    {
        cModule oAiModule;
        oAiModule.Create("AI test module");
        oAiModule.GetStateMachine()->StartupDoneEvent();

        AutomationInterface oAi(oAiModule);
        // set empty handler function
        oAi.SetShutdownHandler(std::function<void()>(nullptr));
        
        oAiModule.GetStateMachine()->ShutdownEvent();
        ASSERT_EQ(ERR_NOERROR, oAiModule.WaitForShutdown(REM_PROP_TIMEOUT * 4));
    }
}