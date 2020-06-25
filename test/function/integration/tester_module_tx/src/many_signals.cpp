/**
 * Implementation of the tester for the integration of FEP Module with FEP Transmission Adapter
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
* Test Case:   TestManySignals
* Test ID:     1.2
* Test Title:  Signal registration test
* Description: This test tests the amount of signals that can be registered
* Strategy:   Try to register many Signals, send a few samples for each signal, see, if the sample is "
*             received. The DDS_V2 driver is used explicitely because the original DDS driver is using a lot of memory "
*             and makes this test crash.
*              
* Passed If:   End of test reached.
*              
* Ticket:      -
* Requirement: FEPSDK-1571 FEPSDK-1572 FEPSDK-1573
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::component_config;
// TODO Use unique IDs for sending commands/states/etc. so that different testers running in the
// same network do not interfere
// TODO Use statemachine for more readable state changes

// DDL template, can be filled with structs using a_util::strings::format()
const std::string s_strDescription = std::string(
    "<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>"
    "<adtf:ddl xmlns:adtf=\"adtf\">"
    "    <header>"
    "        <language_version>3.00</language_version>"
    "        <author>AUDI AG</author>"
    "        <date_creation>07.04.2010</date_creation>"
    "        <date_change>07.04.2010</date_change>"
    "        <description>ADTF Common Description File</description>"
    "    </header>"
    "    <units>"
    "    </units>"
    "    <datatypes>"
    "        <datatype name=\"tBool\" size=\"8\" />"
    "        <datatype name=\"tChar\" size=\"8\" />"
    "        <datatype name=\"tUInt8\" size=\"8\" />"
    "        <datatype name=\"tInt8\" size=\"8\" />"
    "        <datatype name=\"tUInt16\" size=\"16\" />"
    "        <datatype name=\"tInt16\" size=\"16\" />"
    "        <datatype name=\"tUInt32\" size=\"32\" />"
    "        <datatype name=\"tInt32\" size=\"32\" />"
    "        <datatype name=\"tUInt64\" size=\"64\" />"
    "        <datatype name=\"tInt64\" size=\"64\" />"
    "        <datatype name=\"tFloat32\" size=\"32\" />"
    "        <datatype name=\"tFloat64\" size=\"64\" />"
    "    </datatypes>"
    "    <enums>"
    "    </enums>"
    "    <structs>"
    "      <struct alignment=\"1\" name=\"tSignal\" version=\"1\">" \
    "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\""
    "                 name=\"s\" type=\"tFloat64\" />"
    "      </struct>"
    "    </structs>"
    "    <streams>"
    "    </streams>"
    "</adtf:ddl>");

/**
 * This class will receive any signal and stores, how many samples for what signals it has received
 */
class cReceiveAll : public IUserDataListener
{
public: // Implements IUserDataListener
    
    fep::Result Update(const IUserDataSample* poSample)
    {

        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> sync(m_oMapMutex);
        if (m_mapReceiverCounts.find(poSample->GetSignalHandle()) == m_mapReceiverCounts.end())
        {
            // Initialize the entry, first time receiving the signal.
            m_mapReceiverCounts[poSample->GetSignalHandle()] = 1;
        }
        else
        {
            m_mapReceiverCounts[poSample->GetSignalHandle()]++;
        }
        return ERR_NOERROR;
    }

    std::map<handle_t, uint32_t> m_mapReceiverCounts;
    a_util::concurrency::recursive_mutex m_oMapMutex;
};


/**
 * @req_id "FEPSDK-1571 FEPSDK-1572 FEPSDK-1573"
 */
