/**

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
 */
#ifndef _DRIVER_TESTER_HELPERS_H_
#define _DRIVER_TESTER_HELPERS_H_
//TestData

static const std::string s_strDescriptionTemplate = std::string(
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
    "        %s"
    "    </structs>"
    "    <streams>"
    "    </streams>"
    "</adtf:ddl>");

static const std::string s_strSignalDescription = std::string(
    "<struct alignment=\"1\" name=\"tTestSignal\" version=\"2\">"
    "    <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"ui32Signal1\" type=\"tUInt32\" />"
    "    <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"4\" name=\"ui32Signal2\" type=\"tUInt32\" />"
    "</struct>");

#pragma pack(push, 1)
typedef struct
{
    uint32_t ui32Signal1;
    uint32_t ui32Signal2;
} tData;
#pragma pack(pop)

class cDataListener : public IPreparationDataListener
{
public:
    cDataListener()
    { }
    fep::Result Update(IPreparationDataSample const * poPreparationSample)
    {
        m_vecRxSampleSizes.push_back(
            poPreparationSample->GetSize());
        m_vecReceivedData.push_back(
            *(reinterpret_cast<tData*>(poPreparationSample->GetPtr())));
        m_vecReceivedHandles.push_back(poPreparationSample->GetSignalHandle());
        return ERR_NOERROR;
    }
    static bool CompareData(const tData &sfirst, const tData &sSecond)
    {
        return sfirst.ui32Signal1 == sSecond.ui32Signal1 &&
            sfirst.ui32Signal2 == sfirst.ui32Signal2;
    }
    handle_t GetSignalHandle()
    {
        return static_cast<handle_t>(NULL);
    }
    std::list<fep::IUserDataListener *> GetUserDataListener()
    {
        return m_listListeners;
    }

    std::list<handle_t> m_vecReceivedHandles;
    std::list<tData> m_vecReceivedData;
    std::list<size_t> m_vecRxSampleSizes;
    std::list<fep::IUserDataListener *> m_listListeners;
};

//TestRXSampleSizeMismatch

class cSampleCounter : public IPreparationDataListener
{
public:
    cSampleCounter()
        : RcvdSamplesCnt(0)
    {

    }
    ~cSampleCounter()
    {

    }

    fep::Result Update(const IPreparationDataSample *poSample)
    {
        RcvdSamplesCnt++;
        return ERR_NOERROR;
    }

    handle_t GetSignalHandle()
    {
        return static_cast<handle_t>(NULL);
    }

    std::list<fep::IUserDataListener *> GetUserDataListener()
    {
        return m_listListeners;
    }

    uint16_t RcvdSamplesCnt;
    std::list<fep::IUserDataListener *> m_listListeners;
};

class cRXIncidentListener : public fep::IIncidentStrategy
{
public:
    cRXIncidentListener() : m_bRxIncidentReceived(false)
    {

    }

    ~cRXIncidentListener() {}

public: // IIncidentStrategy
    fep::Result HandleLocalIncident(fep::IModule *pElementContext, const int16_t nIncident,
        const fep::tSeverityLevel eSeverity, const char *strOrigin,
        int nLine, const char *strFile, const timestamp_t tmSimTime,
        const char *strDescription)
    {
        if (nIncident == fep::FSI_DRIVER_ISSUE || nIncident == fep::FSI_TRANSM_RX_WRONG_SAMPLE_SIZE)
        {
            m_bRxIncidentReceived = true;
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
    bool m_bRxIncidentReceived;
};


//TestMessageAfterCreate
class cCommandListenerReceiver : public cCommandListener
{
public:
    a_util::concurrency::semaphore oEvt;

    fep::Result Update(ICustomCommand const * poCommand)
    {
        oEvt.notify();
        return ERR_NOERROR;
    }
};

//TestVariableSignalSize

class cVarSignalSizeSender : public cTestBaseModule
{
public:
    fep::Result Terminate()
    {
        GetStateMachine()->StopEvent();
        a_util::system::sleepMilliseconds(1000);
        GetStateMachine()->ShutdownEvent();
        return ERR_NOERROR;
    }
};

class cVarSignalSizeReceiver : public cTestBaseModule, public IUserDataListener
{
public:

    static const size_t szDataA = 1000 * 2;
    static const size_t szDataB = 1000 * 32;
    static const size_t szDataC = 1000 * 128;
    static const size_t szDataD = 1024;// * 1024 *4;
    static const size_t szDataE = 1000 * 3;
    cVarSignalSizeReceiver()
    {
        m_aDataAReceived = new int8_t[szDataA];
        m_aDataBReceived = new int8_t[szDataB];
        m_aDataCReceived = new int8_t[szDataC];
        m_aDataDReceived = new int8_t[szDataD];
        m_aDataEReceived = new int8_t[szDataE];
    }

    ~cVarSignalSizeReceiver()
    {
        delete[] m_aDataAReceived;
        delete[] m_aDataBReceived;
        delete[] m_aDataCReceived;
        delete[] m_aDataDReceived;
        delete[] m_aDataEReceived;
    }

    fep::Result Update(const IUserDataSample* poSample)
    {
        switch (poSample->GetSize())
        {
        case szDataA:
            poSample->CopyTo(&(m_aDataAReceived[0]), szDataA);
            break;
        case szDataB:
            poSample->CopyTo(&(m_aDataBReceived[0]), szDataB);
            break;
        case szDataC:
            poSample->CopyTo(&(m_aDataCReceived[0]), szDataC);
            break;
        case szDataD:
            poSample->CopyTo(&(m_aDataDReceived[0]), szDataD);
            break;
        case szDataE:
            poSample->CopyTo(&(m_aDataEReceived[0]), szDataE);
            break;
        default:
            m_bFailed = true;
        }
        return ERR_NOERROR;
    }

    fep::Result Terminate()
    {
        GetStateMachine()->StopEvent();
        a_util::system::sleepMilliseconds(1000);
        GetStateMachine()->ShutdownEvent();
        return ERR_NOERROR;
    }

    void SetReceiveSample(IUserDataSample* poReceiveSample)
    {
        m_poReceiveSample = poReceiveSample;
    }
public:
    IUserDataSample* m_poReceiveSample;
    int8_t* m_aDataAReceived;
    int8_t* m_aDataBReceived;
    int8_t* m_aDataCReceived;
    int8_t* m_aDataDReceived;
    int8_t* m_aDataEReceived;
    bool m_bFailed;
};
#endif