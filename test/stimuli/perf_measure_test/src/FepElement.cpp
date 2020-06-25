/**
 * Implementation of the Class FepElement.
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

#define NOMINMAX

#include "stdafx.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>

#include "a_util/concurrency.h"
#include "a_util/system.h"
#include "a_util/logging.h"

#include "perfmeasure/perfmeasure.h"

static std::map<FepElementMode, std::string> CreateBaseElementNames()
{
	std::map<FepElementMode, std::string> oResult;
    oResult[ReceiverMode] = "FepPing_Receiver_";
    oResult[SenderMode] = "FepPing_Sender_";
    oResult[ClientMode] = "FepPing_Client_";
    oResult[ServerMode] = "FepPing_Server_";
	return oResult;
};
static std::map<FepElementMode, std::string> s_strBaseElementNames = CreateBaseElementNames();

static std::map<std::string, std::string> CreateElementHeaders()
{
	std::map<std::string, std::string> oResult;
	oResult["Description"] = "FEP Monitor (Experimental)";
	oResult["Context"] = "Experimental";
	oResult["Vendor"] = "AEV";
	oResult["DisplayName"] = "FEP Monitor";
	return oResult;
};
static std::map<std::string, std::string> s_strElementHeaders = CreateElementHeaders();

static const char* s_strMeasureFileBeginning = "results";
static const char* s_strMeasureFileEnding = "_framework_measure.csv";
static const char* s_strDefaultSignalName = "Ping";
static const char* s_strDefaultResponseSignalName = "Pong";
static const char* s_strDefaultSignalType = "Ping";

static const size_t s_szFrequency = 10;
static const size_t s_szTransportSize = 64;
static const timestamp_t s_tmStartupTimeoutMilliseconds = 20000;
static const timestamp_t s_tmStartupCheckMilliseconds = 100;
static const timestamp_t s_tmSendDelayInMicroSeconds = 100 * 1000;
// Constant defining timeout for refresh via method cModule::GetAvailableElements().
//static timestamp_t const s_tmRefreshTimeoutMs = 2500;
static const timestamp_t s_tmRefreshTimeoutMs = -1; // Fixing Bug #31525, No timeout is required any more
static const uint32_t s_nMaxRuntime = 500;
static const uint16_t s_nDefaultDomainID = 65;


namespace impl
{
    struct FepSignalConfig
    {
        std::string m_strSignalType;
        size_t m_szDdbMaxDepth;
        timestamp_t m_szSendDelayInMicroSeconds;
        size_t m_szFrequency;
        size_t m_szNumberOfPacketsPerCycle;
        size_t m_szTransportSize;
        std::string m_strSignalDDL;
        //size_t m_szNumberOfExpectedPackets;

    public:
        FepSignalConfig()
            : m_strSignalType()
            , m_szDdbMaxDepth(0)
            , m_szSendDelayInMicroSeconds(s_tmSendDelayInMicroSeconds)
            , m_szFrequency(s_szFrequency)
            , m_szNumberOfPacketsPerCycle(1)
            , m_szTransportSize(s_szTransportSize)
            , m_strSignalDDL()
        { }
    };

    struct FepElementConfig
    {
        FepElementMode m_eMode;
        fep::tTransmissionType m_eTransmissionType;
        std::string m_strElementName;
        std::string m_strSignalDDL;
        std::string m_strMeasureFile;
        size_t m_nVerbosity;
        double m_fTransmissionLimiterFrequency; // Always off
        size_t m_nTransmissionLimiterBufferSize; // Always off
        bool m_statisticsMode;
        std::vector<FepSignalConfig*> m_vSignals;
        uint16_t m_nCurrentSignalToConfig;
        uint32_t m_nRuntimeLength;
        bool m_bDisableSerialization;
        uint32_t m_nClientId;
        uint32_t m_nServerId;

    public:
        FepElementConfig()
            : m_eMode(ReceiverMode)
            , m_eTransmissionType(fep::TT_RTI_DDS)
            , m_strElementName()
            , m_strSignalDDL()
            , m_strMeasureFile(s_strMeasureFileBeginning)
            , m_nVerbosity(1)
            , m_fTransmissionLimiterFrequency(0.0) // Disabled
            , m_nTransmissionLimiterBufferSize(100) // The Default
            , m_statisticsMode(false)
            , m_vSignals()
            , m_nCurrentSignalToConfig(0)
            , m_nRuntimeLength(0)
            , m_bDisableSerialization(false)
        { }
    };
    struct FepElementPrivate
    {
        std::ostream* m_pOutputStream;
        a_util::concurrency::recursive_mutex  m_oOutputStreamCS;
        fep::Result m_nSystemResult;
        std::string m_strSystemErrorMsg;
        std::vector<handle_t> m_vecInputSignals;
        std::vector<handle_t> m_vecDDBs;
        std::vector<handle_t> m_vecOutputSignals;
        size_t m_szNumberOfPacketUnsync;
        uint64_t m_nRuntimeCount;
    public:
        FepElementPrivate() 
            : m_pOutputStream(NULL)
            , m_oOutputStreamCS()
            , m_nSystemResult(0)
            , m_strSystemErrorMsg("")
            , m_vecInputSignals()
            , m_vecDDBs()
            , m_vecOutputSignals()
            , m_szNumberOfPacketUnsync(0)
            , m_nRuntimeCount(0)
        { }
    };

} // namespace impl

#define LOCK_OUTPUT_STREAM()  a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> __lock(m_pElementPrivate->m_oOutputStreamCS)

#define INITIAL_ERROR(cmd) \
    if (fep::isFailed(nResult)) { \
        std::ostringstream os; \
        m_pElementPrivate->m_nSystemResult = nResult; \
        os << "!! " << "Failed to '" << cmd << "' (Result code: " << nResult.getErrorCode() << ")"; \
        m_pElementPrivate->m_strSystemErrorMsg= os.str(); \
        {\
            LOCK_OUTPUT_STREAM(); \
            *(m_pElementPrivate->m_pOutputStream) << os.str(); \
        }\
        return nResult; \
    }

#define CHECK_ERROR(cmd) \
    if (fep::isFailed(nResult)) { \
        std::ostringstream os; \
        m_pElementPrivate->m_nSystemResult = nResult; \
        os << "!! " << "Failed to '" << cmd << "' (Result code: " << nResult.getErrorCode() << ")"; \
        m_pElementPrivate->m_strSystemErrorMsg= os.str(); \
        {\
            LOCK_OUTPUT_STREAM(); \
            *(m_pElementPrivate->m_pOutputStream) << os.str(); \
        }\
        GetStateMachine()->ErrorEvent(); \
        return nResult; \
    }

static const char* output_prefixes[10] =
{
    "",
    "-- ",
    "-- ",
    "## ",
    "## ",
    "?? ",
    "?? ",
    "?? ",
    "?? ",
    "?? ",
};

#define VERBOSE(LEVEL,ARG) \
    if (m_pElementConfig->m_nVerbosity >= LEVEL) { \
        LOCK_OUTPUT_STREAM(); \
        *(m_pElementPrivate->m_pOutputStream) << output_prefixes[LEVEL] << ARG << std::endl; \
    }


static std::string createSignalDDL(const std::string& signal_type, const size_t& nSize)
{
    const size_t nSizeOfPingPacket= sizeof(t_Ping)-1;
    std::ostringstream os;
    os <<
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
        "        <datatype description=\"predefined ADTF bool datatype\" max=\"true\" min=\"false\" name=\"tBool\" size=\"8\" />"
        "        <datatype description=\"predefined ADTF char datatype\" max=\"127\" min=\"-128\" name=\"tChar\" size=\"8\" />"
        "        <datatype description=\"predefined ADTF uint8_t datatype\" max=\"255\" min=\"0\" name=\"tUInt8\" size=\"8\" />"
        "        <datatype description=\"predefined ADTF int8_t datatype\" max=\"127\" min=\"-128\" name=\"tInt8\" size=\"8\" />"
        "        <datatype description=\"predefined ADTF uint16_t datatype\" max=\"65535\" min=\"0\" name=\"tUInt16\" size=\"16\" />"
        "        <datatype description=\"predefined ADTF int16_t datatype\" max=\"32767\" min=\"-32768\" name=\"tInt16\" size=\"16\" />"
        "        <datatype description=\"predefined ADTF uint32_t datatype\" max=\"4294967295\" min=\"0\" name=\"tUInt32\" size=\"32\" />"
        "        <datatype description=\"predefined ADTF int32_t datatype\" max=\"2147483647\" min=\"-2147483648\" name=\"tInt32\" size=\"32\" />"
        "        <datatype description=\"predefined ADTF uint64_t datatype\" max=\"18446744073709551615\" min=\"0\" name=\"tUInt64\" size=\"64\" />"
        "        <datatype description=\"predefined ADTF int64_t datatype\" max=\"9223372036854775807\" min=\"-9223372036854775808\" name=\"tInt64\" size=\"64\" />"
        "        <datatype description=\"predefined ADTF float datatype\" max=\"3.402823e+38\" min=\"-3.402823e+38\" name=\"tFloat32\" size=\"32\" />"
        "        <datatype description=\"predefined ADTF double datatype\" max=\"1.797693e+308\" min=\"-1.797693e+308\" name=\"tFloat64\" size=\"64\" />"
                "    </datatypes>"
                "    <enums>"
                "    </enums>"
                "<structs>"
                "    <struct alignment=\"1\" name=\"" + signal_type + "\" version=\"1\">"
        "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"nMagic\" type=\"tUInt32\" />"
        "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"4\" name=\"nSeqNr\" type=\"tUInt32\" />"
        "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"8\" name=\"nSize\" type=\"tUInt32\" />"
        "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"12\" name=\"bSyncFlag\" type=\"tBool\" />"
        "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"13\" name=\"bDummy1\" type=\"tBool\" />"
        "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"14\" name=\"bDummy2\" type=\"tBool\" />"
        "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"15\" name=\"bDummy3\" type=\"tBool\" />"
        "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"16\" name=\"nClientId\" type=\"tUInt32\" />"
        "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"20\" name=\"nServerId\" type=\"tUInt32\" />"
        "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"24\" name=\"tm01ClientSend\" type=\"tUInt64\" />"
        "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"32\" name=\"tm02ServerRecv\" type=\"tUInt64\" />"
        "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"40\" name=\"tm03ServerSend\" type=\"tUInt64\" />"
        "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"48\" name=\"tm04ClientRecv\" type=\"tUInt64\" />"
        "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"56\" name=\"tm01ClientSendFinish\" type=\"tUInt64\" />";
    int nPreCalcSize = (static_cast<int>(nSize)*8 + 63 - nSizeOfPingPacket) / 64;

    if (nPreCalcSize > 0)
    {
        os << "        <element alignment=\"1\" arraysize=\"" << nPreCalcSize << "\" byteorder=\"LE\" bytepos=\"48\" name=\"data\" type=\"tUInt64\" />";
    }
    os <<
                "    </struct>"
                "</structs>"
                "<streams>"
                "</streams>"
                "</adtf:ddl>";
    return os.str();
}


FepElement::FepElement() :
    m_vSenders()
{
    m_pElementPrivate= new ::impl::FepElementPrivate();
    m_pElementConfig= new ::impl::FepElementConfig();
}

FepElement::~FepElement()
{
    VERBOSE(9, "Destructing (fep_ping) FepElement.");
    std::cerr << "X: Destructing (fep_ping) FepElement." << std::endl;

    for(std::list<FepCyclicSender*>::iterator it = m_vSenders.begin(); it != m_vSenders.end(); ++it)
    {
        FepCyclicSender* pCyclicSender = (*it);
        delete pCyclicSender;
    }
    m_vSenders.clear();

    for(std::list<FepCyclicDDBSender*>::iterator it = m_vDDBSenders.begin(); it != m_vDDBSenders.end(); ++it)
    {
        FepCyclicDDBSender* pCyclicSender = (*it);
        delete pCyclicSender;
    }
    m_vDDBSenders.clear();

    for(std::vector<handle_t>::iterator it = m_pElementPrivate->m_vecInputSignals.begin(); it != m_pElementPrivate->m_vecInputSignals.end(); ++it)
    {
        handle_t UnregHandle = (*it);
        GetSignalRegistry()->UnregisterSignal(UnregHandle);
    }

    for(std::vector<handle_t>::iterator it = m_pElementPrivate->m_vecOutputSignals.begin(); it != m_pElementPrivate->m_vecOutputSignals.end(); ++it)
    {
        handle_t UnregHandle = (*it);
        GetSignalRegistry()->UnregisterSignal(UnregHandle);
    }

    for(std::vector<handle_t>::iterator it = m_pElementPrivate->m_vecDDBs.begin(); it != m_pElementPrivate->m_vecDDBs.end(); ++it)
    {
        handle_t UnregHandle = (*it);
        cModule::DestroyDDBEntry(UnregHandle);
    }
    
    for(std::vector< ::impl::FepSignalConfig*>::iterator it = m_pElementConfig->m_vSignals.begin();  it != m_pElementConfig->m_vSignals.end(); ++it)
    {
        ::impl::FepSignalConfig* pSignalConfig = (*it);
        delete pSignalConfig;
    }

    delete m_pElementPrivate;
    delete m_pElementConfig;
}

FepElementMode FepElement::GetMode() const
{
    return m_pElementConfig->m_eMode;
}

void FepElement::SetMode(FepElementMode eMode)
{
    m_pElementConfig->m_eMode = eMode;
}

void FepElement::SetSignalType(const std::string& strSignalType)
{
    m_pElementConfig->m_vSignals[m_pElementConfig->m_nCurrentSignalToConfig]->m_strSignalType = strSignalType;
}

void FepElement::SetSignalDDL(const std::string& strSignalDDL)
{
    m_pElementConfig->m_strSignalDDL = strSignalDDL;
}

void FepElement::SetMeasureFile(const std::string& strMeasureFile)
{
    m_pElementConfig->m_strMeasureFile = strMeasureFile;
}

void FepElement::SetDDBMaxDepth(const size_t& szDDBMaxDepth)
{
    m_pElementConfig->m_vSignals[m_pElementConfig->m_nCurrentSignalToConfig]->m_szDdbMaxDepth = szDDBMaxDepth;
}

void FepElement::SetFrequency(const size_t& szFrequency)
{
    m_pElementConfig->m_vSignals[m_pElementConfig->m_nCurrentSignalToConfig]->m_szFrequency = szFrequency;
}

void FepElement::SetSendDelayInMicroSeconds(const timestamp_t& szSendDelayInMicroSeconds)
{
    m_pElementConfig->m_vSignals[m_pElementConfig->m_nCurrentSignalToConfig]->m_szSendDelayInMicroSeconds = szSendDelayInMicroSeconds;
}

void FepElement::SetOutputStream(std::ostream* pOutputStream)
{
    m_pElementPrivate->m_pOutputStream = pOutputStream;
}

void FepElement::SetVerbosity(const size_t& nVerbosity)
{
    m_pElementConfig->m_nVerbosity = nVerbosity;
}

void FepElement::SetNumberOfPacketsPerCycle(const size_t& szNumberOfPacketsPerCycle)
{
    m_pElementConfig->m_vSignals[m_pElementConfig->m_nCurrentSignalToConfig]->m_szNumberOfPacketsPerCycle = szNumberOfPacketsPerCycle;
}

void FepElement::SetTransportSize(const size_t& nSize)
{
    m_pElementConfig->m_vSignals[m_pElementConfig->m_nCurrentSignalToConfig]->m_szTransportSize= nSize;
}

uint32_t FepElement::GetRuntimeLength() const
{
    return m_pElementConfig->m_nRuntimeLength;
}

void FepElement::SetRuntimeLength(const uint32_t& nSeconds)
{
    m_pElementConfig->m_nRuntimeLength = nSeconds;
}

void FepElement::SetStatisticsMode(const bool& bStatisticsMode)
{
    m_pElementConfig->m_statisticsMode = bStatisticsMode;
}

void FepElement::SetCurrentSignalToConfig(const uint16_t& nCurrentSignalToConfigure)
{
    m_pElementConfig->m_nCurrentSignalToConfig = nCurrentSignalToConfigure;
}

void FepElement::ResizeSignalConfig(const size_t& nNewSize)
{
	size_t nOldSize = m_pElementConfig->m_vSignals.size();
	if (nNewSize > nOldSize)
	{
		m_pElementConfig->m_vSignals.resize(nNewSize);
		for(size_t i = nOldSize; i < nNewSize; ++i)
		{
			m_pElementConfig->m_vSignals[i] = new ::impl::FepSignalConfig();
		}
	}
}

uint16_t FepElement::GetSignalConfigSize()
{
    return static_cast<uint16_t>(m_pElementConfig->m_vSignals.size());
}

void FepElement::SetDisableSerialization(bool bDisableSerialization)
{
    m_pElementConfig->m_bDisableSerialization= bDisableSerialization;
}

void FepElement::SetClientId(const uint32_t& nClientId)
{
    m_pElementConfig->m_nClientId = nClientId;
}

void FepElement::SetServerId(const uint32_t& nServerId)
{
    m_pElementConfig->m_nServerId = nServerId;
}

fep::Result FepElement::Start(const fep::cModuleOptions& oModuleOptions)
{
    fep::Result nResult = ERR_NOERROR;

    switch (m_pElementConfig->m_eMode)
    {
        case ClientMode:
            if (m_pElementConfig->m_nRuntimeLength > s_nMaxRuntime)
            {
                nResult |= ERR_INVALID_ARG;
                INITIAL_ERROR("Runtime length exceeds max value (= 60 sec).");
            }
            if (m_pElementConfig->m_nRuntimeLength == 0)
            {
                nResult |= ERR_INVALID_ARG;
                INITIAL_ERROR("Runtime length needed. Please use '-ti'.");
            }
            break;
        default:
            break;
    }

    CHECK_ERROR("FEP Element Failed to start.");

    // Check if name is set ... if not set a default
    if (m_pElementConfig->m_strElementName.empty())
    {
        m_pElementConfig->m_strElementName = s_strBaseElementNames[m_pElementConfig->m_eMode];
        srand((unsigned int) time(NULL));

        std::ostringstream oUnique;
        oUnique << "_" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (rand() & 0xFFFF);

        m_pElementConfig->m_strElementName.append(oUnique.str());
    }

    for(std::vector< ::impl::FepSignalConfig*>::iterator iter = m_pElementConfig->m_vSignals.begin();  iter != m_pElementConfig->m_vSignals.end(); ++iter)
    {
        ::impl::FepSignalConfig* pSignalConfig = *iter;

        if (pSignalConfig->m_strSignalType.empty())
        {
            pSignalConfig->m_strSignalType = s_strDefaultSignalType;
        }

        if (m_pElementConfig->m_strSignalDDL.empty())
        {
            pSignalConfig->m_strSignalDDL = createSignalDDL(pSignalConfig->m_strSignalType, pSignalConfig->m_szTransportSize);
        }
        else
        {
            pSignalConfig->m_strSignalDDL = m_pElementConfig->m_strSignalDDL;
        }
    }

    fep::cModuleOptions oOverrideModuleOptions(oModuleOptions);
    oOverrideModuleOptions.SetParticipantName(m_pElementConfig->m_strElementName.c_str());
    m_pElementConfig->m_eTransmissionType= oOverrideModuleOptions.GetTransmissionType();
    nResult = fep::cModule::Create(oOverrideModuleOptions);
    CHECK_ERROR("Create Element");
    VERBOSE(5, "Fep Element created: name='" << m_pElementConfig->m_strElementName 
		<< "' with Transmission type " << (m_pElementConfig->m_eTransmissionType == fep::TT_RTI_DDS ? "DDS" : "???") << "");

    for (int nStartupMilliseconds = 0; nStartupMilliseconds < s_tmStartupTimeoutMilliseconds; nStartupMilliseconds += s_tmStartupCheckMilliseconds)
    {
        switch (GetStateMachine()->GetState())
        {
            case fep::FS_STARTUP:
            case fep::FS_IDLE:
            case fep::FS_INITIALIZING:
            case fep::FS_READY:
                break;
            case fep::FS_RUNNING:
                return ERR_NOERROR;
            case fep::FS_ERROR:
            case fep::FS_SHUTDOWN:
            case fep::FS_UNKNOWN:
                return ERR_UNEXPECTED;
        }
        a_util::system::sleepMilliseconds(nStartupMilliseconds);
    }
    VERBOSE(5, "Fep Element ready: state='" << fep::cState::ToString(GetStateMachine()->GetState()) << "'");

    if (GetStateMachine()->GetState() != fep::FS_RUNNING)
    {
        VERBOSE(5, "Fep Element did not reach running state: state='" << fep::cState::ToString(GetStateMachine()->GetState()) << "'");
        nResult = ERR_UNEXPECTED;
        CHECK_ERROR("Fep Element took too long to reach running state");
    }

    return nResult;
}

fep::Result FepElement::Stop()
{
    fep::Result nResult = GetStateMachine()->StopEvent();
    CHECK_ERROR("Send Stop Event");

    return nResult;
}

fep::Result FepElement::Shutdown()
{
    fep::Result nResult = GetStateMachine()->ShutdownEvent();
    CHECK_ERROR("Send Shutdown Event");

    nResult |= WaitForShutdown(s_tmRefreshTimeoutMs);
    CHECK_ERROR("Wait For Shutdown");
    VERBOSE(5, "Fep Element ready: state='" << fep::cState::ToString(GetStateMachine()->GetState()) << "'");


    return nResult;
}

fep::Result FepElement::ErrorCode()
{
    return m_pElementPrivate->m_nSystemResult;
}

const std::string& FepElement::ErrorMessage()
{
    return m_pElementPrivate->m_strSystemErrorMsg;
}

fep::Result FepElement::CleanUp(const fep::tState eOldState)
{
    fep::Result nResult= cModule::CleanUp(eOldState);
    
    MEASURE_STOP();
    MEASURE_SAVE((m_pElementConfig->m_strMeasureFile+s_strMeasureFileEnding).c_str());

    return nResult;
}

fep::Result FepElement::ProcessStartupEntry(const fep::tState eOldState)
{
    fep::Result nResult = cModule::ProcessStartupEntry(eOldState);
    CHECK_ERROR("Process Startup Entry");

    nResult |= GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fElementVersion, 1.0);
    nResult |= GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementName, GetName());
    nResult |= GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementDescription, s_strElementHeaders["Description"].c_str());
    nResult |= GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fFEPVersion,
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10);
    nResult |= GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementPlatform,
        FEP_SDK_PARTICIPANT_PLATFORM_STR);
    nResult |= GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementContext, s_strElementHeaders["Context"].c_str());
    nResult |= GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_fElementContextVersion,
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10);
    nResult |= GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementVendor, s_strElementHeaders["Vendor"].c_str());
    nResult |= GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementDisplayName, s_strElementHeaders["DisplayName"].c_str());
    nResult |= GetPropertyTree()->SetPropertyValue(fep::g_strElementHeaderPath_strElementCompilationDate,  __DATE__);
    CHECK_ERROR("Set Common Properties");

    // No console output for incidents
    nResult |= GetPropertyTree()->SetPropertyValue(fep::component_config::g_strIncidentConsoleLogPath_bEnable, false);
    CHECK_ERROR("Set Standalone MoDode");

    // Enable stand-alone mode, so no other FEP Element can (accidently) control this FEP Element
    nResult |= GetPropertyTree()->SetPropertyValue(FEP_STM_STANDALONE_PATH , true);
    CHECK_ERROR("Set Standalone Mode");

    // New: Disable serialization
#if 0
    // FIXME: Enable again sometime, if this feature is available on the API again
    nResult |= GetPropertyTree()->SetPropertyValue(fep::component_config::g_strLimiterPath_bDisableDdlSerialization, m_pElementConfig->m_bDisableSerialization);
    CHECK_ERROR("Set Disable serialization");
#endif

    nResult |= GetStateMachine()->StartupDoneEvent();
    CHECK_ERROR("Send Startup Done Event");

    return nResult;
}

fep::Result FepElement::ProcessIdleEntry(const fep::tState eOldState)
{
    fep::Result nResult = ERR_NOERROR;
    if(fep::FS_RUNNING != eOldState)
    {
        nResult = cModule::ProcessIdleEntry(eOldState);
        CHECK_ERROR("Process Idle Entry");

        nResult|= GetStateMachine()->InitializeEvent();
        CHECK_ERROR("Send Initialize Event");
    }
    else
    {
        nResult |= GetStateMachine()->ShutdownEvent();
        CHECK_ERROR("Send Shutdown Event");
    }

    return nResult;
}

fep::Result FepElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    fep::Result nResult = cModule::ProcessInitializingEntry(eOldState);
    CHECK_ERROR("Process Initializing Entry");

    uint32_t nIndex = 1;
    for(std::vector< ::impl::FepSignalConfig*>::iterator it = m_pElementConfig->m_vSignals.begin(); it != m_pElementConfig->m_vSignals.end(); ++it, ++nIndex)
    {
        ::impl::FepSignalConfig* pSignalConfig = (*it);

        std::string isignal_name;
        std::string osignal_name;
        std::string strIndex = a_util::strings::toString((uint32_t)nIndex);
        std::string signal_type;
        std::string signal_ddl;

        switch (m_pElementConfig->m_eMode)
        {
            case ReceiverMode:
            case ServerMode:
                isignal_name = s_strDefaultSignalName;
                isignal_name += strIndex.c_str();
                osignal_name = s_strDefaultResponseSignalName;
                osignal_name += strIndex.c_str();
                signal_type = pSignalConfig->m_strSignalType;
                signal_ddl = pSignalConfig->m_strSignalDDL;
                break;
            case ClientMode:
                isignal_name = s_strDefaultResponseSignalName;
                isignal_name += strIndex.c_str();
                osignal_name = s_strDefaultSignalName;
                osignal_name += strIndex.c_str();
                signal_type = pSignalConfig->m_strSignalType;
                signal_ddl = pSignalConfig->m_strSignalDDL;
                break;
            default:
                break;
        }

        // Register input signal if required
        handle_t RecvHandle = NULL;
        fep::IDDBAccess* poDDBAccess= NULL;
        switch (m_pElementConfig->m_eMode)
        {
            case SenderMode:
                break;
            case ReceiverMode:
            case ServerMode:
            case ClientMode:
                nResult |= GetSignalRegistry()->RegisterSignalDescription(signal_ddl.c_str());
                if (pSignalConfig->m_szDdbMaxDepth == 0)
                {
                    nResult |= GetSignalRegistry()->RegisterSignal(cUserSignalOptions(isignal_name.c_str(),
                                                fep::SD_Input, signal_type.c_str()), RecvHandle);
                    CHECK_ERROR("Register Input Signal");
                    m_pElementPrivate->m_vecInputSignals.push_back(RecvHandle);

                }
                else
                {
                    nResult |= cModule::InitDDBEntry(isignal_name.c_str(),
                        signal_type.c_str(),
                        pSignalConfig->m_szDdbMaxDepth /*szMaxDepth*/,
                        fep::DDBDS_DeliverIncomplete,
                        RecvHandle, &poDDBAccess);
                    m_pElementPrivate->m_vecDDBs.push_back(RecvHandle);
                    CHECK_ERROR("Init DDB Entry");
                }
                break;
        }

        // Register output signals if required
        handle_t SendHandle = NULL;
        switch (m_pElementConfig->m_eMode)
        {
            case ReceiverMode:
                break;
            case SenderMode:
            case ClientMode:
            case ServerMode:
                nResult |= GetSignalRegistry()->RegisterSignalDescription(signal_ddl.c_str());
                nResult |= GetSignalRegistry()->RegisterSignal(cUserSignalOptions(osignal_name.c_str(),
                                                fep::SD_Output, signal_type.c_str()), SendHandle);
                m_pElementPrivate->m_vecOutputSignals.push_back(SendHandle);
                CHECK_ERROR("Register Output Signal");
                break;
        }

        std::cout << "Configuring element with following parameters:" << std::endl;
        std::cout << "Mode: " << m_pElementConfig->m_eMode << "; Frequency: " << pSignalConfig->m_szFrequency
            << "; Runtime-Length: " << m_pElementConfig->m_nRuntimeLength << "; Number of Packets per Cylce: "
            << pSignalConfig->m_szNumberOfPacketsPerCycle << "; Packet-Size: " << pSignalConfig->m_szTransportSize << "[Byte]; "
            //<< "Expected number of Packets for this Signal: " << pSignalConfig->m_szNumberOfExpectedPackets
            << std::endl;
            
            
        // Create Cyclic Sender
        if (pSignalConfig->m_szDdbMaxDepth == 0)
        {
            FepCyclicSender* pCyclicSender= NULL;
            //uint64_t nPackets = m_pElementConfig->m_nRuntimeLength * pSignalConfig->m_szFrequency;
            //std::cout << "number of packets: " << nPackets << std::endl;
            //std::cout << "SendDelay: " << pSignalConfig->m_szSendDelayInMicroSeconds;
            // Creating a sender/receiver for the signal
            pCyclicSender = new FepCyclicSender(this,
                                                    m_pElementConfig->m_nClientId,
                                                    m_pElementConfig->m_nServerId,
                                                    m_pElementConfig->m_eMode,
                                                    pSignalConfig->m_szSendDelayInMicroSeconds,
                                                    pSignalConfig->m_szNumberOfPacketsPerCycle,
                                                    static_cast<uint32_t>(m_pElementConfig->m_nRuntimeLength * pSignalConfig->m_szFrequency * pSignalConfig->m_szNumberOfPacketsPerCycle),
                                                    RecvHandle, SendHandle);
            m_vSenders.push_back(pCyclicSender);
            CHECK_ERROR("Register Data Listener");
        }
        else
        {
            assert(poDDBAccess);

            FepCyclicDDBSender* pCyclicSender= NULL;
            // Creating a sender/receive for the signal
            pCyclicSender = new FepCyclicDDBSender(this,
                                            m_pElementConfig->m_nClientId,
                                            m_pElementConfig->m_nServerId,
                                            m_pElementConfig->m_eMode,
                                            static_cast<uint16_t>(pSignalConfig->m_szSendDelayInMicroSeconds),
                                            pSignalConfig->m_szNumberOfPacketsPerCycle,
                                            static_cast<uint32_t>(m_pElementConfig->m_nRuntimeLength * pSignalConfig->m_szFrequency * pSignalConfig->m_szNumberOfPacketsPerCycle),
                                            RecvHandle, SendHandle, poDDBAccess);
            assert(pCyclicSender);
            m_vDDBSenders.push_back(pCyclicSender);
            CHECK_ERROR("Register Sync Listener");
        }
	}
    std::cout << "Done registering signals!\n";
    MEASURE_START(20*11*1000);
    GetStateMachine()->InitDoneEvent();
    CHECK_ERROR("Send Init Done Event")

    return nResult;
}

