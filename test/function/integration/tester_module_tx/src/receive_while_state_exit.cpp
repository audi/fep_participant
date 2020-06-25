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
* Test Case:   TestReceiveWhileStateExit
* Test ID:     1.5
* Test Title:  Test sending/receiving while exiting READY, RUNNING and INITIALIZING state
* Description: Test sending/receiving while exiting READY, RUNNING and INITIALIZING state and receiving local thrown incident
* Strategy:   Run FEP element and try to send in ProcessRunningExit()
*              
* Passed If:   End of test reached.
*              
* Ticket:      #FEPSDK-738
* Requirement: XXX
*/


#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::component_config;

const char* const s_strSignalDescription = 
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
    "       <struct alignment=\"1\" name=\"tTestSignal\" version=\"2\">"
    "           <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32Signal1\" type=\"tUInt32\" />"
    "           <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"4\" name=\"ui32Signal2\" type=\"tUInt32\" />"
    "       </struct>"
    "    </structs>"
    "    <streams>"
    "    </streams>"
    "</adtf:ddl>";


class cListener : public fep::IUserDataListener
{
private:
    a_util::concurrency::semaphore* m_pSampNotif;
    a_util::concurrency::fast_mutex m_oSampMutex;
    bool m_bSampleReceived;
public:
    cListener(a_util::concurrency::semaphore* pSampNotif) 
        : m_pSampNotif(pSampNotif), m_oSampMutex(), m_bSampleReceived(false) {}
    ~cListener() {}
public:
    fep::Result Update(const fep::IUserDataSample* poSample)
    {
        {
            a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oSampMutex);
            m_bSampleReceived = true;
        }
        m_pSampNotif->notify();
        return ERR_NOERROR;
    }
    bool SampleReceived() 
    { 
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oSampMutex);
        return m_bSampleReceived; 
    }
    void Reset() 
    { 
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oSampMutex);
        m_bSampleReceived = false; 
    }
};

class cLockableElement : public fep::cModule, public fep::cStateExitListener
{
private:
    a_util::concurrency::fast_mutex* m_pLock;
    a_util::concurrency::semaphore* m_pNotif;
    a_util::concurrency::semaphore* m_pIncidentNotif;
    bool m_bIncidentWasThrown;
    a_util::concurrency::fast_mutex m_oIncidentLock;

public:
    cLockableElement(a_util::concurrency::fast_mutex* pLock, a_util::concurrency::semaphore* pNotif, a_util::concurrency::semaphore* pIncidentNotif) 
        : m_pLock(pLock), m_pNotif(pNotif), m_pIncidentNotif(pIncidentNotif), m_bIncidentWasThrown(false), m_oIncidentLock()
    {

    }
    ~cLockableElement()
    {

    }

public:
    // state entry callbacks
    fep::Result ProcessStartupEntry(const fep::tState eOldState)
    {
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fElementVersion, 7.0);
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementName, GetName());
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementDescription, "ElementDescription");
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fFEPVersion, 117.0);
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementPlatform, "ElementPlatform");
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementContext, "ElementContext");
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fElementContextVersion, 17.0);
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementVendor, "ElementVendor");
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementDisplayName, "ElementDisplayName");
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementCompilationDate, "ElementCompilationDate");
        GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strTypeID, "TypeID");
        GetStateMachine()->RegisterStateExitListener(this);

        return ERR_NOERROR;
    }

    fep::Result ProcessInitializingEntry(const fep::tState eOldState)
    {
        // we continue to FS_READY
        GetStateMachine()->InitDoneEvent();
        return ERR_NOERROR;
    }

public:
    // state exit callbacks
    fep::Result ProcessInitializingExit(const fep::tState eNewState)
    {
        if(fep::FS_READY == eNewState)
        {
            m_pNotif->notify();
            a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oLockGuard(*m_pLock);
        }
        return ERR_NOERROR;
    }

    fep::Result ProcessReadyExit(const fep::tState eNewState)
    {
        if(fep::FS_IDLE == eNewState)
        {
            m_pNotif->notify();
            a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oLockGuard(*m_pLock);
        }
        return ERR_NOERROR;
    }

    fep::Result ProcessRunningExit(const fep::tState eNewState)
    {
        if (fep::FS_IDLE == eNewState)
        {
            m_pNotif->notify();
            a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oLockGuard(*m_pLock);
        }
        return ERR_NOERROR;
    }

public:
    fep::Result HandleLocalIncident(const int16_t nIncident, const fep::tSeverityLevel eSeverity, const char *strOrigin,
        int nLine, const char *strFile, const timestamp_t tmSimTime, const char* strDescription = NULL)
    {
        // we got incident and check whether its the one we need
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oIncidentLock);
        m_bIncidentWasThrown = true;
        std::string strTemp = "Received data while not enabled";
        std::string strDesc = strDescription;
        if (FSI_DRIVER_ISSUE != nIncident)
        {
            m_bIncidentWasThrown = false;
        }
        else if (SL_Warning != eSeverity)
        {
            m_bIncidentWasThrown = false;
        }
        else if (strDesc.find(strTemp) == std::string::npos)
        {
            m_bIncidentWasThrown = false;
        }
        if(m_bIncidentWasThrown)
        {
            m_pIncidentNotif->notify();
        }
        return ERR_NOERROR;
    }

