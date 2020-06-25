/**
 * Implementation of the tester for the FEP Module
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
 * Test Case:   TestShutdownOneOfTwo
 * Test ID:     1.8
 * Test Title:  Test shutdown of one element while second keeps working
 * Description: This tests used to discover a problem concerning two fep elements within an executable
 * Strategy:    1) Construct and Create first FEP element (FEP Continue Running, abrev. FCR)
 *              2) Construct and Create second FEP Elemen (FEP Shutdown And Restart, abrev. FSR)
 *              3) Initialize FCR
 *              4) Initialize FSR
 *              5) Start FCR
 *              6) Stop FSR
 *              7) Shutdown FSR
 *              8) Destroy FSR
 *              9) Create FSR again
 *              10) Initialize FSR
 *              11) Stop FCR
 *              12) Start FCR
 * Passed If:   No errors occur
 * Ticket:      #39030,#39241
 * Requirement: FEPSDK-1806
 */

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;

#include "transmission_adapter/fep_data_sample_factory.h"

const std::string s_strDescriptionTemplateSkalar = std::string(
    "<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>"
    "<adtf:ddl xmlns:adtf=\"adtf\">"
    "    <header>"
    "        <language_version>3.00</language_version>"
    "        <author>AUDI AG</author>"
    "        <date_creation/>"
    "        <date_change/>"
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
    "    <struct alignment=\"1\" name=\"%s\" version=\"2\">"
    "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"%s\" type=\"%s\" />"
    "    </struct>"
    "    </structs>"
    "    <streams>"
    "    </streams>"
    "</adtf:ddl>");

class FepSRModule : public cModule
{
public:
    FepSRModule() 
        : cModule(), nTestResult(ERR_NOERROR)
    {
    }
public: // override cModule
    fep::Result Create(const cModuleOptions& oModuleOptions)
    {
        fep::Result nResult= cModule::Create(oModuleOptions);

        m_oTimer.setCallback(&FepSRModule::RunCyclic, *this);
        m_oTimer.setPeriod(100000);

        return nResult;
    }

  private:
    fep::Result RegisterSignals()
    {
        std::string strSignalType = std::string(GetName()) + "_signal";
        std::string strMediaDesc = a_util::strings::format(s_strDescriptionTemplateSkalar.c_str(), strSignalType.c_str(), "ui16Value", "tUInt16");
 
        RETURN_IF_FAILED(GetSignalRegistry()->RegisterSignalDescription(strMediaDesc.c_str()));

        RETURN_IF_FAILED(GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions(strSignalType.c_str(), SD_Output, strSignalType.c_str()), m_outSignalHandle));

        RETURN_IF_FAILED(fep::cDataSampleFactory::CreateSample(&m_poUserSample));

        RETURN_IF_FAILED(m_poUserSample->SetSignalHandle(m_outSignalHandle));

        size_t szSignal;
        RETURN_IF_FAILED(GetSignalRegistry()->GetSignalSampleSize(m_outSignalHandle, szSignal));

        RETURN_IF_FAILED(m_poUserSample->SetSize(szSignal));

        return ERR_NOERROR;
    }

    fep::Result UnregisterSignals()
    {
        delete m_poUserSample;
        m_poUserSample= NULL;

        return GetSignalRegistry()->UnregisterSignal(m_outSignalHandle);
    }

 public: // override cStateEntryListener

    fep::Result nTestResult;
    fep::Result ProcessIdleEntry(const fep::tState eOldState)
    {
        if (eOldState == FS_INITIALIZING || eOldState == FS_READY || eOldState == FS_RUNNING)
        {
            m_oTimer.stop();
            nTestResult = UnregisterSignals();
        }

        return ERR_NOERROR;
    }

    fep::Result ProcessInitializingEntry(const fep::tState eOldState)
    {
        nTestResult = RegisterSignals();

        if (fep::isOk(nTestResult))
        {
            GetStateMachine()->InitDoneEvent();
        }
        
        return ERR_NOERROR;
    }

    fep::Result ProcessRunningEntry(const fep::tState eOldState)
    {
        m_oTimer.start();
        return ERR_NOERROR;
    }

    void RunCyclic()
    {
        GetUserDataAccess()->TransmitData(m_poUserSample, true);
    }

