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
* Test Case:   TestFEPMapping
* Test ID:     1.13
* Test Title:  Test FEP Mapping
* Description: Tests mapping through the offical FEP SDK API
* Strategy:    If the new input signal is matched to a target signal, 
*              the mapping is instantiated and the source signal, 
*              specified in the mapping configuration, is also registered
* Passed If:   no errors occur
* Ticket:      #32177
* Requirement: FEPSDK-1612
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include "fep_test_common.h"

#include "a_util/concurrency.h"
#include "mapping/fep_mapping.h"
#include "signal_registry/fep_signal_registry.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include <ddl.h>
#include "test_ddl_types.h"

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
 * @req_id "FEPSDK-1397 FEPSDK-1612"
 */
TEST(cTesterFEPMapping, TestFEPMapping)
{
    {
        // Create signal mapping base modules
        cTestBaseModule oSender, oReceiver;
        a_util::concurrency::semaphore oSampNotif;
        cSampListener oListener(&oSampNotif);
        ASSERT_EQ(a_util::result::SUCCESS, oSender.Create(cModuleOptions( "sender")));
        oSender.GetStateMachine()->StartupDoneEvent(); ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_IDLE, 5000));
        ASSERT_EQ(a_util::result::SUCCESS, oSender.GetSignalRegistry()->RegisterSignalDescription(
            "./files/engine.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));

        ASSERT_EQ(a_util::result::SUCCESS, oReceiver.Create(cModuleOptions( "receiver")));
        oReceiver.GetStateMachine()->StartupDoneEvent(); ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, 5000));
        ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignalDescription(
            "./files/engine.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));
        ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalMapping()->RegisterMappingConfiguration(
            "./files/fep_test.map", ISignalMapping::MF_MAPPING_FILE | ISignalMapping::MF_REPLACE));

        oSender.GetStateMachine()->InitializeEvent(); ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_INITIALIZING, 5000));
        oReceiver.GetStateMachine()->InitializeEvent(); ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_INITIALIZING, 5000));

        handle_t hInSignal = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oSender.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("InSignal",
            SD_Output, "InStruct"), hInSignal));
        handle_t hMappedOutSignal = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("OutSignal",SD_Input, "OutStruct"),
            hMappedOutSignal));
        ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->RegisterDataListener(&oListener, hMappedOutSignal));
        {
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
        }

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
        ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_RUNNING, 5000));
        ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_RUNNING, 5000));

        // check defaults
        const IUserDataSample* pSampleOut = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->LockData(hMappedOutSignal, pSampleOut));

        ddl::StaticCodec oOutputCoder = oFactory.makeStaticCodecFor(pSampleOut->GetPtr(), pSampleOut->GetSize());
        ASSERT_EQ(a_util::result::SUCCESS, oOutputCoder.isValid().getErrorCode());
        ASSERT_EQ(ddl::access_element::get_value(oOutputCoder, "i16Val").asInt16(), 0);

        // we explicitly DONT unlock the sample here, to test whether it really stays locked

        oListener.SetExpectedSamples(1);

        // send input sample
        IUserDataSample* pSampleIn = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->
            CreateUserDataSample(pSampleIn, hInSignal));
        ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(pSampleIn, true));

        oSampNotif.wait_for(a_util::chrono::milliseconds(500));

        // now we check whether we still get the old data and the triggered sample got lost in the internal buffer
        const IUserDataSample* pSampleOld = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->LockData(hMappedOutSignal, pSampleOld));
        ASSERT_EQ(a_util::memory::compare(pSampleOut->GetPtr(), pSampleOut->GetSize(), pSampleOld->GetPtr(), pSampleOld->GetSize()), 0);
        ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->UnlockData(pSampleOld));

        // but the listener should have caught the missed sample
        ASSERT_EQ(oListener.GetSampleCount(), 1);

        oListener.SetExpectedSamples(2);
        oSampNotif.reset();
        // now we unlock everything and send an input sample again
        ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->UnlockData(pSampleOut));
        ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(pSampleIn, true));
        oSampNotif.wait_for(a_util::chrono::milliseconds(500));

        ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->LockData(hMappedOutSignal, pSampleOut));

        oOutputCoder = oFactory.makeStaticCodecFor(pSampleOut->GetPtr(), pSampleOut->GetSize());
        ASSERT_EQ(a_util::result::SUCCESS, oOutputCoder.isValid().getErrorCode());
        ASSERT_EQ(ddl::access_element::get_value(oOutputCoder, "i16Val").asInt16(), 2);

        ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->UnlockData(pSampleOut));

        // now check the trigger count of the listener
        ASSERT_EQ(oListener.GetSampleCount(), 2);

        {
            //Test Restarting statemachine
            oReceiver.GetStateMachine()->StopEvent(); ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, 5000));
            oReceiver.GetStateMachine()->RestartEvent(); ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_STARTUP, 5000));

            // Add signal mapping
            oReceiver.GetStateMachine()->StartupDoneEvent(); ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, 5000));
            ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignalDescription(
                "./files/engine.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));
            ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalMapping()->RegisterMappingConfiguration(
                "./files/fep_test.map", ISignalMapping::MF_MAPPING_FILE | ISignalMapping::MF_REPLACE));

            oReceiver.GetStateMachine()->InitializeEvent(); ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_INITIALIZING, 5000));

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

            // start modules
            oReceiver.GetStateMachine()->InitDoneEvent();
            oReceiver.GetStateMachine()->StartEvent();
            a_util::system::sleepMilliseconds(1); // to avoid spurious crashes...
            ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_RUNNING, 5000));

            oListener.ResetCount();
            oListener.SetExpectedSamples(3);
            oSampNotif.reset();

            // Transmit input signal
            ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(pSampleIn, true));
            a_util::system::sleepMicroseconds(1);
            ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(pSampleIn, true));
            a_util::system::sleepMicroseconds(1);
            ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(pSampleIn, true));
            oSampNotif.wait_for(a_util::chrono::milliseconds(5000));

            ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->LockData(hMappedOutSignal, pSampleOut));
            oOutputCoder = oFactory.makeStaticCodecFor(pSampleOut->GetPtr(), pSampleOut->GetSize());
            ASSERT_EQ(a_util::result::SUCCESS, oOutputCoder.isValid().getErrorCode());
            EXPECT_EQ(ddl::access_element::get_value(oOutputCoder, "i16Val").asInt16(), 3);

            ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->UnlockData(pSampleOut));
            oReceiver.GetStateMachine()->StopEvent(); ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, 5000));
        }
        delete pSampleIn;
    }

    // since the mapping driver uses cTimer for periodic triggers, we need to test FEP Mapping here
    // to be sure that periodic triggers work there too (using FEP Timing)
    {
        a_util::concurrency::semaphore oSampNotif;
        cSampListener oListener(&oSampNotif);

        cTestBaseModule oModule;
        ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions( "module")));
        oModule.GetStateMachine()->StartupDoneEvent();
        oModule.WaitForState(FS_IDLE);

        ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignalDescription(
            "./files/engine.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));
        ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalMapping()->RegisterMappingConfiguration(
            "./files/engine_triggers.map", ISignalMapping::MF_MAPPING_FILE | ISignalMapping::MF_REPLACE));
        ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, oModule.GetName()));

        oModule.GetStateMachine()->InitializeEvent(); ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_INITIALIZING, 5000));

        handle_t hMappedOutSignal = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("OutSignalTrigger",
            SD_Input, "OutStruct"), hMappedOutSignal));
        ASSERT_EQ(a_util::result::SUCCESS, oModule.GetUserDataAccess()->RegisterDataListener(&oListener, hMappedOutSignal));

        oListener.SetExpectedSamples(30);

        oModule.GetStateMachine()->InitDoneEvent(); ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_READY, 5000));
        oModule.GetStateMachine()->StartEvent(); ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_RUNNING, 5000));
        oSampNotif.wait_for(a_util::chrono::milliseconds(3000));
        oModule.GetStateMachine()->StopEvent(); ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_IDLE, 5000));
        LOG_INFO(a_util::strings::format("%d periodic triggers", oListener.GetSampleCount()).c_str());
        ASSERT_TRUE(oListener.GetSampleCount() >= 20 && oListener.GetSampleCount() <= 40); // STM is slow (expected 30)
    }
}

