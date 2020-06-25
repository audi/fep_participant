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
 * Test Case:   LeakModule
 * Test ID:     1.1
 * Test Title:  Test the creation of cModule
 * Description: 
 * Strategy:
 * Passed If:
 * Ticket:      
 * Requirement: TODO
 */

#include "gtest/gtest.h"
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
#include <a_util/system/timer.h>
using namespace fep;

// Common Data
static const char* s_strSimpleSignalName= "SimpleSignalName";
static const char* s_strSimpleSignalType= "SimpleSignalType";
static const char* s_strSimpleSignalDesc=
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
    "        <struct alignment=\"1\" name=\"SimpleSignalType\" version=\"2\">"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"CommandUuid\" type=\"tUInt64\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"8\" name=\"CommandMagic\" type=\"tUInt32\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"12\" name=\"CommandIndex\" type=\"tUInt32\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"16\" name=\"BoolValue\" type=\"tBool\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"17\" name=\"CharValue\" type=\"tChar\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"18\" name=\"UInt8Value\" type=\"tUInt8\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"19\" name=\"Int8Value\" type=\"tInt8\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"20\" name=\"UInt16Value\" type=\"tUInt16\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"22\" name=\"Int16Value\" type=\"tInt16\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"24\" name=\"UInt32Value\" type=\"tUInt32\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"28\" name=\"Int32Value\" type=\"tInt32\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"32\" name=\"UInt64Value\" type=\"tUInt64\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"40\" name=\"Int64Value\" type=\"tInt64\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"48\" name=\"Float32Value\" type=\"tFloat32\" />"
    "            <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"52\" name=\"Float64Value\" type=\"tFloat64\" />"
    "        </struct>"
    "    </structs>"
    "    <streams>"
    "    </streams>"
    "</adtf:ddl>";

#pragma pack(push,1)
struct sSimpleSignalType
{
    uint64_t CommandUuid;
    uint32_t CommandMagic;
    uint32_t CommandIndex;
    bool BoolValue;
    char CharValue;
    uint8_t UInt8Value;
    int8_t Int8Value;
    uint16_t UInt16Value;
    int16_t Int16Value;
    uint32_t UInt32Value;
    int32_t Int32Value;
    uint64_t UInt64Value;
    int64_t Int64Value;
    float Float32Value;
    double Float64Value;
};
#pragma pack(pop)

// Helpers
static fep::Result fillElementHeader(cModule* pModule)
{
    IPropertyTree* pPropertyTree=pModule->GetPropertyTree();

    fep::Result nResult= ERR_NOERROR;

    // Fill the participant header
    nResult |= pPropertyTree->SetPropertyValue(fep::g_strElementHeaderPath_fElementVersion,
        1.0);
    nResult |= pPropertyTree->SetPropertyValue(fep::g_strElementHeaderPath_strElementName,
        pModule->GetName());
    nResult |= pPropertyTree->SetPropertyValue(fep::g_strElementHeaderPath_strElementDescription,
        (std::string("This is ") + pModule->GetName()).c_str());
    nResult |= pPropertyTree->SetPropertyValue(fep::g_strElementHeaderPath_fFEPVersion,
        static_cast<float>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<float>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10);
   nResult |= pPropertyTree->SetPropertyValue(fep::g_strElementHeaderPath_strElementPlatform,
        FEP_SDK_PARTICIPANT_PLATFORM_STR);
   nResult |= pPropertyTree->SetPropertyValue(fep::g_strElementHeaderPath_strElementContext,
        "Test");
   nResult |= pPropertyTree->SetPropertyValue(fep::g_strElementHeaderPath_fElementContextVersion,
       static_cast<float>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
       static_cast<float>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10);
   nResult |= pPropertyTree->SetPropertyValue(fep::g_strElementHeaderPath_strElementVendor,
        "AEV");
   nResult |= pPropertyTree->SetPropertyValue(fep::g_strElementHeaderPath_strElementDisplayName,
        pModule->GetName());
   nResult |= pPropertyTree->SetPropertyValue(fep::g_strElementHeaderPath_strElementCompilationDate,
        __DATE__);
   nResult |= pPropertyTree->SetPropertyValue(fep::g_strElementHeaderPath_strTypeID,
       "b79baccd-926a-48fb-aedc-c19ca3141330");

   return nResult;
}

