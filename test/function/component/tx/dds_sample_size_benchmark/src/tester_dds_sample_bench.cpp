/**
 * Implementation of the tester for the FEP DDS Sample Size Benchmark.
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
* Test Case:   TestTransmitSamples
* Test ID:     10.1
* Test Title:  Transmit at Maximum Sample Size
* Description: Transmit samples of total size max_size - 1, max_size, and max_size + 1
* Strategy:  Test if the transmission failed at max_size + 1
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1513 FEPSDK-1514
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include "fep_test_common.h"
#include <ddl.h>

using namespace fep;

#ifdef WIN32
    #pragma push_macro("GetObject")
    #undef GetObject
#endif


const std::string s_strDescriptionBenchmark =
    "<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>"
    "<adtf:ddl xmlns:adtf=\"adtf\">"
    "    <header>"
    "        <language_version>3.00</language_version>"
    "        <author>AUDI AG</author>"
    "        <date_creation>27.04.2015</date_creation>"
    "        <date_change>06.05.2015</date_change>"
    "        <description>Test description file for #26412</description>"
    "    </header>"
    "    <units>"
    "    </units>"
    "    <datatypes>"
    "        <datatype description=\"predefined ADTF tBool datatype\" max=\"true\" min=\"false\" name=\"tBool\" size=\"8\" />"
    "        <datatype description=\"predefined ADTF tChar datatype\" max=\"127\" min=\"-128\" name=\"tChar\" size=\"8\" />"
    "        <datatype description=\"predefined ADTF tUInt8 datatype\" max=\"255\" min=\"0\" name=\"tUInt8\" size=\"8\" />"
    "        <datatype description=\"predefined ADTF tInt8 datatype\" max=\"127\" min=\"-128\" name=\"tInt8\" size=\"8\" />"
    "        <datatype description=\"predefined ADTF tUInt16 datatype\" max=\"65535\" min=\"0\" name=\"tUInt16\" size=\"16\" />"
    "        <datatype description=\"predefined ADTF tInt16 datatype\" max=\"32767\" min=\"-32768\" name=\"tInt16\" size=\"16\" />"
    "        <datatype description=\"predefined ADTF tUInt32 datatype\" max=\"4294967295\" min=\"0\" name=\"tUInt32\" size=\"32\" />"
    "        <datatype description=\"predefined ADTF tInt32 datatype\" max=\"2147483647\" min=\"-2147483648\" name=\"tInt32\" size=\"32\" />"
    "        <datatype description=\"predefined ADTF tUInt64 datatype\" max=\"18446744073709551615\" min=\"0\" name=\"tUInt64\" size=\"64\" />"
    "        <datatype description=\"predefined ADTF tInt64 datatype\" max=\"9223372036854775807\" min=\"-9223372036854775808\" name=\"tInt64\" size=\"64\" />"
    "        <datatype description=\"predefined ADTF tFloat32 datatype\" max=\"3.402823e+38\" min=\"-3.402823e+38\" name=\"tFloat32\" size=\"32\" />"
    "        <datatype description=\"predefined ADTF tFloat64 datatype\" max=\"1.797693e+308\" min=\"-1.797693e+308\" name=\"tFloat64\" size=\"64\" />"
    "    </datatypes>"
    "    <enums>"
    "    </enums>"
    "    <structs>"
    "        <struct alignment=\"1\" name=\"tFooStruct\" version=\"1\">"
    "            <element alignment=\"0\" arraysize=\"125000\" byteorder=\"LE\" bytepos=\"0\" name=\"fepChar1\" type=\"tUInt64\" />"
    "            <element alignment=\"0\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"1000000\" name=\"fepChar2\" type=\"tChar\" />"
    "        </struct>"
    "        <struct alignment=\"1\" name=\"tBarStruct\" version=\"1\">"
    "            <element alignment=\"0\" arraysize=\"125000\" byteorder=\"LE\" bytepos=\"0\" name=\"fepInt1\" type=\"tUInt64\" />"
    "            <element alignment=\"0\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"1000000\" name=\"fepInt2\" type=\"tChar\" />"
    "        </struct>"
    "        <struct alignment=\"1\" name=\"tQuxStruct\" version=\"1\">"
    "            <element alignment=\"0\" arraysize=\"131071\" byteorder=\"LE\" bytepos=\"0\" name=\"fepFloat1\" type=\"tUInt64\" />"
    "            <element alignment=\"0\" arraysize=\"7\" byteorder=\"LE\" bytepos=\"2097144\" name=\"fepFloat2\" type=\"tChar\" />"
    "        </struct>"
    "    </structs>"
    "    <streams />"
    "</adtf:ddl>";


// Tests begin

class cTestModule : public cTestBaseModule, public IUserDataListener
{
public:
    cTestModule() : m_pDDL(NULL), m_bInitCallbackWasExecuted(false), 
        m_evInitCallbackWasExecuted(),
        m_evFooReceived(), m_evBarReceived(), m_evQuxReceived(),
        m_hFooSignalIn(NULL), m_hBarSignalIn(NULL), m_hQuxSignalIn(NULL),
        m_hFooSignalOut(NULL), m_hBarSignalOut(NULL), m_hQuxSignalOut(NULL) { }

    ~cTestModule()
    {
        Destroy();
    }
    fep::Result SendData(handle_t hSignalOut, uint64_t nSize)
    {
        IUserDataSample *pSample = NULL;
        fep::Result nResult = GetUserDataAccess()->CreateUserDataSample(pSample);
        // Setting sample to the size of the biggest sample to send.
        if (fep::isOk(nResult))
        {
            nResult = pSample->SetSize(nSize);
        }
        if (fep::isOk(nResult))
        {
            nResult = pSample->SetSignalHandle(hSignalOut);
        }
        if (fep::isOk(nResult))
        {
            nResult = GetUserDataAccess()->TransmitData(pSample, true);
        }
        if (NULL != pSample)
        {
            delete pSample;
        }
        return nResult;
    }

public: //implements IUserDataListener
    fep::Result Update(const IUserDataSample *poSample)
    {
        if (poSample->GetSignalHandle() == m_hFooSignalIn)
        {
            m_evFooReceived.notify();
        }

        if (poSample->GetSignalHandle() == m_hBarSignalIn)
        {
            m_evBarReceived.notify();
        }

        if (poSample->GetSignalHandle() == m_hQuxSignalIn)
        {
            m_evQuxReceived.notify();
        }

        return ERR_NOERROR;
    }

public: //overrides cTestBaseModule
    fep::Result CleanUp(const fep::tState eOldState)
    {
        if (NULL != m_hFooSignalOut)
        {
            GetUserDataAccess()->UnregisterDataListener(this, m_hFooSignalOut);
            RETURN_IF_FAILED(GetSignalRegistry()->UnregisterSignal(m_hFooSignalOut));
        }
        if (NULL != m_hFooSignalIn)
        {
            GetUserDataAccess()->UnregisterDataListener(this, m_hFooSignalIn);
            RETURN_IF_FAILED(GetSignalRegistry()->UnregisterSignal(m_hFooSignalIn));
        }
        if (NULL != m_hBarSignalOut)
        {
            GetUserDataAccess()->UnregisterDataListener(this, m_hBarSignalOut);
            RETURN_IF_FAILED(GetSignalRegistry()->UnregisterSignal(m_hBarSignalOut));
        }
        if (NULL != m_hBarSignalIn)
        {
            GetUserDataAccess()->UnregisterDataListener(this, m_hBarSignalIn);
            RETURN_IF_FAILED(GetSignalRegistry()->UnregisterSignal(m_hBarSignalIn));
        }
        if (NULL != m_hQuxSignalOut)
        {
            GetUserDataAccess()->UnregisterDataListener(this, m_hQuxSignalOut);
            RETURN_IF_FAILED(GetSignalRegistry()->UnregisterSignal(m_hQuxSignalOut));
        }
        if (NULL != m_hQuxSignalIn)
        {
            GetUserDataAccess()->UnregisterDataListener(this, m_hQuxSignalIn);
            RETURN_IF_FAILED(GetSignalRegistry()->UnregisterSignal(m_hQuxSignalIn));
        }

        return ERR_NOERROR;
    }

    fep::Result ProcessInitializingEntry(const fep::tState eOldState)
    {
        RETURN_IF_FAILED(cModule::ProcessInitializingEntry(eOldState));

        RETURN_IF_FAILED(GetSignalRegistry()->RegisterSignalDescription(s_strDescriptionBenchmark.c_str()));
        
        RETURN_IF_FAILED(GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("FooStruct",
            SD_Output, "tFooStruct"), m_hFooSignalOut));
        LOG_INFO("1");
        RETURN_IF_FAILED(GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("FooStruct",
            SD_Input, "tFooStruct"), m_hFooSignalIn));
        LOG_INFO("2");
        RETURN_IF_FAILED(GetUserDataAccess()->RegisterDataListener(this, m_hFooSignalIn));

        RETURN_IF_FAILED(GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("BarStruct",
            SD_Output, "tBarStruct"), m_hBarSignalOut));
        LOG_INFO("3");
        RETURN_IF_FAILED(GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("BarStruct",
            SD_Input, "tBarStruct"), m_hBarSignalIn));
        LOG_INFO("4");
        RETURN_IF_FAILED(GetUserDataAccess()->RegisterDataListener(this, m_hBarSignalIn));

        RETURN_IF_FAILED(GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("QuxStruct",
            SD_Output,"tQuxStruct"), m_hQuxSignalOut));
        LOG_INFO("5");
        RETURN_IF_FAILED(GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("QuxStruct",
            SD_Input,"tQuxStruct"), m_hQuxSignalIn));
        LOG_INFO("6");
        RETURN_IF_FAILED(GetUserDataAccess()->RegisterDataListener(this, m_hQuxSignalIn));

        m_bInitCallbackWasExecuted = true;
        m_evInitCallbackWasExecuted.notify();
        return ERR_NOERROR;
    }

public:
    ddl::DDLDescription *m_pDDL;
    bool m_bInitCallbackWasExecuted;
    a_util::concurrency::semaphore m_evInitCallbackWasExecuted;

    a_util::concurrency::semaphore m_evFooReceived;
    a_util::concurrency::semaphore m_evBarReceived;
    a_util::concurrency::semaphore m_evQuxReceived;

    handle_t m_hFooSignalIn;
    handle_t m_hBarSignalIn;
    handle_t m_hQuxSignalIn;

    handle_t m_hFooSignalOut;
    handle_t m_hBarSignalOut;
    handle_t m_hQuxSignalOut;
};

/**
 * @req_id "FEPSDK-1513 FEPSDK-1514"
 */
