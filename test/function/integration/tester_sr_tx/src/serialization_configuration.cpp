/**
 * Implementation of the tester for the integration of FEP Property Tree with FEP Transmission Limiter.
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
* Test Case:   TestSerializationConfiguration
* Test ID:     1.15
* Test Title:  Test of the serialization configuration of the FEP signal registry
* Description: This test tests the coumpound of signal registry and dds transmission adapter regarding
*              the configuration of the serialization mechanism.
* Strategy:   Create a FEP Element, register a signal, change the CCA property for serialization
*             of the signal registry to disable serialization, register another signal after reconfiguration signal then start
*             the FEP element and try to start communication between the serialized and unserialized signal.
*             This should result in a signal registry incident with warning severity and a transmission adapter incident with critical
*             local severity. Communication between the serialized and unserialized signal should fail.
*              
* Passed If:   End of test is reached
*              
* Ticket:      #33303
* Requirement: FEPSDK-1607 
*/

#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include <fep_test_common.h>

#include "transmission_adapter/fep_data_sample_factory.h"
#include <ddl.h>

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

fep::Result CalculateSignalSizeFromDescription(const char * strSignalType,
    const char * strDescription, size_t & szSignalSize)
{
    ddl::CodecFactory oFactory(strSignalType, strDescription);
    RETURN_IF_FAILED(oFactory.isValid().getErrorCode());
    szSignalSize = oFactory.getStaticBufferSize();
    return ERR_NOERROR;
}


class cStrategyListener : public fep::IIncidentStrategy
{
public:
    cStrategyListener() : m_bSRIncidentReceived(false)
    {

    }

    ~cStrategyListener() {}

public: // IIncidentStrategy
    fep::Result HandleLocalIncident(fep::IModule *pElementContext, const int16_t nIncident,
                                const fep::tSeverityLevel eSeverity, const char *strOrigin,
                                int nLine, const char *strFile, const timestamp_t tmSimTime, const char *strDescription)
    {
        if (nIncident == fep::FSI_SERIALIZATION_CHANGE_WITH_REGISTERED_SIGNALS)
        {
            m_bSRIncidentReceived = true;
        }
        else if(nIncident == fep::FSI_TRANSM_FEP_PROTO_WRONG_SER_MODE)
        {
            m_bSerIncidentReceived = true;
        }

        return ERR_NOERROR;
    }

    fep::Result HandleGlobalIncident(const char *strSource, const int16_t nIncident,
                                 const fep::tSeverityLevel eSeverity,
                                 const timestamp_t tmSimTime, const char *strDescription)
    {
        //nothing to do
        return ERR_NOERROR;
    }

    fep::Result RefreshConfiguration(const fep::IProperty *pStrategyProperty, const fep::IProperty *pAffectedProperty)
    {
        //nothing to do
        return ERR_NOERROR;
    }

public:
    bool m_bSRIncidentReceived;
    bool m_bSerIncidentReceived;
};


class cSampleCounter : public fep::IUserDataListener
{
public:
    cSampleCounter()
        : RcvdSamplesCnt(0)
    {

    }
    ~cSampleCounter()
    {

    }

    fep::Result Update(const fep::IUserDataSample *poSample)
    {
        RcvdSamplesCnt++;
        return ERR_NOERROR;
    }
    uint16_t RcvdSamplesCnt;
};

/**
 * @req_id "FEPSDK-1607"
 */
TEST(cTesterSignalRegistryTransmissionAdapter, TestSerializationConfiguration)
{
    cTestBaseModule oModule;
    ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions("test_module")));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->StartupDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(fep::FS_IDLE));

    cStrategyListener oIncidentListener;
    cSampleCounter oSampleCounter;

    size_t szSample = 0;
    ASSERT_EQ(a_util::result::SUCCESS, CalculateSignalSizeFromDescription("tSignal", s_strDescription.c_str(), szSample));

    handle_t SerHandle;
    handle_t UnserHandle;

    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetIncidentHandler()->AssociateCatchAllStrategy(&oIncidentListener, "blabla"));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignalDescription(s_strDescription.c_str()));
    cUserSignalOptions oSignalOut("SerOutputSignal", fep::SD_Output, "tSignal");
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oSignalOut, SerHandle));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->SetPropertyValue(fep::component_config::g_strSignalRegistryPath_bSerialization, false));
    a_util::system::sleepMilliseconds(1000);
    ASSERT_TRUE(oIncidentListener.m_bSRIncidentReceived);
    cUserSignalOptions oSignalIn("SerOutputSignal", fep::SD_Input, "tSignal");
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(oSignalIn, UnserHandle));

    fep::IUserDataSample* pSample;
    ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSample));
    ASSERT_EQ(a_util::result::SUCCESS, pSample->SetSize(sizeof(double)));
    ASSERT_EQ(a_util::result::SUCCESS, pSample->SetSignalHandle(SerHandle));

    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetUserDataAccess()->RegisterDataListener(&oSampleCounter, UnserHandle));

    a_util::system::sleepMilliseconds(1);
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->InitializeEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(fep::FS_INITIALIZING));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->InitDoneEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(fep::FS_READY));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->StartEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(fep::FS_RUNNING));

    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetUserDataAccess()->TransmitData(pSample, true));
    a_util::system::sleepMilliseconds(1000);

    ASSERT_TRUE(oIncidentListener.m_bSerIncidentReceived);
    ASSERT_EQ(oSampleCounter.RcvdSamplesCnt, 0);

    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->StopEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(fep::FS_IDLE));
    ASSERT_EQ(a_util::result::SUCCESS, oModule.GetStateMachine()->ShutdownEvent());
    ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(fep::FS_SHUTDOWN));

    delete pSample;
}