// TestCase 1: Erstellen und Zerstören von FEP-Elementen
TEST(LeakModule, CreateAndDestroyModule)
{
    cModule oMod;
    ASSERT_EQ(oMod.Create(cModuleOptions("TestModule")), ERR_NOERROR);

    a_util::system::sleepMilliseconds(1);

    ASSERT_EQ(oMod.Destroy(), ERR_NOERROR);
}


// TestCase 2: Senden und Empfangen von Signalen
TEST(LeakModule, SendAndReceiveSignals)
{
    class cSender : public cModule
    {
    public:
        cSender()
            : m_nSendCount(0)
        {
        }

        void RunCyclic()
        {
            sSimpleSignalType* pSimpleSignal = reinterpret_cast<sSimpleSignalType*>(m_pSendSample->GetPtr());

            pSimpleSignal->Float64Value= 1.11;

            GetUserDataAccess()->TransmitData(m_pSendSample, true);

            ++m_nSendCount;
        }

    public: // override cStateEntryListener
        fep::Result CleanUp(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::CleanUp(eOldState);

            nResult|= GetSignalRegistry()->UnregisterSignal(m_hSendHandle);

            delete m_pSendSample;
            m_pSendSample= NULL;

            return nResult;
        }
        fep::Result ProcessStartupEntry(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::ProcessStartupEntry(eOldState);

            fillElementHeader(this);

            nResult|= GetSignalRegistry()->RegisterSignalDescription(s_strSimpleSignalDesc);
            nResult|= GetSignalRegistry()->RegisterSignal(cUserSignalOptions(s_strSimpleSignalName, SD_Output, s_strSimpleSignalType), m_hSendHandle);
            nResult|= GetUserDataAccess()->CreateUserDataSample(m_pSendSample, m_hSendHandle);

            m_oTimer.setCallback(&cSender::RunCyclic, *this);
            m_oTimer.setPeriod(100 * 1000);

            if (fep::isOk(nResult))
            {
                GetStateMachine()->StartupDoneEvent();
            }

            return nResult;
        }
        fep::Result ProcessIdleEntry(const fep::tState eOldState)
        {
            if (eOldState != fep::FS_STARTUP) m_oTimer.stop();
            return fep::ERR_NOERROR;
        }
        fep::Result ProcessInitializingEntry(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::ProcessInitializingEntry(eOldState);

            if (fep::isOk(nResult))
            {
                GetStateMachine()->InitDoneEvent();
            }

            return nResult;
        }
        fep::Result ProcessRunningEntry(const fep::tState eOldState)
        {
            m_oTimer.start();
            return fep::ERR_NOERROR;
        }

    private:
        handle_t m_hSendHandle;
        fep::IUserDataSample * m_pSendSample;
        a_util::system::Timer m_oTimer;
    public:
        size_t m_nSendCount;
    };

    class cReceiver : public cModule, public IUserDataListener
    {
    public:
        cReceiver()
            : m_nReceiveCount(0)
        {
        }

    public: // override cStateEntryListener
        fep::Result CleanUp(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::CleanUp(eOldState);

            nResult|= GetUserDataAccess()->UnregisterDataListener(this, m_hReceiveHandle);
            nResult|= GetSignalRegistry()->UnregisterSignal(m_hReceiveHandle);

            return nResult;
        }
        fep::Result ProcessStartupEntry(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::ProcessStartupEntry(eOldState);

            fillElementHeader(this);

            nResult|= GetSignalRegistry()->RegisterSignalDescription(s_strSimpleSignalDesc);
            nResult|= GetSignalRegistry()->RegisterSignal(cUserSignalOptions(s_strSimpleSignalName, SD_Input, s_strSimpleSignalType), m_hReceiveHandle);
            nResult|= GetUserDataAccess()->RegisterDataListener(this, m_hReceiveHandle);

            if (fep::isOk(nResult))
            {
                GetStateMachine()->StartupDoneEvent();
            }

            return nResult;
        }
        fep::Result ProcessInitializingEntry(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::ProcessInitializingEntry(eOldState);

            if (fep::isOk(nResult))
            {
                GetStateMachine()->InitDoneEvent();
            }

            return nResult;
        }

    public: // implement IUserDataListener
        fep::Result Update(const IUserDataSample* poSample)
        {
            // Not interested in the sample
            ++m_nReceiveCount;

            return ERR_NOERROR;
        }

    private:
        handle_t m_hReceiveHandle;
    public:
        size_t m_nReceiveCount;
    };

    // Create: <> -> STARTUP -> IDLE
    cReceiver oReceiver;
    ASSERT_EQ(oReceiver.Create(cModuleOptions("Receiver")), ERR_NOERROR);
    cSender oSender;
    ASSERT_EQ(oSender.Create(cModuleOptions("Sender")), ERR_NOERROR);

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oReceiver.GetStateMachine()->GetState(), FS_IDLE);
    ASSERT_EQ(oSender.GetStateMachine()->GetState(), FS_IDLE);

    // Initialize: IDLE -> READY
    oReceiver.GetStateMachine()->InitializeEvent();
    oSender.GetStateMachine()->InitializeEvent();

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oReceiver.GetStateMachine()->GetState(), FS_READY);
    ASSERT_EQ(oSender.GetStateMachine()->GetState(), FS_READY);

    // Start: READY -> RUNNING
    oReceiver.GetStateMachine()->StartEvent();
    oSender.GetStateMachine()->StartEvent();

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oReceiver.GetStateMachine()->GetState(), FS_RUNNING);
    ASSERT_EQ(oSender.GetStateMachine()->GetState(), FS_RUNNING);

    // Stop: RUNNING -> READY
    oReceiver.GetStateMachine()->StopEvent();
    oSender.GetStateMachine()->StopEvent();

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oReceiver.GetStateMachine()->GetState(), FS_IDLE);
    ASSERT_EQ(oSender.GetStateMachine()->GetState(), FS_IDLE);

    // Destroy: READY -> SHUTDOWN
    ASSERT_EQ(oReceiver.Destroy(), ERR_NOERROR);
    ASSERT_EQ(oSender.Destroy(), ERR_NOERROR);

    // Check Results
    EXPECT_GT(oReceiver.m_nReceiveCount, 0);
    EXPECT_GT(oSender.m_nSendCount, 0);
    
    // Account for some lost samples 
    EXPECT_LE(oSender.m_nSendCount - oReceiver.m_nReceiveCount, 5);
}