fep::Result FepElement::ProcessReadyEntry(const fep::tState eOldState)
{
    fep::Result nResult = cModule::ProcessReadyEntry(eOldState);
    CHECK_ERROR("Process Ready Entry");

    GetStateMachine()->StartEvent();
    CHECK_ERROR("Send Start Event")

    return nResult;
}

fep::Result FepElement::ProcessRunningEntry(const fep::tState eOldState)
{
    fep::Result nResult = cModule::ProcessRunningEntry(eOldState);
    CHECK_ERROR("Process Running Entry");

    return nResult;
}

fep::Result FepElement::PrintStatistics()
{
    PrintStatisticsDefault();
    PrintStatisticsDDB();
   
    return ERR_NOERROR;
}

fep::Result FepElement::PrintStatisticsDefault()
{
    std::string signal_placeholder = ".Signal";

    std::string strSaveFileName = m_pElementConfig->m_strMeasureFile;
    strSaveFileName += ".csv";

    if (!m_vSenders.empty())
    {
        std::ofstream saveFile(strSaveFileName.c_str(), std::ios::out);
        if (!saveFile.is_open())
        {
            std::cerr << "Error: " << "Failed top open output file: '" << m_pElementConfig->m_strMeasureFile << "'" << std::endl;
            return ERR_OPEN_FAILED;
        }

        saveFile << "SignalName" << ";" << "SeqNr" << ";" << "ClientId" << ";" << "ServerId" << ";" << "ClientSend" ";" << "ClientSend-Finish" << ";" << "ServerRecv" << ";" << "ServerSend" << ";" << "ClientRecv" << ";" << std::endl;

        uint32_t nIndex = 1;
        for (std::list<FepCyclicSender*>::iterator it = m_vSenders.begin(); it != m_vSenders.end(); ++it, ++nIndex)
        {
            FepCyclicSender* pCyclicSender = (*it);

            ::impl::FepElementStats* pStats = pCyclicSender->GetStatistics();


            {
                // Results CSV
                std::string strIndex = a_util::strings::toString((uint32_t)nIndex);
                strIndex += signal_placeholder;
                for (size_t i = 0; i < pStats->m_nSentPacketCount; ++i)
                {
                    for (size_t j = 0; j < pStats->m_poPerServerStats.size(); ++j)
                    {
                        saveFile << strIndex << ";" << i << ";";

                        impl::FepPerServerStats*& poPerServerStats = pStats->m_poPerServerStats[j];
                        saveFile << poPerServerStats->m_pPingPackets[i].nClientId << ";";
                        saveFile << poPerServerStats->m_pPingPackets[i].nServerId << ";";

                        timestamp_t tiSendTime = poPerServerStats->m_pPingPackets[i].tm01ClientSend;
                        timestamp_t tiSendTimeFinish = poPerServerStats->m_pPingPackets[i].tm01ClientSendFinish;
                        timestamp_t tiSSendTime = poPerServerStats->m_pPingPackets[i].tm03ServerSend;
                        timestamp_t tiSReceiveTime = poPerServerStats->m_pPingPackets[i].tm02ServerRecv;
                        timestamp_t tiReceiveTime = poPerServerStats->m_pPingPackets[i].tm04ClientRecv;

                        saveFile << tiSendTime << ";" << tiSendTimeFinish << ";" << tiSReceiveTime << ";" << tiSSendTime << ";" << tiReceiveTime << ";";
                        saveFile << std::endl;
                    }
                }
            }

            size_t total_nMissedCount = 0;
            timestamp_t total_tiRoundTripTimeMax = 0;
            timestamp_t total_tiRoundTripTimeMin = 999999;
            timestamp_t total_tiRoundTripTotalTime = 0;
            size_t total_nSentPacketCount = 0;
            size_t total_nReceivedPacketCount = 0;

            for (size_t j = 0; j < pStats->m_poPerServerStats.size(); ++j)
            {
                impl::FepPerServerStats*& poPerServerStats = pStats->m_poPerServerStats[j];

                size_t nMissedCount = 0;
                timestamp_t tiRoundTripTimeMax = 0;
                timestamp_t tiRoundTripTimeMin = 999999;
                timestamp_t tiRoundTripTotalTime = 0;

                {
                    for (size_t i = 0; i < pStats->m_nSentPacketCount; ++i)
                    {
                        timestamp_t tiSendTime = poPerServerStats->m_pPingPackets[i].tm01ClientSend;
                        //timestamp_t tiSReceiveTime = poPerServerStats->m_pPingPackets[i].tm02ServerRecv;
                        //timestamp_t tiSSendTime = poPerServerStats->m_pPingPackets[i].tm03ServerSend;
                        timestamp_t tiReceiveTime = poPerServerStats->m_pPingPackets[i].tm04ClientRecv;

                        if (0 != tiReceiveTime)
                        {
                            timestamp_t tiRoundTrip = tiReceiveTime - tiSendTime;
                            tiRoundTripTotalTime += tiRoundTrip;
                            if (0 == tiRoundTripTimeMax)
                            {
                                tiRoundTripTimeMax = tiRoundTrip;
                                tiRoundTripTimeMin = tiRoundTrip;
                            }
                            else
                            {
                                if (tiRoundTrip > tiRoundTripTimeMax)
                                {
                                    tiRoundTripTimeMax = tiRoundTrip;
                                }
                                else if (tiRoundTrip < tiRoundTripTimeMin)
                                {
                                    tiRoundTripTimeMin = tiRoundTrip;
                                }
                            }
                        }
                        else
                        {
                            nMissedCount++;
                        }
                    }
                }

                total_nMissedCount += nMissedCount;
                total_tiRoundTripTimeMax = std::max(total_tiRoundTripTimeMax, tiRoundTripTimeMax);
                total_tiRoundTripTimeMin = std::min(total_tiRoundTripTimeMin, tiRoundTripTimeMin);;
                total_tiRoundTripTotalTime += tiRoundTripTotalTime;
                total_nSentPacketCount += pStats->m_nSentPacketCount;
                total_nReceivedPacketCount += poPerServerStats->m_nReceivedPacketCount;

                if (!m_pElementConfig->m_statisticsMode)
                {
                    VERBOSE(0, "--- statistics ---");
                }

                switch (m_pElementConfig->m_eMode)
                {
                case SenderMode:
                    VERBOSE(0, pStats->m_nSentPacketCount << " packets sent");
                    break;
                case ReceiverMode:
                    for (size_t t = 0; t < pStats->m_poPerServerStats.size(); ++t)
                    {
                        VERBOSE(0, poPerServerStats->m_nReceivedPacketCount << " packets received from server " << t);
                    }
                    break;
                case ClientMode:
                    if (m_pElementConfig->m_statisticsMode)
                    {
                        std::cout
                            << "===PACKET_STAT==="
                            << ";" << j
                            << ";" << pStats->m_nSentPacketCount
                            << ";" << poPerServerStats->m_nReceivedPacketCount
                            << ";" << nMissedCount
                            << std::endl;
                        std::cout
                            << "===RTT_STAT==="
                            << ";" << j
                            << ";" << tiRoundTripTimeMin
                            << ";" << (poPerServerStats->m_nReceivedPacketCount > 0 ? tiRoundTripTotalTime / poPerServerStats->m_nReceivedPacketCount : 0)
                            << ";" << tiRoundTripTimeMax
                            << std::endl;

                    }
                    else
                    {
                       VERBOSE(0, pStats->m_nSentPacketCount << " packets transmitted, "
                            << poPerServerStats->m_nReceivedPacketCount << " received, "
                            << nMissedCount << " packets lost, ");
                        VERBOSE(0, std::fixed << "rtt min/avg/max = "
                            << tiRoundTripTimeMin << "/"
                            << (poPerServerStats->m_nReceivedPacketCount > 0 ? tiRoundTripTotalTime / poPerServerStats->m_nReceivedPacketCount : 0) << "/"
                            << tiRoundTripTimeMax << " us");

                    }
                    break;
                case ServerMode:
                    VERBOSE(0, pStats->m_nSentPacketCount << " packets relayed");
                    break;
                }
            }


            switch (m_pElementConfig->m_eMode)
            {
            case SenderMode:
                break;
            case ReceiverMode:
                break;
            case ClientMode:
                if (m_pElementConfig->m_statisticsMode)
                {
                    std::cout
                        << "=== PACKET_STAT==="
                        << ";" << "TOTAL"
                        << ";" << total_nSentPacketCount
                        << ";" << total_nReceivedPacketCount
                        << ";" << total_nMissedCount
                        << std::endl;
                    std::cout
                        << "===RTT_STAT==="
                        << ";" << "TOTAL"
                        << ";" << total_tiRoundTripTimeMin
                        << ";" << (total_nReceivedPacketCount > 0 ? total_tiRoundTripTotalTime / total_nReceivedPacketCount : 0)
                        << ";" << total_tiRoundTripTimeMax
                        << std::endl;
                    std::cout  
                        << "===RTT_TRANS==="
                        << ";" << pStats->m_nTransmitTimeMin
                        << ";" << (pStats->m_nSentPacketCount > 0 ? pStats->m_nTransmitTimeTotal / pStats->m_nSentPacketCount : 0)
                        << ";" << pStats->m_nTransmitTimeMax
                        << std::endl;
                }
                else
                {
                    VERBOSE(0, "Total: " << total_nSentPacketCount << " packets transmitted, "
                        << total_nReceivedPacketCount << " received, "
                        << total_nMissedCount << " packets lost, ");
                    VERBOSE(0, std::fixed << "rtt min/avg/max = "
                        << total_tiRoundTripTimeMin << "/"
                        << (total_nReceivedPacketCount > 0 ? total_tiRoundTripTotalTime / total_nReceivedPacketCount : 0) << "/"
                        << total_tiRoundTripTimeMax << " us");
                    VERBOSE(0, std::fixed << "transmission min/avg/max = "
                        << pStats->m_nTransmitTimeMin << "/"
                        << (pStats->m_nSentPacketCount > 0 ? pStats->m_nTransmitTimeTotal / pStats->m_nSentPacketCount : 0) << "/"
                        << pStats->m_nTransmitTimeMax << " us");

                }
                break;
            case ServerMode:
                VERBOSE(0, pStats->m_nSentPacketCount << " packets relayed");
                break;
            }

        }

        saveFile.close();
    }
    return ERR_NOERROR;
}

