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
* Test Case:   TestMuteUnmuteSignalsRemote
* Test ID:     1.10
* Test Title:  Mute / Unmute Signals
* Description: Mute/unmute signals via the Automation Interface
* Strategy:    Some (= three) signals are registered by a module. Now we try to mute two 
*              signals and see if theses signals are really muted and other signals are 
*              still send. Now we unmute again and see if all signals are send again. 
*              This is done remotely. Also local error codes are provoked.
*              
* Passed If:   see strategy
* Ticket:      -
* Requirement: FEPSDK-1491 FEPSDK-1492 FEPSDK-1493 FEPSDK-1494 FEPSDK-1495 FEPSDK-1496 FEPSDK-1613 FEPSDK-1746
*/
#include <gtest/gtest.h>
#include <fep_test_common.h>
#include "transmission_adapter/fep_data_sample_factory.h"
#include "helper_functions.h"

/**
 * @req_id "FEPSDK-1491 FEPSDK-1492 FEPSDK-1493 FEPSDK-1494 FEPSDK-1495 FEPSDK-1496 FEPSDK-1613 FEPSDK-1746"
 */
TEST(cTesterFepAutomation, TestMuteUnmuteSignalsRemote)
{
    AutomationInterface oAI;
    fep::Result nResult = ERR_NOERROR;
    cTestBaseModule oMod2Mute, oMod2Rx;
    a_util::concurrency::semaphore oSampNotif;
    cSampListener oSampListener(&oSampNotif);
    bool bStatus = false;

    handle_t hSignal_Mute = NULL;
    handle_t hSignal_NotMute = NULL;
    handle_t hSignal_Mute_In = NULL;
    handle_t hSignal_NotMute_In = NULL;

    std::string strName_Mute1 = "TestMuteUnmuteSignals_Mute1";
    std::string strName_Mute1_type = "tTestMuteUnmuteSignals_Mute1";
    std::string strName_NoMute = "TestMuteUnmuteSignals_NoMute3";
    std::string strName_NoMute_type = "tTestMuteUnmuteSignals_NoMute3";

    /* create the modules */
    ASSERT_EQ(a_util::result::SUCCESS, 
        oMod2Mute.Create(cModuleOptions("TestMuteUnmuteSignals_Mod2Mute")));
    ASSERT_EQ(a_util::result::SUCCESS, 
        oMod2Rx.Create(cModuleOptions("TestMuteUnmuteSignals_Mod2Rx")));

    IUserDataAccess * pBus2Mute = oMod2Mute.GetUserDataAccess();

    /* register all signals */
    nResult = oMod2Mute.GetSignalRegistry()->
        RegisterSignalDescription(RETURN_MEDIA_DESC(strName_Mute1_type.c_str(), "ui16Value", "tUInt16"),
            ISignalRegistry::DF_MERGE);
    nResult = oMod2Mute.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strName_Mute1.c_str(),
        SD_Output, strName_Mute1_type.c_str()), hSignal_Mute);
    nResult |= oMod2Mute.GetSignalRegistry()->
        RegisterSignalDescription(RETURN_MEDIA_DESC(strName_NoMute_type.c_str(), "ui16Value", "tUInt16"),
            ISignalRegistry::DF_MERGE);
    nResult |= oMod2Mute.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strName_NoMute.c_str(),
        SD_Output, strName_NoMute_type.c_str()), hSignal_NotMute);
    nResult |= oMod2Rx.GetSignalRegistry()->
        RegisterSignalDescription(RETURN_MEDIA_DESC(strName_Mute1_type.c_str(), "ui16Value", "tUInt16"),
            ISignalRegistry::DF_MERGE);
    nResult |= oMod2Rx.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strName_Mute1.c_str(),
        SD_Input, strName_Mute1_type.c_str()), hSignal_Mute_In);
    nResult |= oMod2Rx.GetSignalRegistry()->
        RegisterSignalDescription(RETURN_MEDIA_DESC(strName_NoMute_type.c_str(), "ui16Value", "tUInt16"),
            ISignalRegistry::DF_MERGE);
    nResult |= oMod2Rx.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strName_NoMute.c_str(),
        SD_Input, strName_NoMute_type.c_str()), hSignal_NotMute_In);

    ASSERT_EQ(a_util::result::SUCCESS, nResult) <<  "Error during registration of signals.";

    oMod2Rx.GetUserDataAccess()->
        RegisterDataListener(&oSampListener, hSignal_Mute_In);
    oMod2Rx.GetUserDataAccess()->
        RegisterDataListener(&oSampListener, hSignal_NotMute_In);

    // bring up modules
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

    // send data and check for reception
    ASSERT_EQ(a_util::result::SUCCESS, poUserSample->SetSignalHandle(hSignal_Mute));
    ASSERT_EQ(a_util::result::SUCCESS, pBus2Mute->TransmitData(poUserSample, true));
    ASSERT_EQ(a_util::result::SUCCESS, poUserSample->SetSignalHandle(hSignal_NotMute));
    ASSERT_EQ(a_util::result::SUCCESS, pBus2Mute->TransmitData(poUserSample, true));

    oSampNotif.wait_for(a_util::chrono::milliseconds(REM_PROP_TIMEOUT));

    // check reception
    ASSERT_EQ(oSampListener.GetSampleCount() , 2);
    oSampListener.ResetCount();
    oSampNotif.reset();

    // now mute a signal remotely
    ASSERT_EQ(a_util::result::SUCCESS, oAI.MuteSignal(
        strName_Mute1, oMod2Mute.GetName(), REM_PROP_TIMEOUT));
    oSampNotif.wait_for(a_util::chrono::milliseconds(REM_PROP_TIMEOUT));
    a_util::system::sleepMilliseconds(REM_PROP_TIMEOUT);

    /* remote check */
    ASSERT_EQ(a_util::result::SUCCESS, oAI.IsSignalMuted(
        strName_Mute1, bStatus, oMod2Mute.GetName(), REM_PROP_TIMEOUT));
    ASSERT_TRUE(bStatus);

    ASSERT_EQ(a_util::result::SUCCESS, poUserSample->SetSignalHandle(hSignal_Mute));
    ASSERT_EQ(a_util::result::SUCCESS, pBus2Mute->TransmitData(poUserSample, true));
    ASSERT_EQ(a_util::result::SUCCESS, poUserSample->SetSignalHandle(hSignal_NotMute));
    ASSERT_EQ(a_util::result::SUCCESS, pBus2Mute->TransmitData(poUserSample, true));

    // just a wait since this should not be triggered
    oSampNotif.wait_for(a_util::chrono::milliseconds(REM_PROP_TIMEOUT));
    
    // check reception
    ASSERT_EQ(oSampListener.GetSampleCount() , 1);
    oSampListener.ResetCount();
    oSampNotif.reset();

    // now remote unmute signal
    ASSERT_EQ(a_util::result::SUCCESS, oAI.UnmuteSignal(
        strName_Mute1, oMod2Mute.GetName(), REM_PROP_TIMEOUT));

    // check remotely if unmuted
    ASSERT_EQ(a_util::result::SUCCESS, oAI.IsSignalMuted(
        strName_Mute1, bStatus, oMod2Mute.GetName(), REM_PROP_TIMEOUT));
    ASSERT_FALSE(bStatus);

    ASSERT_EQ(a_util::result::SUCCESS, poUserSample->SetSignalHandle(hSignal_Mute));
    ASSERT_EQ(a_util::result::SUCCESS, pBus2Mute->TransmitData(poUserSample, true));
    ASSERT_EQ(a_util::result::SUCCESS, poUserSample->SetSignalHandle(hSignal_NotMute));
    ASSERT_EQ(a_util::result::SUCCESS, pBus2Mute->TransmitData(poUserSample, false));

    oSampNotif.wait_for(a_util::chrono::milliseconds(REM_PROP_TIMEOUT));

    // check reception
    ASSERT_EQ(oSampListener.GetSampleCount() , 2);

    /* now try to get all error codes for remote requests */
    ASSERT_TRUE(ERR_INVALID_ARG ==
        oAI.MuteSignal(strName_Mute1, "*", REM_PROP_TIMEOUT));
    ASSERT_TRUE(ERR_INVALID_ARG ==
        oAI.MuteSignal(strName_Mute1, "ABC*", REM_PROP_TIMEOUT));
    ASSERT_TRUE(ERR_INVALID_ARG ==
        oAI.MuteSignal(strName_Mute1, "?", REM_PROP_TIMEOUT));
    ASSERT_TRUE(ERR_INVALID_ARG ==
        oAI.MuteSignal(strName_Mute1, "ABC?", REM_PROP_TIMEOUT));
    ASSERT_TRUE(ERR_INVALID_ARG ==
        oAI.MuteSignal(strName_Mute1, "", REM_PROP_TIMEOUT));
    // invalid timeout
    ASSERT_TRUE(ERR_INVALID_ARG ==
        oAI.MuteSignal(strName_Mute1, oMod2Mute.GetName(), -1));
    // Multiple Mute
    ASSERT_EQ(a_util::result::SUCCESS, oAI.MuteSignal(
        strName_Mute1, oMod2Mute.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.MuteSignal(
        strName_Mute1, oMod2Mute.GetName(), REM_PROP_TIMEOUT));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.UnmuteSignal(
        strName_Mute1, oMod2Mute.GetName(), REM_PROP_TIMEOUT));
    // INPUT Signal
    ASSERT_EQ(ERR_NOT_FOUND, oAI.MuteSignal(strName_Mute1, oMod2Rx.GetName(), REM_PROP_TIMEOUT));
    // Non-existing module
    ASSERT_EQ(ERR_TIMEOUT, oAI.MuteSignal(strName_Mute1, "FooModulesDoNotExist", REM_PROP_TIMEOUT));

    /* and now we check the error codes for IsSignalMuted */
    ASSERT_EQ(ERR_TIMEOUT, oAI.IsSignalMuted("", bStatus, oMod2Mute.GetName(), REM_PROP_TIMEOUT));
    // timeout negative (remote only)
    ASSERT_EQ(ERR_INVALID_ARG, oAI.IsSignalMuted(strName_Mute1, bStatus, oMod2Mute.GetName(), -1));
    // Module name contains wild cards (remote only)
    ASSERT_EQ(ERR_INVALID_ARG, oAI.IsSignalMuted(strName_Mute1, bStatus, "AB*", REM_PROP_TIMEOUT));
    ASSERT_EQ(ERR_INVALID_ARG, oAI.IsSignalMuted(strName_Mute1, bStatus, "A?C", REM_PROP_TIMEOUT));
    // timeout due to non existing module (remote only)
    ASSERT_EQ(ERR_TIMEOUT, oAI.IsSignalMuted(strName_Mute1, bStatus, "Otto", REM_PROP_TIMEOUT));
}