// TestCase 3a: Senden und Empfangen von Localen Incidents
TEST(LeakModule, SendAndReceiveIncidentsLocal)
{
    class cIncidentLocal : public cModule
    {
    public:
        cIncidentLocal()
            : m_nSendCount(0)
            , m_nReceiveCount(0)
        {
        }

    public: // override cStateEntryListener
        fep::Result CleanUp(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::CleanUp(eOldState);

            return nResult;
        }
        fep::Result ProcessStartupEntry(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::ProcessStartupEntry(eOldState);

            fillElementHeader(this);

            m_oTimer.setCallback(&cIncidentLocal::RunCyclic, *this);
            m_oTimer.setPeriod(100 * 1000);

            if (fep::isOk(nResult))
            {
                GetStateMachine()->StartupDoneEvent();
            }

            return nResult;
        }
        fep::Result ProcessIdleEntry(const fep::tState eOldState)
        {
            if (eOldState != fep::FS_STARTUP) m_oTimer.stop();
            return fep::ERR_NOERROR;
        }
        fep::Result ProcessInitializingEntry(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::ProcessInitializingEntry(eOldState);

            if (fep::isOk(nResult))
            {
                GetStateMachine()->InitDoneEvent();
            }

            return nResult;
        }
        fep::Result ProcessRunningEntry(const fep::tState eOldState)
        {
            m_oTimer.start();
            return fep::ERR_NOERROR;
        }

        void RunCyclic()
        {
            GetIncidentHandler()->InvokeIncident(10815, fep::SL_Critical_Local,
                            "Test Local Incident", NULL, 0, NULL);

            ++m_nSendCount;
        }

    private: // overrides cModule
        fep::Result HandleLocalIncident(const int16_t nIncident,
                                                    const fep::tSeverityLevel eSeverity,
                                                    const char *strOrigin,
                                                    int nLine,
                                                    const char *strFile,
                                                    const timestamp_t tmSimTime,
                                                    const char* strDescription = NULL)
        {
            fep::Result nResult = ERR_NOERROR;

            //std::cerr << "Received Incident:"
            //          << " strOrigin=" << (strOrigin ? strOrigin : "<NULL>")
            //          << " strFile=" << (strFile ? strFile : "<NULL>")
            //          << " strDescription=" << (strDescription ? strDescription : "<NULL>")
            //          << std::endl;
            ++m_nReceiveCount;

            return nResult;
        }
        a_util::system::Timer m_oTimer;

    public:
        size_t m_nSendCount;
        size_t m_nReceiveCount;
   };

    // Create: <> -> STARTUP -> IDLE
    cIncidentLocal oIncidentLocal;
    ASSERT_EQ(oIncidentLocal.Create(cModuleOptions("IncidentLocal")), ERR_NOERROR);

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oIncidentLocal.GetStateMachine()->GetState(), FS_IDLE);

    // Initialize: IDLE -> READY
    oIncidentLocal.GetStateMachine()->InitializeEvent();

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oIncidentLocal.GetStateMachine()->GetState(), FS_READY);

    // Start: READY -> RUNNING
    oIncidentLocal.GetStateMachine()->StartEvent();

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oIncidentLocal.GetStateMachine()->GetState(), FS_RUNNING);

    // Stop: RUNNING -> READY
    oIncidentLocal.GetStateMachine()->StopEvent();

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oIncidentLocal.GetStateMachine()->GetState(), FS_IDLE);

    // Destroy: READY -> SHUTDOWN
    ASSERT_EQ(oIncidentLocal.Destroy(), ERR_NOERROR);

    // Check Results
    EXPECT_GT(oIncidentLocal.m_nReceiveCount, 0);
    EXPECT_GT(oIncidentLocal.m_nSendCount, 0);
    EXPECT_EQ(oIncidentLocal.m_nReceiveCount, oIncidentLocal.m_nSendCount);
}