/**
 * @req_id "FEPSDK-1789"
 */
TEST(cTesterFEPMapping, TestFEPMappingWithoutTriggerWithTiming)
{
    // A mapped signal should be accessible if no trigger but the FEP timing is used
    cTestBaseModule oProducer;
    ASSERT_EQ(a_util::result::SUCCESS, oProducer.Create(cModuleOptions("Producer")));
    oProducer.GetStateMachine()->StartupDoneEvent();
    oProducer.WaitForState(FS_IDLE);
    
    cMappingTimingModule oConsumer;
    ASSERT_EQ(a_util::result::SUCCESS, oConsumer.Create(cModuleOptions("Consumer")));
    oConsumer.GetStateMachine()->StartupDoneEvent();
    oConsumer.WaitForState(FS_IDLE);

    cTestBaseModule oMaster;
    ASSERT_EQ(a_util::result::SUCCESS, oMaster.Create(cModuleOptions("FEP_Timing_Master")));
    oMaster.GetStateMachine()->StartupDoneEvent();
    oMaster.WaitForState(FS_IDLE);

    ASSERT_EQ(a_util::result::SUCCESS, oProducer.GetSignalRegistry()->RegisterSignalDescription(
        "./files/mapping_without_triggers.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));
    ASSERT_EQ(a_util::result::SUCCESS, oProducer.GetPropertyTree()->SetPropertyValue(
        FEP_TIMING_MASTER_PARTICIPANT, oMaster.GetName()));

    ASSERT_EQ(a_util::result::SUCCESS, oConsumer.GetSignalRegistry()->RegisterSignalDescription(
        "./files/mapping_without_triggers.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));
    ASSERT_EQ(a_util::result::SUCCESS, oConsumer.GetSignalMapping()->RegisterMappingConfiguration(
        "./files/mapping_without_triggers.map", ISignalMapping::MF_MAPPING_FILE | ISignalMapping::MF_REPLACE));
    ASSERT_EQ(a_util::result::SUCCESS, oConsumer.GetPropertyTree()->SetPropertyValue(
        FEP_TIMING_MASTER_PARTICIPANT, oMaster.GetName()));

    ASSERT_EQ(a_util::result::SUCCESS, oMaster.GetPropertyTree()->SetPropertyValue(
        FEP_TIMING_MASTER_PARTICIPANT, oMaster.GetName()));
    ASSERT_EQ(a_util::result::SUCCESS, oMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, "SYSTEM_TIME"));
    ASSERT_EQ(a_util::result::SUCCESS, oMaster.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, (float)1.0));

    oProducer.GetStateMachine()->InitializeEvent(); ASSERT_EQ(a_util::result::SUCCESS, 
        oProducer.WaitForState(FS_INITIALIZING, 5000));
    oConsumer.GetStateMachine()->InitializeEvent(); ASSERT_EQ(a_util::result::SUCCESS, 
        oConsumer.WaitForState(FS_INITIALIZING, 5000));
    oMaster.GetStateMachine()->InitializeEvent(); ASSERT_EQ(a_util::result::SUCCESS, 
        oMaster.WaitForState(FS_INITIALIZING, 5000));

    handle_t hInSignal = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oProducer.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("LightOrientation",
        SD_Output, "tFEP_VU_Coord"), hInSignal));

    // register the output signal 'LightPos'
    handle_t hLightPosSignal = NULL;
    fep::cUserSignalOptions oLightPosOptions("LightPos", fep::SD_Output, "tFEP_VU_PointCartesian");
    oProducer.GetSignalRegistry()->RegisterSignal(oLightPosOptions, hLightPosSignal);

    handle_t hOutSignal = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oConsumer.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("LightSource",
        SD_Input, "tFEP_VU_LightSource"), hOutSignal));

    fep::StepConfig step_config(1);
    oConsumer.GetTimingInterface()->RegisterStepListener("StepListenerConsumer", 
        step_config, &cMappingTimingModule::timingCallback_caller, &oConsumer);

    oConsumer.GetPropertyTree()->SetPropertyValue(FEP_TIMING_CLIENT_CONFIGURATION_FILE, "./files/timing_configuration.xml");

    a_util::system::sleepMilliseconds(500);

    oProducer.GetStateMachine()->InitDoneEvent(); ASSERT_EQ(a_util::result::SUCCESS, oProducer.WaitForState(FS_READY, 5000));
    oConsumer.GetStateMachine()->InitDoneEvent(); ASSERT_EQ(a_util::result::SUCCESS, oConsumer.WaitForState(FS_READY, 5000));
    oMaster.GetStateMachine()->InitDoneEvent(); ASSERT_EQ(a_util::result::SUCCESS, oMaster.WaitForState(FS_READY, 5000));

    oProducer.GetStateMachine()->StartEvent(); ASSERT_EQ(a_util::result::SUCCESS, oProducer.WaitForState(FS_RUNNING, 5000));
    oConsumer.GetStateMachine()->StartEvent(); ASSERT_EQ(a_util::result::SUCCESS, oConsumer.WaitForState(FS_RUNNING, 5000));
    oMaster.GetStateMachine()->StartEvent(); ASSERT_EQ(a_util::result::SUCCESS, oMaster.WaitForState(FS_RUNNING, 5000));

    // send input sample
    IUserDataSample* pSampleIn = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oProducer.GetUserDataAccess()->
        CreateUserDataSample(pSampleIn, hInSignal));

    const char* strDesc = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oConsumer.GetSignalRegistry()->ResolveSignalType("tFEP_VU_LightSource", strDesc));
    ddl::CodecFactory oFactory("tFEP_VU_LightSource", strDesc);
    ASSERT_EQ(a_util::result::SUCCESS, oFactory.isValid().getErrorCode());

    for (int i = 0; i <= 30; ++i)
    {
        test_ddl::tFEP_Test_Coord* pInStruct = (test_ddl::tFEP_Test_Coord*)pSampleIn->GetPtr();
        if (pInStruct)
        {
            pInStruct->f64H = i;
        }
        ASSERT_EQ(a_util::result::SUCCESS, oProducer.GetUserDataAccess()->TransmitData(pSampleIn, true));
        a_util::system::sleepMilliseconds(10);

        const fep::IUserDataSample* pSampleTest;
        ASSERT_EQ(a_util::result::SUCCESS, oConsumer.GetUserDataAccess()->LockData(hOutSignal, pSampleTest));
        if (pSampleTest->GetSignalHandle() == hOutSignal)
        {
            a_util::system::sleepMilliseconds(10);
            ddl::StaticCodec oOutputCoder = oFactory.makeStaticCodecFor(pSampleTest->GetPtr(), pSampleTest->GetSize());
            ASSERT_EQ(a_util::result::SUCCESS, oOutputCoder.isValid().getErrorCode());

            ASSERT_EQ(i, static_cast<int>(ddl::access_element::get_value(oOutputCoder, "sPosIntertial.f64H").asDouble()));
        }
    }

    oProducer.GetStateMachine()->StopEvent(); ASSERT_EQ(a_util::result::SUCCESS, oProducer.WaitForState(FS_IDLE, 5000));
    oConsumer.GetStateMachine()->StopEvent(); ASSERT_EQ(a_util::result::SUCCESS, oConsumer.WaitForState(FS_IDLE, 5000));
    oMaster.GetStateMachine()->StopEvent(); ASSERT_EQ(a_util::result::SUCCESS, oMaster.WaitForState(FS_IDLE, 5000));
}

