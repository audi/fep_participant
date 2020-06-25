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
* Test Case:   TestMuteUnmuteModuleRemote
* Test ID:     1.12
* Test Title:  Mute / Unmute Modules
* Description: Mute/unmute modules via the Automation Interface
* Strategy:    Some signals are registered by a module. Now we try to mute the module and 
*              check if this module really stopped sending while another module is still 
*              sending. Now we unmute again and check if the module started to send 
*              again. This is done locally and remotely. Also error codes are provoked.
*              
* Passed If:   see strategy
* Ticket:      -
* Requirement: FEPSDK-1497 FEPSDK-1498 FEPSDK-1499
*/
#include <gtest/gtest.h>
#include <fep_test_common.h>
#include "transmission_adapter/fep_data_sample_factory.h"
#include "helper_functions.h"

/**
 * @req_id "FEPSDK-1497 FEPSDK-1498 FEPSDK-1499 FEPSDK-1749"
 */
TEST(cTesterFepAutomation, TestMuteUnmuteModuleRemote)
{
    AutomationInterface oAI;
    timestamp_t tmTransWait = 3 * 1000;
    timestamp_t tmMuteWait = 10 * 1000;

    fep::Result nResult = ERR_NOERROR;
    cTestBaseModule oMod2Mute;
    cReceiverModuleUI16 oMod2Rx;
    a_util::concurrency::semaphore oSampNotif;
    cSampListener oSampListener(&oSampNotif);
    bool bStatus = false;

    handle_t hSignal_Mute1 = NULL;
    handle_t hSignal_Mute2 = NULL;
    handle_t hSignal_Mute1_In = NULL;
    handle_t hSignal_Mute2_In = NULL;

    std::string strName_Mute1 = "TestMuteUnmuteModule_Mute1";
    std::string strName_Mute1_type = "tTestMuteUnmuteModule_Mute1";
    std::string strName_Mute2 = "TestMuteUnmuteModule_Mute2";
    std::string strName_Mute2_type = "tTestMuteUnmuteModule_Mute2";

    /* create the modules */
    ASSERT_EQ(a_util::result::SUCCESS, 
        oMod2Mute.Create(cModuleOptions("TestMuteUnmuteModule_Mod2Mute")));
    ASSERT_EQ(a_util::result::SUCCESS, 
        oMod2Rx.Create(cModuleOptions("TestMuteUnmuteModule_Mod2Rx")));

    IUserDataAccess * pBus2Mute = oMod2Mute.GetUserDataAccess();

    /* register all signals */
    nResult = oMod2Mute.GetSignalRegistry()->
        RegisterSignalDescription(RETURN_MEDIA_DESC(strName_Mute1_type.c_str(), "ui16Value", "tUInt16"),
            ISignalRegistry::DF_MERGE);
    nResult = oMod2Mute.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strName_Mute1.c_str(),
        SD_Output, strName_Mute1_type.c_str()), hSignal_Mute1);

    nResult |= oMod2Mute.GetSignalRegistry()->
        RegisterSignalDescription(RETURN_MEDIA_DESC(strName_Mute2_type.c_str(), "ui16Value", "tUInt16"),
            ISignalRegistry::DF_MERGE);
    nResult |= oMod2Mute.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strName_Mute2.c_str(),
        SD_Output, strName_Mute2_type.c_str()), hSignal_Mute2);

    nResult |= oMod2Rx.GetSignalRegistry()->
        RegisterSignalDescription(RETURN_MEDIA_DESC(strName_Mute1_type.c_str(), "ui16Value", "tUInt16"),
            ISignalRegistry::DF_MERGE);
    nResult |= oMod2Rx.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strName_Mute1.c_str(),
        SD_Input, strName_Mute1_type.c_str()), hSignal_Mute1_In);

    nResult |= oMod2Rx.GetSignalRegistry()->
        RegisterSignalDescription(RETURN_MEDIA_DESC(strName_Mute2_type.c_str(), "ui16Value", "tUInt16"),
            ISignalRegistry::DF_MERGE);
    nResult |= oMod2Rx.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strName_Mute2.c_str(),
        SD_Input, strName_Mute2_type.c_str()), hSignal_Mute2_In);

    ASSERT_EQ(a_util::result::SUCCESS, nResult) <<  "Error during registration of signals.";

    oMod2Rx.GetUserDataAccess()->
        RegisterDataListener(&oSampListener, hSignal_Mute1_In);
    oMod2Rx.GetUserDataAccess()->
        RegisterDataListener(&oSampListener, hSignal_Mute2_In);

    /* bring modules up */
    ASSERT_EQ(a_util::result::SUCCESS, oMod2Mute.StartUpModule(true));
    ASSERT_EQ(a_util::result::SUCCESS, oMod2Rx.StartUpModule(true));
    a_util::system::sleepMilliseconds(10); // wait till all state notifications are done

    /* now prepare sample and data pointer to send samples */
    fep::IUserDataSample * poUserSample = NULL;
    uint16_t * pUi16Data = NULL; /* data pointer for sample value */
    ASSERT_EQ(a_util::result::SUCCESS, fep::cDataSampleFactory::CreateSample(&poUserSample)) <<
        "Creating a new sample by using the \"cDataSampleFactory\"";
    ASSERT_EQ(a_util::result::SUCCESS, poUserSample->SetSize(sizeof(uint16_t))) << 
        " - configure sample: set memory size";
    pUi16Data = (uint16_t *)poUserSample->GetPtr();
    *pUi16Data = 42; /* any value we can identify */

    oSampListener.SetExpectedSamples(2);

    /* now send some data and see if all 2 samples are received */
    ASSERT_EQ(a_util::result::SUCCESS, poUserSample->SetSignalHandle(hSignal_Mute1));
    ASSERT_EQ(a_util::result::SUCCESS, pBus2Mute->TransmitData(poUserSample, true));
    ASSERT_EQ(a_util::result::SUCCESS, poUserSample->SetSignalHandle(hSignal_Mute2));
    ASSERT_EQ(a_util::result::SUCCESS, pBus2Mute->TransmitData(poUserSample, true));

    oSampNotif.wait_for(a_util::chrono::milliseconds(tmTransWait));

    // check reception
    ASSERT_EQ(2 , oSampListener.GetSampleCount());
    oSampNotif.reset();
    oSampListener.ResetCount();

    /* remote check */
    ASSERT_EQ(a_util::result::SUCCESS, oAI.IsParticipantMuted(bStatus, 
        oMod2Mute.GetName(), tmMuteWait));
    ASSERT_FALSE(bStatus);

    /* now mute module remotely, send data again and check if no signals
    * are received */
    ASSERT_EQ(a_util::result::SUCCESS, oAI.MuteParticipant(oMod2Mute.GetName(), tmMuteWait));
    a_util::system::sleepMilliseconds(1000);

    /* now send some data and see if no samples are received */
    ASSERT_EQ(a_util::result::SUCCESS, poUserSample->SetSignalHandle(hSignal_Mute1));
    ASSERT_EQ(a_util::result::SUCCESS, pBus2Mute->TransmitData(poUserSample, true));
    ASSERT_EQ(a_util::result::SUCCESS, poUserSample->SetSignalHandle(hSignal_Mute2));
    ASSERT_EQ(a_util::result::SUCCESS, pBus2Mute->TransmitData(poUserSample, true));

    oSampNotif.wait_for(a_util::chrono::milliseconds(tmTransWait));

    // check reception
    ASSERT_EQ(0 , oSampListener.GetSampleCount());
    oSampNotif.reset();
    oSampListener.ResetCount();

    /* remote check */
    ASSERT_EQ(a_util::result::SUCCESS, oAI.IsParticipantMuted(bStatus, 
        oMod2Mute.GetName(), tmMuteWait));
    ASSERT_TRUE(bStatus);

    /* now unmute module remotely, send data again and check if 5 signals are
    * received */
    ASSERT_EQ(a_util::result::SUCCESS, oAI.UnmuteParticipant(oMod2Mute.GetName(), tmMuteWait));
    a_util::system::sleepMilliseconds(1000);

    ASSERT_EQ(a_util::result::SUCCESS, poUserSample->SetSignalHandle(hSignal_Mute1));
    ASSERT_EQ(a_util::result::SUCCESS, pBus2Mute->TransmitData(poUserSample, true));
    ASSERT_EQ(a_util::result::SUCCESS, poUserSample->SetSignalHandle(hSignal_Mute2));
    ASSERT_EQ(a_util::result::SUCCESS, pBus2Mute->TransmitData(poUserSample, true));

    oSampNotif.wait_for(a_util::chrono::milliseconds(tmTransWait));

    // check reception
    ASSERT_EQ(2 , oSampListener.GetSampleCount());
    oSampNotif.reset();
    oSampListener.ResetCount();

    /* remote check */
    ASSERT_EQ(a_util::result::SUCCESS, oAI.IsParticipantMuted(bStatus, 
        oMod2Mute.GetName(), tmMuteWait));
    ASSERT_FALSE(bStatus);

    /* now we try to get all error codes returned by IsParticipantMuted */
    // module name contains wildcards (remote only)
    ASSERT_EQ(ERR_INVALID_ARG, oAI.IsParticipantMuted(bStatus, "AB*", REM_PROP_TIMEOUT));
    ASSERT_EQ(ERR_INVALID_ARG, oAI.IsParticipantMuted(bStatus, "A?C", REM_PROP_TIMEOUT));
    // negative timeout (remote only)
    ASSERT_EQ(ERR_INVALID_ARG, oAI.IsParticipantMuted(bStatus, oMod2Mute.GetName(), -2));
    // timeout due to non existing module (remote only)
    ASSERT_EQ(ERR_TIMEOUT, oAI.IsParticipantMuted(bStatus, "AnotherFooModule", REM_PROP_TIMEOUT));

}