// TestCase 3b: Senden und Empfangen von Globalen Incidents
TEST(LeakModule, SendAndReceiveIncidentsGlobal)
{
    class cIncidentGlobal : public cModule
    {
    public:
        cIncidentGlobal()
            : m_nSendCount(0)
        {
        }

    public: // override cStateEntryListener
        fep::Result CleanUp(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::CleanUp(eOldState);

            return nResult;
        }
        fep::Result ProcessStartupEntry(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::ProcessStartupEntry(eOldState);

            fillElementHeader(this);

            // Do not log
            nResult|= GetPropertyTree()->SetPropertyValue(
                fep::component_config::g_strIncidentConsoleLogPath_bEnable, false);

            m_oTimer.setCallback(&cIncidentGlobal::RunCyclic, *this);
            m_oTimer.setPeriod(100 * 1000);

            if (fep::isOk(nResult))
            {
                GetStateMachine()->StartupDoneEvent();
            }

            return nResult;
        }
        fep::Result ProcessIdleEntry(const fep::tState eOldState)
        {
            if (eOldState != fep::FS_STARTUP) m_oTimer.stop();
            return fep::ERR_NOERROR;
        }
        fep::Result ProcessInitializingEntry(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::ProcessInitializingEntry(eOldState);

            if (fep::isOk(nResult))
            {
                GetStateMachine()->InitDoneEvent();
            }

            return nResult;
        }
        fep::Result ProcessRunningEntry(const fep::tState eOldState)
        {
            m_oTimer.start();
            return fep::ERR_NOERROR;
        }

        void RunCyclic()
        {
            GetIncidentHandler()->InvokeIncident(10815, fep::SL_Critical_Global,
                            "Test Global Incident", NULL, 0, NULL);

            ++m_nSendCount;
        }

    public:
        size_t m_nSendCount;
        a_util::system::Timer m_oTimer;
   };

    class cIncidentLogger : public cModule, public fep::IIncidentStrategy
    {
    public:
        cIncidentLogger()
            : m_nReceiveCount(0)
        {
        }

    public: // override cStateEntryListener
        fep::Result CleanUp(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::CleanUp(eOldState);

            return nResult;
        }
        fep::Result ProcessStartupEntry(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::ProcessStartupEntry(eOldState);

            fillElementHeader(this);

            // # Enabling the incident handler for all purposes including but not limited to
            // # error handling, logging and post-processing.
            RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
                              fep::component_config::g_strIncidentHandlerPath_bEnable, true));

            // # Enable handling of remote incidents issued by other FEP Elements on the bus since
            // # this is the Master Element
            RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
                              fep::component_config::g_strIncidentHandlerPath_bEnableGlobalScope, true));

            // # Always print a log dump of incidents to stdout (can never hurt; disable this
            // # if running in a productive realtime context!)
            RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
                              fep::component_config::g_strIncidentConsoleLogPath_bEnable, true));

            // # Instead of specific incidents, all incidents are to be considered in the output
            RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
                              fep::component_config::g_strIncidentConsoleLogPath_bEnableCatchAll, true));

            // # Turning off file logging - it is out of the scope if this example
            RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
                              fep::component_config::g_strIncidentFileLogPath_bEnable, false));

            // # Turn off the history strategy for the same reason
            RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
                              fep::component_config::g_strIncidentHistoryLogPath_bEnable, false));

            // # Turning off the Notification Strategy. This is the Master Element and there
            // # is no point in enabling these...
            RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(
                              fep::component_config::g_strIncidentNotificationLogPath_bEnable, false));

            /// The base path for the "Master Strategy" configuration
            #define MASTER_STRAT_ROOT_CONFIG "MyMasterStrategy"
            /// The path to the option whether the "Master Strategy" is set to be fussy or not.
            #define MASTER_STRAT_PROP_BE_FUSSY MASTER_STRAT_ROOT_CONFIG".bBeFussyAboutEveryting"
            /// The path to the option of the "Master Strategy" which element is to be monitored more
            /// precisely.
            #define MASTER_STRAT_CRITICAL_ELEMENT MASTER_STRAT_ROOT_CONFIG".strCriticalModule"

            // # Associating our own incident strategy with all incidents, no matter which one.
            // # This basically provides the intention of this example
            RETURN_IF_FAILED(GetIncidentHandler()->AssociateCatchAllStrategy(
                              this, MASTER_STRAT_ROOT_CONFIG, SA_REPLACE));
            //GetIncidentHandler()->AssociateCatchAllStrategy(this, NULL, SA_APPEND);
            //GetIncidentHandler()->AssociateCatchAllStrategy(this, NULL, SA_REPLACE);

            // Configuring some base properties for this demo:
            // Fussy Mode means, that this Master Element will be "allergic" against
            // specific warnings either issued by the Master Element itself or issued
            // by the Elements under its control.
            RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(MASTER_STRAT_PROP_BE_FUSSY, true));
            RETURN_IF_FAILED(GetPropertyTree()->SetPropertyValue(MASTER_STRAT_CRITICAL_ELEMENT, ""));

            #undef MASTER_STRAT_ROOT_CONFIG
            #undef MASTER_STRAT_PROP_BE_FUSSY
            #undef MASTER_STRAT_CRITICAL_ELEMENT


            if (fep::isOk(nResult))
            {
                GetStateMachine()->StartupDoneEvent();
            }

            return nResult;
        }
        fep::Result ProcessInitializingEntry(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::ProcessInitializingEntry(eOldState);

            if (fep::isOk(nResult))
            {
                GetStateMachine()->InitDoneEvent();
            }

            return nResult;
        }

    public: // IIncidentStrategy interface
        fep::Result HandleLocalIncident(fep::IModule *pElementContext, const int16_t nIncident,
                                    const fep::tSeverityLevel eSeverity,
                                    const char *strOrigin,
                                    int nLine,
                                    const char *strFile,
                                    const timestamp_t tmSimTime,
                                    const char *strDescription)
        {
            // should not happen
            return ERR_UNEXPECTED;
        }

        fep::Result HandleGlobalIncident(const char *strSource, const int16_t nIncident,
                                     const fep::tSeverityLevel eSeverity,
                                     const timestamp_t tmSimTime,
                                     const char *strDescription)
        {
            fep::Result nResult = ERR_NOERROR;

            //std::cerr << "Received Global Incident:"
            //          << " strSource=" << (strSource ? strSource : "<NULL>")
            //          << " strDescription=" << (strDescription ? strDescription : "<NULL>")
            //          << std::endl;
            ++m_nReceiveCount;

            return nResult;
        }

        fep::Result RefreshConfiguration(const fep::IProperty *pStrategyProperty,
                                     const fep::IProperty *pAffectedProperty)
        {
            // Not interested in this ... but interface needs it
            return ERR_NOERROR;
        }

    private:
        handle_t m_hReceiveHandle;
    public:
        size_t m_nReceiveCount;
    };

    // Create: <> -> STARTUP -> IDLE
    cIncidentLogger oIncidentLogger;
    ASSERT_EQ(oIncidentLogger.Create(cModuleOptions("IncidentLogger")), ERR_NOERROR);
    cIncidentGlobal oIncidentGlobal;
    ASSERT_EQ(oIncidentGlobal.Create(cModuleOptions("IncidentGlobal")), ERR_NOERROR);

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oIncidentLogger.GetStateMachine()->GetState(), FS_IDLE);
    ASSERT_EQ(oIncidentGlobal.GetStateMachine()->GetState(), FS_IDLE);

    // Initialize: IDLE -> READY
    oIncidentLogger.GetStateMachine()->InitializeEvent();
    oIncidentGlobal.GetStateMachine()->InitializeEvent();

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oIncidentLogger.GetStateMachine()->GetState(), FS_READY);
    ASSERT_EQ(oIncidentGlobal.GetStateMachine()->GetState(), FS_READY);

    // Start: READY -> RUNNING
    oIncidentLogger.GetStateMachine()->StartEvent();
    oIncidentGlobal.GetStateMachine()->StartEvent();

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oIncidentLogger.GetStateMachine()->GetState(), FS_RUNNING);
    ASSERT_EQ(oIncidentGlobal.GetStateMachine()->GetState(), FS_RUNNING);

    // Stop: RUNNING -> READY
    oIncidentLogger.GetStateMachine()->StopEvent();
    oIncidentGlobal.GetStateMachine()->StopEvent();

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oIncidentLogger.GetStateMachine()->GetState(), FS_IDLE);
    ASSERT_EQ(oIncidentGlobal.GetStateMachine()->GetState(), FS_IDLE);

    // Destroy: READY -> SHUTDOWN
    ASSERT_EQ(oIncidentLogger.Destroy(), ERR_NOERROR);
    ASSERT_EQ(oIncidentGlobal.Destroy(), ERR_NOERROR);

    // Check Results
    EXPECT_GT(oIncidentLogger.m_nReceiveCount, 0);
    EXPECT_GT(oIncidentGlobal.m_nSendCount, 0);
    EXPECT_EQ(oIncidentLogger.m_nReceiveCount, oIncidentGlobal.m_nSendCount);
}