TEST(cTesterDDSSampleSizeBenchmark, TestTransmitSamples)
{
    cTestModule oModule;

    int tmTimeOut = 5000; // 5s

    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions( "test_dds_max_sample_size")));
    // Set module to state INITIALIZING
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_IDLE));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->InitializeEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_INITIALIZING));

    // Ensure, the INITIALIZING callback was called
    // Note: Changed timeout from 5s to 500s as new ddl takes very long to compute sizes
    //       This is due to missing user defined types in this version
    oModule.m_evInitCallbackWasExecuted.wait_for(a_util::chrono::seconds(500)); // Wait 500 s
    ASSERT_TRUE(oModule.m_bInitCallbackWasExecuted);

    // Set module to state RUNNING
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->InitDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_READY));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->StartEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_RUNNING));

    // Send a sample and test, if it has been received. Test should succeed.
    ASSERT_EQ(a_util::result::SUCCESS, oModule.SendData(oModule.m_hFooSignalOut, 1000001));
    // Wait for the sample to arrive
    ASSERT_TRUE(oModule.m_evFooReceived.wait_for(a_util::chrono::milliseconds(tmTimeOut)));

    // Send a sample and test, if it has been received. Test should fail
    // because signal size does not match sample size.
    ASSERT_NE(a_util::result::SUCCESS, oModule.SendData(oModule.m_hBarSignalOut, 1000004));

    // Send a sample and test, if it has been received. Test should fail. Needs the limiter to be
    // turned off.
    ASSERT_NE(a_util::result::SUCCESS, oModule.SendData(oModule.m_hQuxSignalOut, 2097144));

    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->StopEvent());
    oModule.WaitForState(FS_IDLE);
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Destroy());
}
