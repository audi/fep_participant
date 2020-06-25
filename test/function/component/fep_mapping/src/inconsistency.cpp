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
* Test Case:   TestFEPMappingInconsistency
* Test ID:     1.16
* Test Title:  Test FEP Mapping Inconsistency
* Description: Tests mapping when inconsistent DDL is registered
* Strategy:    1) Register mapping configuration and inconsistent DDL description"
*              2) Initialize, try register signal, check for failure and correct incident
* Passed If:   see strategy
* Ticket:      #38151
* Requirement: FEPSDK-1724 FEPSDK-1725
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

class cMappingIncidentElement : public cTestBaseModule
{
public:
    cMappingIncidentElement()
        : m_bFirstIncident(false)
        , m_bSecondIncident(false)
    {

    }
    ~cMappingIncidentElement()
    {

    }
private:
    fep::Result HandleLocalIncident(const int16_t nIncident,
                                const tSeverityLevel eSeverity,
                                const char *strOrigin,
                                int nLine,
                                const char *strFile,
                                const timestamp_t tmSimTime,
                                const char *strDescription)
    {
        oMutex.lock();
        if (nIncident == fep::FSI_MAPPED_SIGNAL_INCONSISTENCY_FAIL)
        {
            m_bFirstIncident = true;
        }
        else if (nIncident == fep::FSI_MAPPING_CONFIG_DDL_INCONSISTENCY)
        {
            m_bSecondIncident = true;
        }
        oMutex.unlock();
        return ERR_NOERROR;
    }
public:
    bool WasSuccessful()
    {
        oMutex.lock();
        bool bSuccess = (m_bFirstIncident && m_bSecondIncident);
        oMutex.unlock();
        return bSuccess;
    }
private:
    a_util::concurrency::mutex oMutex;
    bool m_bFirstIncident;
    bool m_bSecondIncident;
};

/**
 * @req_id "FEPSDK-1724 FEPSDK-1725"
 */
TEST(cTesterFEPMapping, TestFEPMappingInconsistency)
{
    cTestMaster oMaster;
    ASSERT_EQ(a_util::result::SUCCESS, oMaster.Create(cModuleOptions( "Master")));
    ASSERT_EQ(a_util::result::SUCCESS, oMaster.StartUpModule(true));

    // Create signal mapping base modules
    cTestBaseModule oSender;
    cMappingIncidentElement oReceiver;
    ASSERT_EQ(a_util::result::SUCCESS, oSender.Create(cModuleOptions( "sender")));
    oSender.GetStateMachine()->StartupDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_IDLE, 5000));

    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.Create(cModuleOptions( "receiver")));
    oReceiver.GetStateMachine()->StartupDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, 5000));

    // Set mapping configuration file by means of FEP Property Tree
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetPropertyTree()->SetPropertyValue(
        fep::component_config::g_strMappingPath_strRemoteMapping, "./files/fep_test.map"));

    oSender.GetStateMachine()->InitializeEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_INITIALIZING, 5000));
    oReceiver.GetStateMachine()->InitializeEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_INITIALIZING, 5000));

    // Register description AFTER mapping configuration
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetSignalRegistry()->RegisterSignalDescription(
        "./files/engine_inconsistent.description", ISignalRegistry::DF_DESCRIPTION_FILE
                                                   | ISignalRegistry::DF_REPLACE));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignalDescription(
        "./files/engine_inconsistent.description", ISignalRegistry::DF_DESCRIPTION_FILE
                                                   | ISignalRegistry::DF_REPLACE));

    handle_t hMappedOutSignal;
    ASSERT_NE(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("OutSignal",
        SD_Input, "FailStruct"), hMappedOutSignal));

    // wait 5 seconds for incidents to be received
    a_util::system::sleepMilliseconds(1000 * 5);

    // now lets check
    ASSERT_TRUE(oReceiver.WasSuccessful());

    oSender.GetStateMachine()->StopEvent();
    oReceiver.GetStateMachine()->StopEvent();
    oSender.GetStateMachine()->ShutdownEvent();
    oReceiver.GetStateMachine()->ShutdownEvent();
    oMaster.GetStateMachine()->StopEvent();
    oMaster.GetStateMachine()->ShutdownEvent();
}