TEST(cTesterModuleTransmissionAdapter, TestManySignals)
{
 // name template for the test signals
    std::string strTemplateSignalName = "signal%d";
    // type for the test signals
    std::string strSignalType = "tSignal";
    // Some test parameters
    // - how many samples should be send per signal?
    uint32_t ui32AmountOfSamplesToSend = 10;
    // - how many signals should be registered?
    uint32_t ui32AmountOfSignalsToRegister = 90;

    // whats the cycle time for sending the signals?
    timestamp_t tmSleepTime = static_cast<timestamp_t>(10000);
    LOG_INFO(a_util::strings::format("Test will take at least %d seconds to run",
        ui32AmountOfSignalsToRegister * ui32AmountOfSamplesToSend * tmSleepTime / 1000000).c_str());

    // The modules under test
    cTestBaseModule oSenderModule;
    cTestBaseModule oReceiverModule;
    cReceiveAll oListener;
    // Reference list of expected handles of received signals
    std::list<handle_t> lstReceiveHandles;
    // List of handles for sending signals
    std::list<handle_t> lstSendHandles;

    // Initialize modules and put them to the state INITIALIZING
    ASSERT_EQ(a_util::result::SUCCESS, oSenderModule.Create(cModuleOptions(fep::TT_RTI_DDS, "Sender")));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiverModule.Create(cModuleOptions(fep::TT_RTI_DDS, "Receiver")));

    // Sample that will be used for all signals
    IUserDataSample * pSample = NULL;
    oSenderModule.GetUserDataAccess()->CreateUserDataSample(pSample);
    // "Memory" that will be transmitted
    double f64Value = 0;
    pSample->Attach(static_cast<void*>(&f64Value), sizeof(f64Value));

    ASSERT_EQ(a_util::result::SUCCESS, oSenderModule.GetStateMachine()->StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oReceiverModule.GetStateMachine()->StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSenderModule.WaitForState(FS_IDLE));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiverModule.WaitForState(FS_IDLE));
    ASSERT_EQ(a_util::result::SUCCESS, oSenderModule.GetStateMachine()->InitializeEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oReceiverModule.GetStateMachine()->InitializeEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSenderModule.WaitForState(FS_INITIALIZING));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiverModule.WaitForState(FS_INITIALIZING));

    // Just being curious how long registering a signal (and listener) takes. Not actually part of
    // the test.
    int64_t tmRegOut = 0;
    int64_t tmRegIn = 0;
    int64_t tmRegLi = 0;

    ASSERT_EQ(a_util::result::SUCCESS, oSenderModule.GetSignalRegistry()->RegisterSignalDescription(s_strDescription.c_str()));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiverModule.GetSignalRegistry()->RegisterSignalDescription(s_strDescription.c_str()));
    
    // Register a huge amount of signals
    fep::Result nRes = ERR_NOERROR;
    for (uint32_t ui32Idx = 0; ui32Idx < ui32AmountOfSignalsToRegister; ui32Idx++)
    {
        std::string strSignalName = a_util::strings::format(strTemplateSignalName.c_str(), ui32Idx);

        handle_t hHandle;
        // Register the signal and store its handle
        tmRegOut -= a_util::system::getCurrentMicroseconds();
        cUserSignalOptions oOptOut(strSignalName.c_str(), SD_Output, strSignalType.c_str());
        nRes |= oSenderModule.GetSignalRegistry()->RegisterSignal(oOptOut, hHandle);
        tmRegOut += a_util::system::getCurrentMicroseconds();
        lstSendHandles.push_back(hHandle);
        tmRegIn -= a_util::system::getCurrentMicroseconds();
        cUserSignalOptions oOptIn(strSignalName.c_str(), SD_Input, strSignalType.c_str());
        nRes |= oReceiverModule.GetSignalRegistry()->RegisterSignal( oOptIn, hHandle);
        tmRegIn += a_util::system::getCurrentMicroseconds();
        lstReceiveHandles.push_back(hHandle);
        tmRegLi -= a_util::system::getCurrentMicroseconds();
        nRes |= oReceiverModule.GetUserDataAccess()->RegisterDataListener(&oListener,
            hHandle);
        tmRegLi += a_util::system::getCurrentMicroseconds();
    }

    ASSERT_EQ(a_util::result::SUCCESS, nRes);

    // print out time averages
    tmRegOut /= ui32AmountOfSignalsToRegister;
    tmRegIn /= ui32AmountOfSignalsToRegister;
    tmRegLi /= ui32AmountOfSignalsToRegister;
    LOG_INFO(a_util::strings::format("Average time to register signal: %fms (in)", tmRegIn / 1000.0).c_str());
    LOG_INFO(a_util::strings::format("Average time to register signal: %fms (out)", tmRegOut / 1000.0).c_str());
    LOG_INFO(a_util::strings::format("Average time to register listener: %fms", tmRegOut / 1000.0).c_str());
    
    // Put the modules to state RUNNING
    int64_t tmStartup = -a_util::system::getCurrentMicroseconds();
    ASSERT_EQ(a_util::result::SUCCESS, oSenderModule.GetStateMachine()->InitDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSenderModule.GetStateMachine()->StartEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oReceiverModule.GetStateMachine()->InitDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oReceiverModule.GetStateMachine()->StartEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSenderModule.WaitForState(FS_RUNNING));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiverModule.WaitForState(FS_RUNNING));
    tmStartup += a_util::system::getCurrentMicroseconds();
    tmStartup /= 2;
    LOG_INFO(a_util::strings::format("Average time to startup: %fms", tmStartup / 1000.0).c_str());

    // Send some samples
    for (uint32_t ui32Idx = 0; ui32Idx < ui32AmountOfSamplesToSend; ui32Idx++)
    {
        for (std::list<handle_t>::iterator pIter = lstSendHandles.begin();
            lstSendHandles.end() != pIter; pIter++)
        {
            nRes |= pSample->SetSignalHandle(*pIter);
            nRes |= oSenderModule.GetUserDataAccess()->TransmitData(pSample, true);
            // Give the system a little bit of time to process
            a_util::system::sleepMilliseconds(static_cast<uint32_t>(tmSleepTime / 1000));
        }
    }
    ASSERT_EQ(a_util::result::SUCCESS, nRes);

    // Shutdown everything
    // Just being curious how long unregistering a signal (and listener) takes. Not actually part of
    // the test.
    int64_t tmUnregOut = 0;
    int64_t tmUnregIn = 0;
    int64_t tmUnregLi = 0;
    int64_t tmStop = -a_util::system::getCurrentMicroseconds();
    ASSERT_EQ(a_util::result::SUCCESS, oSenderModule.GetStateMachine()->StopEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oReceiverModule.GetStateMachine()->StopEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oSenderModule.WaitForState(FS_IDLE));
    ASSERT_EQ(a_util::result::SUCCESS, oReceiverModule.WaitForState(FS_IDLE));
    tmStop += a_util::system::getCurrentMicroseconds();
    tmStop /= 2;
    LOG_INFO(a_util::strings::format("Average time to stop: %fms", tmStop / 1000.0).c_str());

    for (std::list<handle_t>::iterator pIter = lstSendHandles.begin();
        lstSendHandles.end() != pIter; pIter++)
    {
        tmUnregOut -= a_util::system::getCurrentMicroseconds();
        nRes |= oSenderModule.GetSignalRegistry()->UnregisterSignal(*pIter);
        tmUnregOut += a_util::system::getCurrentMicroseconds();
    }
    ASSERT_EQ(a_util::result::SUCCESS, nRes);

    for (std::list<handle_t>::iterator pIter = lstReceiveHandles.begin();
        lstReceiveHandles.end() != pIter; pIter++)
    {
        tmUnregIn -= a_util::system::getCurrentMicroseconds();
        nRes |= oReceiverModule.GetUserDataAccess()->UnregisterDataListener(&oListener,
            *pIter);
        tmUnregIn += a_util::system::getCurrentMicroseconds();
        tmUnregLi -= a_util::system::getCurrentMicroseconds();
        nRes |= oReceiverModule.GetSignalRegistry()->UnregisterSignal(*pIter);
        tmUnregLi += a_util::system::getCurrentMicroseconds();
    }
    ASSERT_EQ(a_util::result::SUCCESS, nRes);

    tmUnregOut /= ui32AmountOfSignalsToRegister;
    tmUnregIn /= ui32AmountOfSignalsToRegister;
    tmUnregLi /= ui32AmountOfSignalsToRegister;
    LOG_INFO(a_util::strings::format("Average time to unregister signal: %fms (in)", tmUnregOut / 1000.0).c_str());
    LOG_INFO(a_util::strings::format("Average time to unregister signal: %fms (out)", tmUnregIn / 1000.0).c_str());
    LOG_INFO(a_util::strings::format("Average time to unregister listner: %fms", tmUnregLi / 1000.0).c_str());

    // Analyze result
    std::map<handle_t, uint32_t> mapReceiverCounts = oListener.m_mapReceiverCounts;
    std::list<handle_t> lstListenerHandles;
    for (std::map<handle_t, uint32_t>::iterator pIter = mapReceiverCounts.begin();
        mapReceiverCounts.end() != pIter; pIter++)
    {
        lstListenerHandles.push_back(pIter->first);
    }
    lstReceiveHandles.sort();
    lstListenerHandles.sort();
    // Check whether all sent signals have been received.
    ASSERT_EQ(lstListenerHandles, lstReceiveHandles);
    // Check whether all signals have been received.
    bool bVal = true;
    for (std::map<handle_t, uint32_t>::iterator pIter = mapReceiverCounts.begin();
        mapReceiverCounts.end() != pIter; pIter++)
    {
        bVal |= pIter->second == ui32AmountOfSamplesToSend;
    }
    ASSERT_TRUE(bVal);
    delete pSample;
}