private:
    handle_t m_outSignalHandle;
    fep::IUserDataSample* m_poUserSample;
    a_util::system::Timer m_oTimer;
};

/**
 * @req_id "FEPSDK-1542"
 */
TEST(TesterFepModule, TestShutdownOneOfTwo)
{
    // 0) Prepare
    FepSRModule* pFCR= NULL;
    FepSRModule* pFSR= NULL;

    // 1) Construct and Create first FEP element (FEP Continue Running, abrev. FCR)
    pFCR= new FepSRModule();
    pFCR->Create("FCR");

    // 2) Construct and Create second FEP Elemen (FEP Shutdown And Restart, abrev. FSR)
    pFSR= new FepSRModule();
    pFSR->Create("FSR");
    ASSERT_TRUE(ERR_NOERROR == pFCR->nTestResult);
    ASSERT_TRUE(ERR_NOERROR == pFSR->nTestResult);

    // 3) Initialize FCR
    pFCR->GetStateMachine()->StartupDoneEvent();
    pFCR->WaitForState(FS_IDLE);
    ASSERT_TRUE(ERR_NOERROR == pFCR->nTestResult);
    ASSERT_TRUE(ERR_NOERROR == pFSR->nTestResult);

    // 4) Initialize FSR
    pFSR->GetStateMachine()->StartupDoneEvent();
    pFSR->WaitForState(FS_IDLE);
    ASSERT_TRUE(ERR_NOERROR == pFCR->nTestResult);
    ASSERT_TRUE(ERR_NOERROR == pFSR->nTestResult);

    // 5) Start FCR
    pFCR->GetStateMachine()->InitializeEvent();
    pFCR->WaitForState(FS_READY);
    ASSERT_TRUE(ERR_NOERROR == pFCR->nTestResult);
    ASSERT_TRUE(ERR_NOERROR == pFSR->nTestResult);

    // 6) Stop FSR
    pFSR->GetStateMachine()->StopEvent();
    pFSR->WaitForState(FS_IDLE);
    ASSERT_TRUE(ERR_NOERROR == pFCR->nTestResult);
    ASSERT_TRUE(ERR_NOERROR == pFSR->nTestResult);

    // 7) Shutdown FSR
    pFSR->GetStateMachine()->ShutdownEvent();
    pFSR->WaitForState(FS_SHUTDOWN);
    ASSERT_TRUE(ERR_NOERROR == pFCR->nTestResult);
    ASSERT_TRUE(ERR_NOERROR == pFSR->nTestResult);

    // 8) Destroy FSR
    pFSR->Destroy();
    //delete pFSR; // The delete is not required, but should also work

    // 9) Create FSR again
    //pFSR= new FepSRModule(); // The recreate is not required, but should also work
    pFSR->Create("FSR");
    ASSERT_TRUE(ERR_NOERROR == pFCR->nTestResult);
    ASSERT_TRUE(ERR_NOERROR == pFSR->nTestResult);

    // 10) Initialize FSR
    pFSR->GetStateMachine()->StartupDoneEvent();
    pFSR->WaitForState(FS_IDLE);
    ASSERT_TRUE(ERR_NOERROR == pFCR->nTestResult);
    ASSERT_TRUE(ERR_NOERROR == pFSR->nTestResult);

    // 11) Stop FCR
    pFCR->GetStateMachine()->StopEvent();
    pFCR->WaitForState(FS_IDLE);
    ASSERT_TRUE(ERR_NOERROR == pFCR->nTestResult);
    ASSERT_TRUE(ERR_NOERROR == pFSR->nTestResult);

    // 12) Start FCR
    pFCR->GetStateMachine()->InitializeEvent();
    pFCR->WaitForState(FS_READY);
    ASSERT_TRUE(ERR_NOERROR == pFCR->nTestResult);
    ASSERT_TRUE(ERR_NOERROR == pFSR->nTestResult);

    // ... Wait for crash
    a_util::system::sleepMicroseconds(1 * 1000);
}