fep::Result FepElement::PrintStatisticsDDB()
{
    std::string signal_placeholder = ".Signal";

    std::string strSaveFileName = m_pElementConfig->m_strMeasureFile;
    //strSaveFileName += "DDB.csv";
    strSaveFileName += "_ddb.csv";

    if(!m_vDDBSenders.empty())
    {
        std::ofstream saveFile(strSaveFileName.c_str(), std::ios::out);
        if (!saveFile.is_open())
        {
            std::cerr << "Error: " << "Failed top open output file: '" << m_pElementConfig->m_strMeasureFile << "'" << std::endl;
            return ERR_OPEN_FAILED;
        }

        saveFile << "SignalName" << ";" << "FrameNr" << ";" << "ClientSend" << ";" << "ClientRecv" << ";" << "IsComplete" << ";" << "NumOfValidSamples" << ";" << std::endl;

        uint32_t nIndex = 1;
        for(std::list<FepCyclicDDBSender*>::iterator it = m_vDDBSenders.begin(); it != m_vDDBSenders.end(); ++it, ++nIndex)
        {
            FepCyclicDDBSender* pCyclicSender = (*it);

            ::impl::FepElementDDBStats* pStats = pCyclicSender->GetStatistics();

            size_t nMissedCount = 0;
            timestamp_t tiRoundTripTimeMax = 0;
            timestamp_t tiRoundTripTimeMin = 0;
            timestamp_t tiRoundTripTotalTime = 0;

            {
                std::string strIndex = a_util::strings::toString((uint32_t)nIndex);
                strIndex += signal_placeholder;
                for (size_t i= 0; i< pStats->m_nSentFrameCount; ++i)
                {
                    saveFile << strIndex << ";" << i << ";";
                    timestamp_t tiSendTime= pStats->m_pFrames[i].tm01ClientSend;
                    timestamp_t tiReceiveTime= pStats->m_pFrames[i].tm02ClientRecv;
                    bool bIsComplete = pStats->m_pFrames[i].bIsComplete;
                    uint32_t nValidSamples = pStats->m_pFrames[i].nValidSampleCount;

                    saveFile << tiSendTime << ";" << tiReceiveTime << ";" << bIsComplete << ";" << nValidSamples;
                    saveFile << std::endl;
                    if(0 != tiReceiveTime)
                    {
                        timestamp_t tiRoundTrip = tiReceiveTime - tiSendTime;
                        tiRoundTripTotalTime += tiRoundTrip;
                        if(0 == tiRoundTripTimeMax)
                        {
                            tiRoundTripTimeMax = tiRoundTrip;
                            tiRoundTripTimeMin = tiRoundTrip;
                        }
                        else
                        {
                            if(tiRoundTrip > tiRoundTripTimeMax)
                            {
                                tiRoundTripTimeMax = tiRoundTrip;
                            }
                            else if(tiRoundTrip < tiRoundTripTimeMin)
                            {
                                tiRoundTripTimeMin = tiRoundTrip;
                            }
                        }
                    }
                    else
                    {
                        nMissedCount++;
                    }
                }
            }

            if (!m_pElementConfig->m_statisticsMode)
            {
                VERBOSE(0, "--- statistics ---");
            }

            switch (m_pElementConfig->m_eMode)
            {
                case SenderMode:
                    VERBOSE(0, pStats->m_nSentFrameCount << " frames sent");
                    break;
                case ReceiverMode:
                    VERBOSE(0, pStats->m_nReceivedFrameCount << " frames received");
                    break;
                case ClientMode:
                    if (m_pElementConfig->m_statisticsMode)
                    {
                        VERBOSE(0, "===PACKET_STAT==="
                                << ";" << pStats->m_nSentFrameCount
                                << ";" << pStats->m_nReceivedFrameCount
                                << ";" << nMissedCount );
                        VERBOSE(0, "===RTT_STAT==="
                                << ";" << tiRoundTripTimeMin
                                << ";" << (pStats->m_nReceivedFrameCount > 0 ? tiRoundTripTotalTime / pStats->m_nReceivedFrameCount : 0)
                                << ";" << tiRoundTripTimeMax );
                    }
                    else
                    {
                        VERBOSE(0, pStats->m_nSentFrameCount << " frames transmitted, "
                                << pStats->m_nReceivedFrameCount << " received, "
                                << nMissedCount << " frames lost, ");
                        VERBOSE(0, std::fixed << "rtt min/avg/max = "
                                << tiRoundTripTimeMin << "/"
                                << (pStats->m_nReceivedFrameCount > 0 ? tiRoundTripTotalTime / pStats->m_nReceivedFrameCount : 0) << "/"
                                << tiRoundTripTimeMax << " us");
                    }
                    break;
                case ServerMode:
                    VERBOSE(0, pStats->m_nSentFrameCount << " frames relayed");
                    break;
            }
        }
        saveFile.close();
    }

    return ERR_NOERROR;
}