// TestCase 4: Schreiben und Lesen von Properties
TEST(LeakModule, SetAndReadProperties)
{
    class cPropertyServer : public cModule
    {
    public:
        cPropertyServer()
            : m_nPropertyAChangeCount(0)
            , m_nRemoteNumberValue(0)
        {
        }

    public: // override cStateEntryListener
        fep::Result CleanUp(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::CleanUp(eOldState);

            // Read final property value
            {
                IPropertyTree* pPropertyTree= GetPropertyTree();
                nResult|= pPropertyTree->GetPropertyValue("bTree.RemoteNumber", m_nRemoteNumberValue);
            }

            return nResult;
        }
        fep::Result ProcessStartupEntry(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::ProcessStartupEntry(eOldState);

            fillElementHeader(this);

            m_oTimer.setCallback(&cPropertyServer::RunCyclic, *this);
            m_oTimer.setPeriod(100 * 1000);


            if (fep::isOk(nResult))
            {
                GetStateMachine()->StartupDoneEvent();
            }

            return nResult;
        }
        fep::Result ProcessIdleEntry(const fep::tState eOldState)
        {
            if (eOldState != fep::FS_STARTUP) m_oTimer.stop();
            return fep::ERR_NOERROR;
        }
        fep::Result ProcessInitializingEntry(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::ProcessInitializingEntry(eOldState);

            // Set some properties
            {
                IPropertyTree* pPropertyTree= GetPropertyTree();

                nResult|= pPropertyTree->SetPropertyValue("aTree.IncreasingNumber", 0);
                nResult|= pPropertyTree->SetPropertyValue("bTree.RemoteNumber", 0);
            }

            if (fep::isOk(nResult))
            {
                GetStateMachine()->InitDoneEvent();
            }

            return nResult;
        }
        fep::Result ProcessRunningEntry(const fep::tState eOldState)
        {
            m_oTimer.start();
            return fep::ERR_NOERROR;
        }

        void RunCyclic()
        {
            fep::Result nResult = ERR_NOERROR;

            // Set some properties
            {
                IPropertyTree* pPropertyTree= GetPropertyTree();

                {
                    int32_t nValue;
                    nResult|= pPropertyTree->GetPropertyValue("aTree.IncreasingNumber", nValue);
                    ++nValue;
                    nResult|= pPropertyTree->SetPropertyValue("aTree.IncreasingNumber", nValue);

                    ++m_nPropertyAChangeCount;
                }
            }
        }

    public:
        size_t m_nPropertyAChangeCount;
        int32_t m_nRemoteNumberValue;
        a_util::system::Timer m_oTimer;
    };

    class cPropertyClient : public cModule, public cStateExitListener, public IPropertyListener
    {
    public:
        cPropertyClient()
            : m_nPropertyAChangeCount(0)
            , m_nRemoteNumberValue(0)
            , m_bRemoteNumberValueFailure(false)
        {
        }

    public: // override cStateEntryListener
        fep::Result CleanUp(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::CleanUp(eOldState);

            nResult|= GetStateMachine()->UnregisterStateExitListener(this);

            return nResult;
        }
        fep::Result ProcessStartupEntry(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::ProcessStartupEntry(eOldState);

            fillElementHeader(this);

            nResult|= GetStateMachine()->RegisterStateExitListener(this);

            m_oTimer.setCallback(&cPropertyClient::RunCyclic, *this);
            m_oTimer.setPeriod(100 * 1000);

            if (fep::isOk(nResult))
            {
                GetStateMachine()->StartupDoneEvent();
            }

            return nResult;
        }
        fep::Result ProcessIdleEntry(const fep::tState eOldState)
        {
            if (eOldState != fep::FS_STARTUP) m_oTimer.stop();
            return fep::ERR_NOERROR;
        }
        fep::Result ProcessInitializingEntry(const fep::tState eOldState)
        {
            fep::Result nResult = cModule::ProcessInitializingEntry(eOldState);

            if (fep::isOk(nResult))
            {
                GetStateMachine()->InitDoneEvent();
            }

            return nResult;
        }

        fep::Result ProcessRunningEntry(const fep::tState eOldState)
        {
            fep::Result nResult= ERR_NOERROR;

            {
                IPropertyTree * pPropertyTree = GetPropertyTree();
                nResult|= pPropertyTree->MirrorRemoteProperty("PropertyServer", "aTree", "aLocal", 5000);

                if (fep::isOk(nResult))
                {
                    pPropertyTree->RegisterListener("aLocal", this);
                }
            }
            m_oTimer.start();

            return ERR_NOERROR;
        }

        fep::Result ProcessRunningExit(const fep::tState eOldState)
        {
            fep::Result nResult= ERR_NOERROR;

            {
                IPropertyTree * pPropertyTree = GetPropertyTree();

                if (fep::isOk(nResult))
                {
                    pPropertyTree->UnregisterListener("aLocal", this);
                }

                nResult|= pPropertyTree->UnmirrorRemoteProperty("PropertyServer", "aTree", "aLocal", 5000);
            }

            return ERR_NOERROR;
        }

    public: // override IPropertyListener
        fep::Result ProcessPropertyAdd(IProperty const * poProperty, IProperty const * poAffectedProperty, char const * strRelativePath)
        {
            return ERR_NOERROR;
        }

        fep::Result ProcessPropertyChange(IProperty const * poProperty, IProperty const * poAffectedProperty, char const * strRelativePath)
        {
            ++m_nPropertyAChangeCount;
            return ERR_NOERROR;
        }

        fep::Result ProcessPropertyDelete(IProperty const * poProperty, IProperty const * poAffectedProperty, char const * strRelativePath)
        {
            return ERR_NOERROR;
        }

        void RunCyclic()
        {
            fep::Result nResult = ERR_NOERROR;

            // Get some properties
            {
                IPropertyTree* pPropertyTree= GetPropertyTree();

                {
                    IProperty* pProperty;
                    nResult|= pPropertyTree->GetRemoteProperty("PropertyServer", "bTree.RemoteNumber", &pProperty, 5000);

                    if (fep::isFailed(nResult))
                    {
                          m_bRemoteNumberValueFailure= true;
                    }
                    else
                    {
                        int32_t nValue;
                        nResult|= pProperty->GetValue(nValue);
                        ++nValue;
                        nResult|= pProperty->SetValue(nValue);

                        if (m_nRemoteNumberValue +1 != nValue)
                        {
                            m_bRemoteNumberValueFailure= true;
                        }

                        nResult|= pPropertyTree->SetRemotePropertyValue("PropertyServer", "bTree.RemoteNumber", nValue);

                        m_nRemoteNumberValue= nValue;

                        delete pProperty;
                    }
                }
            }
        }

    public:
        size_t m_nPropertyAChangeCount;
        int32_t m_nRemoteNumberValue;
        bool m_bRemoteNumberValueFailure;
        a_util::system::Timer m_oTimer;
    };

    // Create: <> -> STARTUP -> IDLE
    cPropertyServer oPropertyServer;
    ASSERT_EQ(oPropertyServer.Create(cModuleOptions("PropertyServer")), ERR_NOERROR);
    cPropertyClient oPropertyClient;
    ASSERT_EQ(oPropertyClient.Create(cModuleOptions("PropertyClient")), ERR_NOERROR);

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oPropertyServer.GetStateMachine()->GetState(), FS_IDLE);
    ASSERT_EQ(oPropertyClient.GetStateMachine()->GetState(), FS_IDLE);

    // Initialize: IDLE -> READY
    oPropertyServer.GetStateMachine()->InitializeEvent();
    oPropertyClient.GetStateMachine()->InitializeEvent();

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oPropertyServer.GetStateMachine()->GetState(), FS_READY);
    ASSERT_EQ(oPropertyClient.GetStateMachine()->GetState(), FS_READY);

    // Start: READY -> RUNNING
    oPropertyServer.GetStateMachine()->StartEvent();
    oPropertyClient.GetStateMachine()->StartEvent();

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oPropertyServer.GetStateMachine()->GetState(), FS_RUNNING);
    ASSERT_EQ(oPropertyClient.GetStateMachine()->GetState(), FS_RUNNING);

    // Stop: RUNNING -> READY
    oPropertyServer.GetStateMachine()->StopEvent();
    oPropertyClient.GetStateMachine()->StopEvent();

    a_util::system::sleepMilliseconds(1 * 1000);
    ASSERT_EQ(oPropertyServer.GetStateMachine()->GetState(), FS_IDLE);
    ASSERT_EQ(oPropertyClient.GetStateMachine()->GetState(), FS_IDLE);

    // Destroy: READY -> SHUTDOWN
    ASSERT_EQ(oPropertyServer.Destroy(), ERR_NOERROR);
    ASSERT_EQ(oPropertyClient.Destroy(), ERR_NOERROR);

    // Check Results: Property A
    EXPECT_GT(oPropertyServer.m_nPropertyAChangeCount, 0);
    EXPECT_GT(oPropertyClient.m_nPropertyAChangeCount, 0);
    EXPECT_NEAR(oPropertyServer.m_nPropertyAChangeCount, oPropertyClient.m_nPropertyAChangeCount, 3);

    // Check Results: Property B
    EXPECT_FALSE(oPropertyClient.m_bRemoteNumberValueFailure);
    EXPECT_GT(oPropertyServer.m_nRemoteNumberValue, 0);
    EXPECT_GT(oPropertyClient.m_nRemoteNumberValue, 0);
    EXPECT_NEAR(oPropertyServer.m_nRemoteNumberValue, oPropertyClient.m_nRemoteNumberValue, 3);
}