/**
 * @req_id ""
 */
TEST(cTesterFEPMapping, TestFEPMappingTransformation)
{
    // Create signal mapping base modules
    cTestBaseModule oSender, oReceiver;
    a_util::concurrency::semaphore oSampNotif;
    cSampListener oListener(&oSampNotif);

    ASSERT_EQ(a_util::result::SUCCESS, oSender.Create(cModuleOptions("sender")));
    oSender.GetStateMachine()->StartupDoneEvent(); 
    ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_IDLE, 5000));
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetSignalRegistry()->RegisterSignalDescription(
        "./files/engine.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));

    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.Create(cModuleOptions("receiver")));
    oReceiver.GetStateMachine()->StartupDoneEvent(); 
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, 5000));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignalDescription(
        "./files/engine.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalMapping()->RegisterMappingConfiguration(
        "./files/engine_transformations.map", ISignalMapping::MF_MAPPING_FILE | ISignalMapping::MF_REPLACE));

    oSender.GetStateMachine()->InitializeEvent(); ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_INITIALIZING, 5000));
    oReceiver.GetStateMachine()->InitializeEvent(); ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_INITIALIZING, 5000));

    handle_t hInSignal = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("InSignal",
        SD_Output, "InStruct"), hInSignal));
    handle_t hMappedOutSignal = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("OutSignal2", SD_Input, "OutStruct"),
        hMappedOutSignal));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->RegisterDataListener(&oListener, hMappedOutSignal));
    {
        // check if the input signal was registered in the receiver
        IStringList* pInputs = NULL, *pOutputs = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->GetSignalNamesAndTypes(pInputs, pOutputs));
        ASSERT_EQ(pInputs->GetListSize(), 6);
        ASSERT_EQ(std::string(pInputs->GetStringAt(0)), "OutSignal2");
        ASSERT_EQ(std::string(pInputs->GetStringAt(1)), "OutStruct");
        ASSERT_EQ(std::string(pInputs->GetStringAt(2)), "InSignal");
        ASSERT_EQ(std::string(pInputs->GetStringAt(3)), "InStruct");
        ASSERT_EQ(std::string(pInputs->GetStringAt(4)), "MinimalSignal");
        ASSERT_EQ(std::string(pInputs->GetStringAt(5)), "MinimalStruct");
        delete pInputs;
        ASSERT_EQ(pOutputs->GetListSize(), 0);
        delete pOutputs;
    }

    const char* strOutDesc = NULL, *strInDesc = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->ResolveSignalType("OutStruct", strOutDesc));
    ddl::CodecFactory oOutFactory("OutStruct", strOutDesc);
    ASSERT_EQ(a_util::result::SUCCESS, oOutFactory.isValid().getErrorCode());
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetSignalRegistry()->ResolveSignalType("InStruct", strInDesc));
    ddl::CodecFactory oInFactory("InStruct", strInDesc);
    ASSERT_EQ(a_util::result::SUCCESS, oInFactory.isValid().getErrorCode());

    // start modules
    oSender.GetStateMachine()->InitDoneEvent();
    oSender.GetStateMachine()->StartEvent();
    oReceiver.GetStateMachine()->InitDoneEvent();
    oReceiver.GetStateMachine()->StartEvent();
    a_util::system::sleepMilliseconds(1); // to avoid spurious crashes...
    ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_RUNNING, 5000));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_RUNNING, 5000));

    oListener.SetExpectedSamples(1);

    // send input sample
    IUserDataSample* pSampleIn = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->
        CreateUserDataSample(pSampleIn, hInSignal));
    ddl::StaticCodec oInputCoder = oInFactory.makeStaticCodecFor(pSampleIn->GetPtr(), pSampleIn->GetSize());
    ASSERT_EQ(a_util::result::SUCCESS, oInputCoder.isValid().getErrorCode());
    ASSERT_EQ(ddl::access_element::set_value(oInputCoder, "enumVal", 20).getErrorCode(), a_util::result::SUCCESS);
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(pSampleIn, true));

    ASSERT_TRUE(oSampNotif.wait_for(a_util::chrono::milliseconds(500)));

    // read output sample
    const IUserDataSample* pSampleOut = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->LockData(hMappedOutSignal, pSampleOut));

    ddl::StaticCodec oOutputCoder = oOutFactory.makeStaticCodecFor(pSampleOut->GetPtr(), pSampleOut->GetSize());
    ASSERT_EQ(a_util::result::SUCCESS, oOutputCoder.isValid().getErrorCode());
    ASSERT_EQ(ddl::access_element::get_value(oOutputCoder, "enumVal").asInt16(), 10);

    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->UnlockData(pSampleOut));

    // cleanup
    oSender.GetStateMachine()->StopEvent();
    oReceiver.GetStateMachine()->StopEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_IDLE, 5000));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, 5000));
    delete pSampleIn;
    delete pSampleOut;
}