public:
    fep::Result WaitForCorrectIncident(timestamp_t tmTimeout)
    {
        fep::Result nResult = ERR_FAILED;
        timestamp_t tmDeadline = a_util::system::getCurrentMilliseconds();
        while((tmDeadline < a_util::system::getCurrentMilliseconds())
            && (nResult != ERR_NOERROR))
        {
            {
                a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oIncidentLock);
                if(m_bIncidentWasThrown)
                {
                    m_bIncidentWasThrown = false;
                    nResult = ERR_NOERROR;
                }
            }
            a_util::system::sleepMilliseconds(100);
        }

        return nResult;
    }
    void Reset() 
    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oIncidentLock);
        m_bIncidentWasThrown = false; 
    }
};

// this test will currently fail
/**
 * @req_id "FEPSDK-1425"
 */
TEST(cTesterModuleTransmissionAdapter, TestReceiveWhileStateExit)
{
    a_util::concurrency::fast_mutex oLock;
    a_util::concurrency::semaphore oNotif;
    a_util::concurrency::semaphore oSampNotif;
    a_util::concurrency::semaphore oIncidentNotif;
    handle_t hSendSignalOut;
    handle_t hReceiveSignalIn;
    IUserDataSample* poSendSample;
    timestamp_t tmStateWait = 1 * 1000;
    timestamp_t tmIncidentWait = 5 * 1000;
    timestamp_t tmSampleWait = 5 * 1000;

    // set up elements
    cLockableElement oLockableElement(&oLock, &oNotif, &oIncidentNotif);
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.Create(cModuleOptions("TestReceiveWhileExit")));
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.SetStandAloneModeEnabled(true));
    oLockableElement.GetStateMachine()->StartupDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.WaitForState(fep::FS_IDLE, tmStateWait));

    cTestBaseModule oSendElement;
    ASSERT_EQ(a_util::result::SUCCESS, oSendElement.Create(cModuleOptions("Sender")));
    // register signals and create data to transmit
    ASSERT_EQ(a_util::result::SUCCESS, oSendElement.GetSignalRegistry()->RegisterSignalDescription(s_strSignalDescription));
    cUserSignalOptions oSignalOut("TestSignal", SD_Output, "tTestSignal");
    ASSERT_EQ(a_util::result::SUCCESS, oSendElement.GetSignalRegistry()->RegisterSignal(oSignalOut, hSendSignalOut));
    ASSERT_EQ(a_util::result::SUCCESS, oSendElement.GetUserDataAccess()->CreateUserDataSample(poSendSample, hSendSignalOut));

    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.GetSignalRegistry()->RegisterSignalDescription(s_strSignalDescription));
    cUserSignalOptions oSignalIn("TestSignal", SD_Input, "tTestSignal");
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.GetSignalRegistry()->RegisterSignal(oSignalIn, hReceiveSignalIn));
    cListener oListener(&oSampNotif);
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.GetUserDataAccess()->RegisterDataListener(&oListener, hReceiveSignalIn));
    ASSERT_EQ(a_util::result::SUCCESS, oSendElement.StartUpModule(true));
    ASSERT_EQ(a_util::result::SUCCESS, oSendElement.WaitForState(fep::FS_RUNNING, tmStateWait));

    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(oLock);
        oLockableElement.GetStateMachine()->InitializeEvent();
        oLockableElement.GetStateMachine()->InitDoneEvent();
        // we wait until exit listener is reached
        ASSERT_TRUE(oNotif.wait_for(a_util::chrono::milliseconds(tmStateWait)));

        // transmit sample, reception should fail and incident is expected
        ASSERT_EQ(a_util::result::SUCCESS, oSendElement.GetUserDataAccess()->TransmitData(poSendSample, true));
        ASSERT_FALSE(oSampNotif.wait_for(a_util::chrono::milliseconds(tmSampleWait)));
        ASSERT_FALSE(oListener.SampleReceived());
        ASSERT_TRUE(oIncidentNotif.wait_for(a_util::chrono::milliseconds(tmIncidentWait)));
    }

    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.WaitForState(fep::FS_READY, tmStateWait));
    oNotif.reset();
    oSampNotif.reset();

    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(oLock);
        oLockableElement.GetStateMachine()->StopEvent();
        // we wait until exit listener is reached
        ASSERT_TRUE(oNotif.wait_for(a_util::chrono::milliseconds(tmStateWait)));
        // reception should be a success
        ASSERT_EQ(a_util::result::SUCCESS, oSendElement.GetUserDataAccess()->TransmitData(poSendSample, true));
        oSampNotif.wait_for(a_util::chrono::milliseconds(tmSampleWait));
        ASSERT_TRUE(oListener.SampleReceived());
        oListener.Reset();
    }

    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.WaitForState(fep::FS_IDLE, tmStateWait));
    oLockableElement.GetStateMachine()->InitializeEvent();
    oLockableElement.GetStateMachine()->InitDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.WaitForState(fep::FS_READY, tmStateWait));
    oLockableElement.GetStateMachine()->StartEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.WaitForState(fep::FS_RUNNING, tmStateWait));

    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(oLock);
        oNotif.reset();
        oSampNotif.reset();

        oLockableElement.GetStateMachine()->StopEvent();
        // we wait until exit listener is reached
        ASSERT_TRUE(oNotif.wait_for(a_util::chrono::milliseconds(tmStateWait)));
        // transmitting should be a success
        ASSERT_EQ(a_util::result::SUCCESS, oSendElement.GetUserDataAccess()->TransmitData(poSendSample, true));
        oSampNotif.wait_for(a_util::chrono::milliseconds(tmSampleWait));
        ASSERT_TRUE(oListener.SampleReceived());
        oListener.Reset();
    }

    oLockableElement.GetStateMachine()->ShutdownEvent();
    oSendElement.GetStateMachine()->ShutdownEvent();
    oLockableElement.Destroy();
    oSendElement.Destroy();
}