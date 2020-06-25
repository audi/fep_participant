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
* Test Case:   TestFEPMappingByProperty
* Test ID:     1.15
* Test Title:  Test FEP Mapping by Property Setting
* Description: Tests mapping through setting by means of the FEP Property Tree
* Strategy:    1) Set mapping configuration by means of FEP Property Tree
*              2) Initialize, register descriptions and signals and check whether mapping registration was successful
*              3) Stop and try to register different mapping while old signals are registered
*              4) Initialize and catch incident
* Passed If:   see strategy
* Ticket:      #38151
* Requirement: FEPSDK-1721 FEPSDK-1722 FEPSDK-1723
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

class cSampListener : public fep::IUserDataListener
{
private:
    a_util::concurrency::semaphore* m_pSampNotif;
    a_util::concurrency::fast_mutex m_oCountMutex;
    uint32_t m_nSampCount;
    uint32_t m_nExpCount;
public:
    cSampListener(a_util::concurrency::semaphore* pSampNotif)
        : m_pSampNotif(pSampNotif), m_oCountMutex(), m_nSampCount(0), m_nExpCount(0) {}
    ~cSampListener() {}
public:
    fep::Result Update(const fep::IUserDataSample* poSample)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oCountMutex);
        m_nSampCount++;
        if (m_nSampCount == m_nExpCount)
        {
            m_pSampNotif->notify();
        }
        return ERR_NOERROR;
    }
    void SetExpectedSamples(uint32_t nExpCount) 
    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oCountMutex);
        m_nExpCount = nExpCount;
    }
    void ResetCount() 
    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oCountMutex);
        m_nSampCount = 0; 
    }
    uint32_t GetSampleCount()
    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oCountMutex);
        return m_nSampCount;
    }
};

/**
 * @req_id "FEPSDK-1721 FEPSDK-1722 FEPSDK-1723"
 */
