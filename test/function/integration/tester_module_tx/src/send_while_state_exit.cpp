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
* Test Case:   TestSendWhileStateExit
* Test ID:     1.4
* Test Title:  Test sending/receiving while exiting READY, RUNNING and INITIALIZING state
* Description: Test sending/receiving while exiting READY, RUNNING and INITIALIZING state and receiving local thrown incident
* Strategy:   Run FEP element and try to send in ProcessRunningExit()
*              
* Passed If:   End of test reached.
*              
* Ticket:      #FEPSDK-738
* Requirement: FEPSDK-1574 FEPSDK-1575
*/


#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::component_config;

/*--------------------------------------------------------------------------------------------------------------------*/
/*//////// TestSendWhileRunningExit //////////////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------------------------------------------------*/
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

/**
 * @req_id "FEPSDK-1425"
 */
TEST(cTesterModuleTransmissionAdapter, TestSendWhileStateExit)
{
    class cLockableElement : public fep::cModule, public fep::cStateExitListener
{
private:
    a_util::concurrency::fast_mutex* m_pLock;
    a_util::concurrency::semaphore* m_pNotif;
    bool m_bIncidentWasThrown;
    a_util::concurrency::fast_mutex m_oIncidentLock;

public:
    cLockableElement(a_util::concurrency::fast_mutex* pLock, a_util::concurrency::semaphore* pNotif) : m_pLock(pLock), m_pNotif(pNotif), m_bIncidentWasThrown(false), m_oIncidentLock()
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

    fep::Result ProcessReadyEntry(const fep::tState eOldState)
    {
        m_pNotif->notify();
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
        std::string strTemp = "wrong state";
        if (FSI_TRANSM_RX_INVALID_STATE != nIncident)
        {
            m_bIncidentWasThrown = false;
        }
        else if (SL_Critical_Local != eSeverity)
        {
            m_bIncidentWasThrown = false;
        }
        else if (a_util::strings::compare(strTemp.c_str(), strDescription) != 0)
        {
            m_bIncidentWasThrown = false;
        }
        return ERR_NOERROR;
    }

public:
    fep::Result WaitForState(tState eState, timestamp_t tmTimeout = -1)
    {
        return ::WaitForState(GetStateMachine(), eState, tmTimeout);
    }

    void Reset() 
    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oIncidentLock);
        m_bIncidentWasThrown = false; 
    }
};

    a_util::concurrency::fast_mutex oLock;
    a_util::concurrency::semaphore oNotif;
    handle_t hSendSignalOut;
    IUserDataSample* poSendSample;
    timestamp_t tmStateWait = 1 * 1000;

    cLockableElement oLockableElement(&oLock, &oNotif);
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.Create(cModuleOptions("TestSendWhileExit")));
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.SetStandAloneModeEnabled(true));
    oLockableElement.GetStateMachine()->StartupDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.WaitForState(fep::FS_IDLE, tmStateWait));

    // register signal and create data to transmit
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.GetSignalRegistry()->RegisterSignalDescription(s_strSignalDescription));
    cUserSignalOptions oSigOptions("TestSignal", SD_Output, "tTestSignal");
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.GetSignalRegistry()->RegisterSignal(oSigOptions, hSendSignalOut));
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.GetUserDataAccess()->CreateUserDataSample(poSendSample, hSendSignalOut));

    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(oLock);
        oLockableElement.GetStateMachine()->InitializeEvent();
        oLockableElement.GetStateMachine()->InitDoneEvent();
        // we wait until exit listener is reached
        ASSERT_TRUE(oNotif.wait_for(a_util::chrono::milliseconds(tmStateWait)));

        // transmitting should fail
        ASSERT_NE(a_util::result::SUCCESS, oLockableElement.GetUserDataAccess()->TransmitData(poSendSample, true));
        oNotif.reset();
    }
    
    ASSERT_TRUE(oNotif.wait_for(a_util::chrono::milliseconds(tmStateWait * 10)));
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.GetUserDataAccess()->TransmitData(poSendSample, true));

    
    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(oLock);
        oLockableElement.GetStateMachine()->StopEvent();
        // we wait until exit listener is reached
        ASSERT_TRUE(oNotif.wait_for(a_util::chrono::milliseconds(tmStateWait)));
        // transmitting should be a success
        ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.GetUserDataAccess()->TransmitData(poSendSample, true));
    }
   

    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.WaitForState(fep::FS_IDLE, tmStateWait));
    // now to FS_RUNNING
    oLockableElement.GetStateMachine()->InitializeEvent();
    oLockableElement.GetStateMachine()->InitDoneEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.WaitForState(fep::FS_READY, tmStateWait));
    oLockableElement.GetStateMachine()->StartEvent();
    ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.WaitForState(fep::FS_RUNNING, tmStateWait));

    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(oLock);
        oNotif.reset();

        oLockableElement.GetStateMachine()->StopEvent();
        // we wait until exit listener is reached
        ASSERT_TRUE(oNotif.wait_for(a_util::chrono::milliseconds(tmStateWait)));
        // transmitting should be a success
        ASSERT_EQ(a_util::result::SUCCESS, oLockableElement.GetUserDataAccess()->TransmitData(poSendSample, true));
    }
    
    oLockableElement.GetStateMachine()->ShutdownEvent();
    oLockableElement.Destroy();
}