/**
 * @req_id ""
 */
TEST(cTesterFEPMapping, TestFEPMappingOneSourceToTwoTargets)
{
    // Create signal mapping base modules
    cTestBaseModule oSender, oReceiver;
    a_util::concurrency::semaphore oSampNotifVTDSpeed;
    a_util::concurrency::semaphore oSampNotifVTDSimTime;
    cSampListener oListenerVTDSpeed(&oSampNotifVTDSpeed);
    cSampListener oListenerVTDSimTime(&oSampNotifVTDSimTime);

    ASSERT_EQ(a_util::result::SUCCESS, oSender.Create(cModuleOptions("sender")));
    oSender.GetStateMachine()->StartupDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_IDLE, 5000));
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetSignalRegistry()->RegisterSignalDescription(
        "./files/fep_test_two_targets.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));

    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.Create(cModuleOptions("receiver")));
    oReceiver.GetStateMachine()->StartupDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, 5000));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignalDescription(
        "./files/fep_test_two_targets.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalMapping()->RegisterMappingConfiguration(
        "./files/fep_test_two_targets.map", ISignalMapping::MF_MAPPING_FILE | ISignalMapping::MF_REPLACE));

    oReceiver.GetPropertyTree()->SetPropertyValue("ComponentConfig.Timing.TimingMaster.strMasterElement", oReceiver.GetName());

    oSender.GetStateMachine()->InitializeEvent(); ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_INITIALIZING, 5000));
    oReceiver.GetStateMachine()->InitializeEvent(); ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_INITIALIZING, 5000));

    handle_t hObjectStateSignal = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("FEPVUProvider_ObjectState",
        SD_Output, "tFEP_VU_ObjectState"), hObjectStateSignal));
    handle_t hVTDSpeedSignal = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("sVTD_speed", SD_Input, "tVTD_speed"),
        hVTDSpeedSignal));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->RegisterDataListener(&oListenerVTDSpeed, hVTDSpeedSignal));
    handle_t hVTDSimTimeSignal = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("sVTD_simTime", SD_Input, "tVTD_simTime"),
        hVTDSimTimeSignal));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->RegisterDataListener(&oListenerVTDSimTime, hVTDSimTimeSignal));
    {
        // check if the input signal was registered in the receiver
        IStringList* pInputs = NULL, *pOutputs = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->GetSignalNamesAndTypes(pInputs, pOutputs));
        ASSERT_EQ(pInputs->GetListSize(), 6);
        ASSERT_EQ(std::string(pInputs->GetStringAt(0)), "sVTD_speed");
        ASSERT_EQ(std::string(pInputs->GetStringAt(1)), "tVTD_speed");
        ASSERT_EQ(std::string(pInputs->GetStringAt(2)), "FEPVUProvider_ObjectState");
        ASSERT_EQ(std::string(pInputs->GetStringAt(3)), "tFEP_VU_ObjectState");
        ASSERT_EQ(std::string(pInputs->GetStringAt(4)), "sVTD_simTime");
        ASSERT_EQ(std::string(pInputs->GetStringAt(5)), "tVTD_simTime");
        delete pInputs;
        ASSERT_EQ(pOutputs->GetListSize(), 0);
        delete pOutputs;
    }

    const char* strVTDSpeedDesc = NULL, *strVTDSimTimeDesc = NULL, *strObjectStateDesc;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->ResolveSignalType("tVTD_speed", strVTDSpeedDesc));
    ddl::CodecFactory oVTDSpeedFactory("tVTD_speed", strVTDSpeedDesc);
    ASSERT_EQ(a_util::result::SUCCESS, oVTDSpeedFactory.isValid().getErrorCode());
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetSignalRegistry()->ResolveSignalType("tVTD_simTime", strVTDSimTimeDesc));
    ddl::CodecFactory oVTDSimTimeFactory("tVTD_simTime", strVTDSimTimeDesc);
    ASSERT_EQ(a_util::result::SUCCESS, oVTDSimTimeFactory.isValid().getErrorCode());
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetSignalRegistry()->ResolveSignalType("tFEP_VU_ObjectState", strObjectStateDesc));
    ddl::CodecFactory oObjectStateFactory("tFEP_VU_ObjectState", strObjectStateDesc);
    ASSERT_EQ(a_util::result::SUCCESS, oObjectStateFactory.isValid().getErrorCode());

    // start modules
    oSender.GetStateMachine()->InitDoneEvent();
    oSender.GetStateMachine()->StartEvent();
    oReceiver.GetStateMachine()->InitDoneEvent();
    oReceiver.GetStateMachine()->StartEvent();
    a_util::system::sleepMilliseconds(1); // to avoid spurious crashes...
    ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_RUNNING, 5000));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_RUNNING, 5000));

    // send input sample
    IUserDataSample* pSampleObjectState = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->
        CreateUserDataSample(pSampleObjectState, hObjectStateSignal));
    ddl::StaticCodec oObjectStateCoder = oObjectStateFactory.makeStaticCodecFor(pSampleObjectState->GetPtr(), pSampleObjectState->GetSize());
    ASSERT_EQ(a_util::result::SUCCESS, oObjectStateCoder.isValid().getErrorCode());
    ASSERT_EQ(ddl::access_element::set_value(oObjectStateCoder, "f64SimTime", 42.0).getErrorCode(), a_util::result::SUCCESS);
    ASSERT_EQ(ddl::access_element::set_value(oObjectStateCoder, "ui32Id", 1).getErrorCode(), a_util::result::SUCCESS);
    ASSERT_EQ(ddl::access_element::set_value(oObjectStateCoder, "f64SpeedLongitudinal", 40.0).getErrorCode(), a_util::result::SUCCESS);
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(pSampleObjectState, true));

    // send sample again with different id and speed
    ASSERT_EQ(ddl::access_element::set_value(oObjectStateCoder, "ui32Id", 2).getErrorCode(), a_util::result::SUCCESS);
    ASSERT_EQ(ddl::access_element::set_value(oObjectStateCoder, "f64SpeedLongitudinal", 60.0).getErrorCode(), a_util::result::SUCCESS);
    ASSERT_EQ(a_util::result::SUCCESS, oSender.GetUserDataAccess()->TransmitData(pSampleObjectState, true));

    a_util::system::sleepMilliseconds(100);
    ASSERT_EQ(oListenerVTDSpeed.GetSampleCount(), 1); // Number of VTDSpeed Samples received; second input sample should not be getting mapped
    ASSERT_GE(oListenerVTDSimTime.GetSampleCount(), (uint32_t)10); // Number of VTDSimTime Samples received

    // read VTDSpeed output sample
    const IUserDataSample* pSampleVTDSpeed = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->LockData(hVTDSpeedSignal, pSampleVTDSpeed));

    ddl::StaticCodec oVTDSpeedCoder = oVTDSpeedFactory.makeStaticCodecFor(pSampleVTDSpeed->GetPtr(), pSampleVTDSpeed->GetSize());
    ASSERT_EQ(a_util::result::SUCCESS, oVTDSpeedCoder.isValid().getErrorCode());
    ASSERT_EQ(ddl::access_element::get_value(oVTDSpeedCoder, "SpeedLongitudinal__kph").asDouble(), 144.0);

    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->UnlockData(pSampleVTDSpeed));

    // read VTDSimTime output sample
    const IUserDataSample* pSampleVTDSimTime = NULL;
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->LockData(hVTDSimTimeSignal, pSampleVTDSimTime));

    ddl::StaticCodec oVTDSimTimeCoder = oVTDSimTimeFactory.makeStaticCodecFor(pSampleVTDSimTime->GetPtr(), pSampleVTDSimTime->GetSize());
    ASSERT_EQ(a_util::result::SUCCESS, oVTDSimTimeCoder.isValid().getErrorCode());
    ASSERT_EQ(ddl::access_element::get_value(oVTDSimTimeCoder, "f64SimTime").asDouble(), 42.0);

    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.GetUserDataAccess()->UnlockData(pSampleVTDSimTime));

    // cleanup
    oSender.GetStateMachine()->StopEvent();
    oReceiver.GetStateMachine()->StopEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oSender.WaitForState(FS_IDLE, 5000));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiver.WaitForState(FS_IDLE, 5000));
    delete pSampleObjectState;
    delete pSampleVTDSpeed;
    delete pSampleVTDSimTime;
}