TEST(cTesterFEPMapping, TestFEPMappingByProperty)
{
    //timeouts
    timestamp_t tmStateWait = 5 * 1000;
    timestamp_t tmTransWait = 5 * 1000;
    timestamp_t tmIncidentWait = 5 * 1000;

    // Create a test master to test for incident
    cTestMaster oIncidentMaster;
    oIncidentMaster.Create(cModuleOptions("Master"));
    ASSERT_EQ(a_util::result::SUCCESS, oIncidentMaster.SetSilentFlag(true));
    ASSERT_EQ(a_util::result::SUCCESS, oIncidentMaster.StartUpModule(true));

    // Create signal mapping base modules
    cTestBaseModule oSender, oReceiver;
    a_util::concurrency::semaphore oSampNotif;
    cSampListener oListener(&oSampNotif);
    ASSERT_EQ(a_util::result::SUCCESS, oSender.Create(cModuleOptions("sender")));
    oSender.GetStateMachine()->StartupDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_IDLE, tmStateWait));

    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.Create(cModuleOptions("receiver")));
    oReceiver.GetStateMachine()->StartupDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, tmStateWait));

    const char* strTwoMappings[2] = { "./files/fep_test.map","./files/fep_test_same_but_different.map"};
    // Set mapping configuration file by means of FEP Property Tree
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetPropertyTree()->SetPropertyValues(
        fep::component_config::g_strMappingPath_strRemoteMapping, strTwoMappings, 2));

    oSender.GetStateMachine()->InitializeEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_INITIALIZING, tmStateWait));
    oReceiver.GetStateMachine()->InitializeEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_INITIALIZING, tmStateWait));

    // Register description AFTER mapping configuration
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetSignalRegistry()->RegisterSignalDescription(
        "./files/engine.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignalDescription(
        "./files/engine.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));

    handle_t hInSignal = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("InSignal", SD_Output, "InStruct"),
        hInSignal));
    IUserDataSample* pSampleIn = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->
        CreateUserDataSample(pSampleIn, hInSignal));
    handle_t hMappedOutSignal = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("OutSignal",
        SD_Input,"OutStruct"), hMappedOutSignal));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->RegisterDataListener(&oListener, hMappedOutSignal));

    // check if the input signal was registered in the receiver
    IStringList* pInputs = NULL, *pOutputs = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->GetSignalNamesAndTypes(pInputs, pOutputs));
    ASSERT_EQ(pInputs->GetListSize(), 4);
    ASSERT_EQ(std::string(pInputs->GetStringAt(0)), "OutSignal");
    ASSERT_EQ(std::string(pInputs->GetStringAt(1)), "OutStruct");
    ASSERT_EQ(std::string(pInputs->GetStringAt(2)), "InSignal");
    ASSERT_EQ(std::string(pInputs->GetStringAt(3)), "InStruct");
    delete pInputs;
    ASSERT_EQ(pOutputs->GetListSize(), 0);
    delete pOutputs;

    const char* strDesc = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->ResolveSignalType("OutStruct", strDesc));
    ddl::CodecFactory oFactory("OutStruct", strDesc);
    ASSERT_EQ(a_util::result::SUCCESS, oFactory.isValid().getErrorCode());

    // start modules
    oSender.GetStateMachine()->InitDoneEvent();
    oSender.GetStateMachine()->StartEvent();
    oReceiver.GetStateMachine()->InitDoneEvent();
    oReceiver.GetStateMachine()->StartEvent();
    a_util::system::sleepMilliseconds(1); // to avoid spurious crashes...
    ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_RUNNING, tmStateWait));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_RUNNING, tmStateWait));

    oListener.SetExpectedSamples(3);
    oListener.ResetCount();

    // Transmit input signal
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(pSampleIn, true));
    a_util::system::sleepMicroseconds(1);
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(pSampleIn, true));
    a_util::system::sleepMicroseconds(1);
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(pSampleIn, true));
    oSampNotif.wait_for(a_util::chrono::milliseconds(tmTransWait));
    oSampNotif.reset();
    oListener.ResetCount();

    const IUserDataSample* pSampleOut = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->LockData(hMappedOutSignal, pSampleOut));

    ddl::StaticCodec oOutputCoder = oFactory.makeStaticCodecFor(pSampleOut->GetPtr(), pSampleOut->GetSize());
    ASSERT_EQ(a_util::result::SUCCESS, oOutputCoder.isValid().getErrorCode());

    // since mapping is configured to signal trigger with trigger counter on i16val we check if mapping worked by comparing it to 3
    EXPECT_EQ(ddl::access_element::get_value(oOutputCoder, "i16Val").asInt16(), 3);
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->UnlockData(pSampleOut));

    // Stop, set different mapping, initialize again and catch incident
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetStateMachine()->StopEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, tmStateWait));

    /* 
    * file with different name and slightly different content - 
    * thus property mapping mechanism works with a new configuration    
    */
    std::string content;
    a_util::filesystem::readTextFile("./files/fep_test_different.map", content);
    a_util::filesystem::writeTextFile("./fep_test_different.map", content);
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetPropertyTree()->SetPropertyValue(
        fep::common_config::g_strMappingPath_strRemoteMapping, "./fep_test_different.map"));

    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->UnregisterSignal(hMappedOutSignal));

    oReceiver.GetStateMachine()->InitializeEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_INITIALIZING, tmStateWait));

    // Register description AFTER mapping configuration
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignalDescription(
        "./files/engine.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));

    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("OutSignal",
        SD_Input, "OutStruct"), hMappedOutSignal));
    handle_t hMappedOutSignal2 = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("OutSignal2",
        SD_Input, "OutStruct"), hMappedOutSignal2));

    // check if the input signal was registered in the receiver
    pInputs = NULL, pOutputs = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->GetSignalNamesAndTypes(pInputs, pOutputs));
    ASSERT_EQ(pInputs->GetListSize(), 6);
    ASSERT_EQ(std::string(pInputs->GetStringAt(0)), "OutSignal");
    ASSERT_EQ(std::string(pInputs->GetStringAt(1)), "OutStruct");
    ASSERT_EQ(std::string(pInputs->GetStringAt(2)), "OutSignal2");
    ASSERT_EQ(std::string(pInputs->GetStringAt(3)), "OutStruct");
    ASSERT_EQ(std::string(pInputs->GetStringAt(4)), "InSignal");
    ASSERT_EQ(std::string(pInputs->GetStringAt(5)), "InStruct");
    delete pInputs;
    ASSERT_EQ(pOutputs->GetListSize(), 0);
    delete pOutputs;

    // start modules
    oReceiver.GetStateMachine()->InitDoneEvent();
    oReceiver.GetStateMachine()->StartEvent();    
    a_util::system::sleepMilliseconds(1); // to avoid spurious crashes...
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_RUNNING, tmStateWait));
    
    oListener.SetExpectedSamples(3);
    oListener.ResetCount();

    // Transmit input signal
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(pSampleIn, true));
    a_util::system::sleepMicroseconds(1);
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(pSampleIn, true));
    a_util::system::sleepMicroseconds(1);
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(pSampleIn, true));
    oSampNotif.wait_for(a_util::chrono::milliseconds(tmTransWait));
    oSampNotif.reset();
    oListener.ResetCount();

    pSampleOut = NULL;
    const IUserDataSample* pSampleOut2 = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->LockData(hMappedOutSignal, pSampleOut));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->LockData(hMappedOutSignal2, pSampleOut2));

    oOutputCoder = oFactory.makeStaticCodecFor(pSampleOut->GetPtr(), pSampleOut->GetSize());
    ASSERT_EQ(a_util::result::SUCCESS, oOutputCoder.isValid().getErrorCode());
    ddl::StaticCodec oOutputCoder2 = oFactory.makeStaticCodecFor(pSampleOut2->GetPtr(), pSampleOut2->GetSize());
    ASSERT_EQ(a_util::result::SUCCESS, oOutputCoder2.isValid().getErrorCode());

    // since mapping is configured to signal trigger with trigger counter on i64Val we check if mapping worked by comparing it to 3
    EXPECT_EQ(ddl::access_element::get_value(oOutputCoder2, "i64Val").asInt64(), 3);
    // also verify other mapping is not working anymore
    EXPECT_EQ(ddl::access_element::get_value(oOutputCoder, "i16Val").asInt16(), 0);
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->UnlockData(pSampleOut));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->UnlockData(pSampleOut2));

    // Stop, set different mapping, initialize again and catch incident
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetStateMachine()->StopEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, tmStateWait));

    /*
    * file with same name and slightly different content -
    * thus property mapping mechanism works with a new configuration
    */
    // Modify test_map.map
    a_util::strings::replace(content, "function=\"trigger_counter()\"", "constant=\"1\"");
    a_util::filesystem::writeTextFile("./fep_test_different.map", content);
     
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->UnregisterSignal(hMappedOutSignal2));

    oReceiver.GetStateMachine()->InitializeEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_INITIALIZING, tmStateWait));

    // Register description AFTER mapping configuration
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignalDescription(
        "./files/engine.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));

    hMappedOutSignal2 = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("OutSignal2",
        SD_Input, "OutStruct"), hMappedOutSignal2));

    // check if the input signal was registered in the receiver
    pInputs = NULL, pOutputs = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->GetSignalNamesAndTypes(pInputs, pOutputs));
    ASSERT_EQ(pInputs->GetListSize(), 6);
    delete pInputs;
    delete pOutputs;

    // start modules
    oReceiver.GetStateMachine()->InitDoneEvent();
    oReceiver.GetStateMachine()->StartEvent();
    a_util::system::sleepMilliseconds(1); // to avoid spurious crashes...
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_RUNNING, tmStateWait));

    pSampleOut2 = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->LockData(hMappedOutSignal2, pSampleOut2));
    oOutputCoder2 = oFactory.makeStaticCodecFor(pSampleOut2->GetPtr(), pSampleOut2->GetSize());
    ASSERT_EQ(a_util::result::SUCCESS, oOutputCoder2.isValid().getErrorCode());

    // trigger_counter() is now constant=1
    EXPECT_EQ(ddl::access_element::get_value(oOutputCoder2, "i64Val").asInt64(), 1);
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->UnlockData(pSampleOut2));

    // Stop, set different mapping, initialize again and catch incident
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetStateMachine()->StopEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, tmStateWait));

    /*
    * now try to clear while mapped signals exist
    */
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetPropertyTree()->SetPropertyValue(
        fep::common_config::g_strMappingPath_strRemoteMapping, ""));

    ASSERT_TRUE(oIncidentMaster.WaitForIncident(
                    "Clearing mapping configuration while mapped signals exist - this may cause undefined behavior!",
                    fep::FSI_MAPPING_REMOTE_PROP_CLEAR, fep::SL_Critical_Global, tmIncidentWait));

    oReceiver.GetStateMachine()->StopEvent();
    oSender.GetStateMachine()->StopEvent();

    /* 
    * at the end we also verify that it is possible to give multiple mappings as one array 
    */
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetPropertyTree()->SetPropertyValue(
        fep::component_config::g_strMappingPath_strRemoteMapping, "./files/fep_test.map; ./files/fep_test_same_but_different.map"));
    // let him start normally and see if everything works as expected
    oReceiver.GetStateMachine()->InitializeEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_INITIALIZING, tmStateWait));
    // start module
    oReceiver.GetStateMachine()->InitDoneEvent();
    oReceiver.GetStateMachine()->StartEvent();
    a_util::system::sleepMilliseconds(1); // to avoid spurious crashes...
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_RUNNING, tmStateWait));
    // Stop and check state again
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetStateMachine()->StopEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, tmStateWait));

    oReceiver.GetStateMachine()->StopEvent();
    oReceiver.GetStateMachine()->ShutdownEvent();
    oIncidentMaster.GetStateMachine()->ShutdownEvent();
    oSender.GetStateMachine()->StopEvent();
    oSender.GetStateMachine()->ShutdownEvent();
}
