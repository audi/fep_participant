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
#ifndef _ADAPTER_TESTER_HELPERS_H_
#define _ADAPTER_TESTER_HELPERS_H_

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include "fep_test_common.h"

#include "transmission_adapter/fep_transmission.h"
#include "transmission_adapter/fep_receiver.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include "transmission_adapter/fep_serialization_helpers.h"

//Mocks
#include "fep_mock_tx_driver.h"
#include "fep_mock_incident_invoaktion_handler.h"
#include "fep_mock_property_tree.h"

//Messages
#include "messages/fep_command_control.h"
#include "messages/fep_command_custom.h"
#include "messages/fep_command_delete_property.h"
#include "messages/fep_command_get_property.h"
#include "messages/fep_command_get_signal_info.h"
#include "messages/fep_command_mapping_configuration.h"
#include "messages/fep_command_name_change.h"
#include "messages/fep_command_reg_prop_listener.h"
#include "messages/fep_command_resolve_signal_type.h"
#include "messages/fep_command_set_property.h"
#include "messages/fep_command_signal_description.h"
#include "messages/fep_command_unreg_prop_listener.h"
#include "messages/fep_command_get_schedule.h"

#include "messages/fep_message.h"
#include "messages/fep_notification_incident.h"
#include "messages/fep_notification_name_changed.h"
#include "messages/fep_notification_prop_changed.h"
#include "messages/fep_notification_property.h"
#include "messages/fep_notification_reg_prop_listener_ack.h"
#include "messages/fep_notification_resultcode.h"
#include "messages/fep_notification_signal_description.h"
#include "messages/fep_notification_signal_info.h"
#include "messages/fep_notification_state.h"
#include "messages/fep_notification_unreg_prop_listener_ack.h"
#include "messages/fep_notification_schedule.h"

#include "signal_registry/fep_signal_struct.h"

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

class cHeaderListener : public IPreparationDataListener
{
public:
    cHeaderListener()
        : m_lSyncFlags(),
        m_lFrameNr(),
        m_lSampleNr()
    {

    }
    ~cHeaderListener()
    {
        m_lSyncFlags.clear();
        m_lFrameNr.clear();
        m_lSampleNr.clear();
    }
public:
    fep::Result Update(const IPreparationDataSample *poPreparationSample)
    {
        m_lSyncFlags.push_back(poPreparationSample->GetSyncFlag());
        m_lFrameNr.push_back(poPreparationSample->GetFrameId());
        m_lSampleNr.push_back(poPreparationSample->GetSampleNumberInFrame());
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

public:
    std::vector<bool>m_lSyncFlags;
    std::vector<uint64_t>m_lFrameNr;
    std::vector<uint16_t>m_lSampleNr;
    std::list<fep::IUserDataListener *> m_listListeners;
};

class cBlockingNotificationListener : public cNotificationListener
{
public:
    a_util::concurrency::semaphore m_oSemBlock;
    a_util::concurrency::semaphore m_oFinished;
    uint32_t m_nReceiveCounter;
    uint32_t m_nExpectedReceiveCounter;
    bool m_bBlock;

public:
    cBlockingNotificationListener(uint32_t nExpected) :
        m_oSemBlock(),
        m_nReceiveCounter(0),
        m_bBlock(true),
        m_nExpectedReceiveCounter(nExpected)
    {
    }

    fep::Result Update(IResultCodeNotification const * pResultNotification)
    {
        if (m_bBlock)
        {
            m_oSemBlock.wait();
            m_bBlock = false;
        }
        ++m_nReceiveCounter;
        if (m_nExpectedReceiveCounter == m_nReceiveCounter)
        {
            m_oFinished.notify();
        }
        return ERR_NOERROR;
    }
};

class cSender
{
public:
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
    cTransmissionAdapter* m_pAdapter;
    const void* m_pMessage;
    size_t m_szSize;
    a_util::concurrency::semaphore m_SemFinished;
    uint32_t m_nSamples;


public:
    cSender(cTransmissionAdapter* pAdapter, const void* pMessage, size_t szSize, uint32_t nSamples) :
        m_pAdapter(pAdapter),
        m_pMessage(pMessage),
        m_szSize(szSize),
        m_nSamples(nSamples),
        m_SemFinished()
    {
    }

    void ThreadFunc()
    {
        for (uint32_t nCounter = 0; nCounter < m_nSamples; ++nCounter)
        {
            m_pAdapter->ReceiveMessage(m_pAdapter, m_pMessage, m_szSize);
        }
        m_SemFinished.notify();
    }

    void Start()
    {
        m_pThread.reset(new a_util::concurrency::thread(&cSender::ThreadFunc, this));
    }

    void Stop()
    {
        if (m_pThread)
        {
            m_pThread->join();
            m_pThread.reset();
        }
    }
};

class cCommandCreator
{
public:
    fep::cCustomCommand createCommand(uint64_t nSize)
    {
        std::string strCustomCmd;
        for (uint64_t i = 0; i < nSize; ++i)
        {
            char strRandSymbol = 70;
            std::string strPlaceholder;
            strPlaceholder.push_back(strRandSymbol);
            strCustomCmd += strPlaceholder;
        }
        std::string strJSONStr;
        strJSONStr.clear();
        strJSONStr.append("{ \"Parameter\" : \"");
        strJSONStr.append(strCustomCmd);
        strJSONStr.append("\" }");
        fep::cCustomCommand oCmd(
            "Custom",                  // const char *  	strCommand,
            strJSONStr.c_str(),  //{ \"Parameter\" : \"Test\" }",                 // const char *  	strParameterTree,
            "Sender",    // const char *  	strSender,
            "Receiver", // const char *  	strReceiver,
            0,                           // timestamp_t  	tmTimeStamp
            0);
        return oCmd;
    }
};

class cMessageSendingThread
{
public:
    cMessageSendingThread(cTransmissionAdapter* pTransmissionAdapter, cCustomCommand oCommand, uint16_t nMsgs)
        : m_pTransmissionAdapter(pTransmissionAdapter)
        , m_Command(oCommand)
        , m_nMessageCount(0)
        , m_nMessagesToSend(nMsgs)
        , m_oShutdown()
    {
        m_pThread.reset(new a_util::concurrency::thread(&cMessageSendingThread::ThreadFunc, this));
    }

    ~cMessageSendingThread()
    {
        m_oShutdown.notify();
        m_pThread->join();
    }
private:
    void ThreadFunc()
    {
        while (!m_oShutdown.is_set())
        {
            if (m_nMessageCount < m_nMessagesToSend)
            {
                m_pTransmissionAdapter->TransmitCommand(&m_Command);
                m_pTransmissionAdapter->ReceiveMessage(m_pTransmissionAdapter, m_Command.ToString(),
                    a_util::strings::getLength(m_Command.ToString()) + 1);
            }
            m_nMessageCount++;
        }
    }

    cTransmissionAdapter* m_pTransmissionAdapter;
    cCustomCommand m_Command;
    uint16_t m_nMessageCount;
    uint16_t m_nMessagesToSend;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
    a_util::concurrency::semaphore m_oShutdown;
};

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

class cMultiThreadTestDataTransmitThread
{
public:
    cMultiThreadTestDataTransmitThread(cTransmissionAdapter* pTransmissionAdapter, handle_t SendHandle, cDataReceiver* pReceiver, cMockTxDriver *pDriver)
        : m_pTransmissionAdapter(pTransmissionAdapter)
        , m_pReceiver(pReceiver)
        , m_pDriver(pDriver)
        , m_SendHandle(SendHandle)
        , m_nSampleCount(0)
        , m_oShutdown()
    {
        cDataSampleFactory::CreateSample(&m_pDataSample);
        m_pDataSample->SetSize(sizeof(tData));
        m_pDataSample->SetSignalHandle(SendHandle);
        m_pDataSample->SetSyncFlag(true);

        m_pThread.reset(new a_util::concurrency::thread(&cMultiThreadTestDataTransmitThread::ThreadFunc, this));

    }
    ~cMultiThreadTestDataTransmitThread()
    {
        m_oShutdown.notify();
        m_pThread->join();
        delete m_pDataSample;
    }
private:
    void ThreadFunc()
    {
        while (!m_oShutdown.is_set())
        {
            if (m_nSampleCount < 50)
            {
                m_pTransmissionAdapter->TransmitData(m_pDataSample);
                const void* pData = m_pDriver->m_vecTransmitters.at(1)->m_pData;
                size_t szSize = m_pDriver->m_vecTransmitters.at(1)->m_szSize;
                m_pReceiver->EnqueueReceivedData(m_pReceiver, pData, szSize);
                a_util::system::sleepMicroseconds(10);
            }
            m_nSampleCount++;
        }
    }
    cTransmissionAdapter* m_pTransmissionAdapter;
    cDataReceiver* m_pReceiver;
    cMockTxDriver *m_pDriver;
    handle_t m_SendHandle;
    IPreparationDataSample* m_pDataSample;
    uint32_t m_nSampleCount;
    a_util::concurrency::semaphore m_oShutdown;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
};

class cMessageCounter : public cCommandListener
{
public:
    cMessageCounter() : m_nMessageCounter(0)
    {

    }
    ~cMessageCounter()
    {

    }

public:
    fep::Result Update(const ICustomCommand *poCommand)
    {
        m_nMessageCounter++;
        return ERR_NOERROR;
    }

public:
    uint16_t m_nMessageCounter;
};

class cMockPropertyTreePrivate : public cMockPropertyTree
{
public:
    fep::Result GetPropertyValue(const char * strPropPath, int32_t & nValue) const
    {
        if (0 == a_util::strings::compare(strPropPath, fep::component_config::g_strTxAdapterPath_nNumberOfWorkerThreads))
        {
            nValue = m_nWorkerThreads;
            return fep::ERR_NOERROR;
        }
        else
        {
            return ERR_NOT_FOUND;
        }
    }

    fep::Result GetPropertyValue(const char * strPropPath,
        const char *& strValue) const
    {
        if (0 == a_util::strings::compare(strPropPath, fep::g_strElementHeaderPath_strElementName))
        {
            strValue = m_strModuleName.c_str();
            return fep::ERR_NOERROR;
        }
        else
        {
            return ERR_NOT_FOUND;
        }
    }
public:
    int32_t m_nWorkerThreads;
    std::string m_strModuleName;
};

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


class cGetCurrentSampleThread
{
public:
    cGetCurrentSampleThread(cTransmissionAdapter* pAdapter, handle_t hInput, IPreparationDataSample* pSample)
        : m_pAdapter(pAdapter)
        , m_hInput(hInput)
        , m_pSample(pSample)
    {

    }
    ~cGetCurrentSampleThread()
    {
    }
public:
    void Start()
    {
        m_pThread.reset(new a_util::concurrency::thread(&cGetCurrentSampleThread::ThreadFunc, this));
    }
    void Stop()
    {
        m_oShutdown.notify();
        m_pThread->join();
    }

private:
    void ThreadFunc()
    {
        while (!m_oShutdown.is_set())
        {
            m_nRes = m_pAdapter->GetRecentSample(m_hInput, m_pSample);
            if (fep::isFailed(m_nRes))
            {
                return;
            }
        }
    }
public:
    fep::Result m_nRes;
    cTransmissionAdapter* m_pAdapter;
    handle_t m_hInput;
    IPreparationDataSample* m_pSample;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
    a_util::concurrency::semaphore m_oShutdown;
};
class cMultiThreadTestRegisterSignalThread
{
public:
    cMultiThreadTestRegisterSignalThread(cTransmissionAdapter* pTransmissionAdapter, uint8_t ThreadId)
        : m_pTransmissionAdapter(pTransmissionAdapter)
        , m_RegHandle(NULL)
        , m_strDDLDesc("")
        , m_nThreadId(ThreadId)
        , m_oShutdown()
    {
        m_strDDLDesc = a_util::strings::format(s_strDescriptionTemplate.c_str(), s_strSignalDescription.c_str());
        ASSERT_RESULT_OR_THROW(fep::helpers::CalculateSignalSizeFromDescription("tTestSignal", m_strDDLDesc.c_str(), m_szSample));
        m_pThread.reset(new a_util::concurrency::thread(&cMultiThreadTestRegisterSignalThread::ThreadFunc, this));
    }
    ~cMultiThreadTestRegisterSignalThread()
    {
        m_oShutdown.notify();
        m_pThread->join();
    }
private:
    void ThreadFunc()
    {
        std::string strSignalName = a_util::strings::format("ThreadSignal%d", m_nThreadId);
        tSignal oTestSignal = { strSignalName.c_str(),"tTestSignal",m_strDDLDesc.c_str(),SD_Input,m_szSample,false,false,1,SER_Ddl,false, true, false, std::string("") };
        while (!m_oShutdown.is_set())
        {
            m_pTransmissionAdapter->RegisterSignal(oTestSignal, m_RegHandle);
            m_pTransmissionAdapter->UnregisterSignal(m_RegHandle);
        }
    }
    cTransmissionAdapter* m_pTransmissionAdapter;
    handle_t m_RegHandle;
    std::string m_strDDLDesc;
    size_t m_szSample;
    uint8_t m_nThreadId;
    a_util::concurrency::semaphore m_oShutdown;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
};
class cMessageSyncListener : public cNotificationListener,
    public cCommandListener,
    public IPreparationDataListener
{
private:
    int * m_pnTestVar;
public:
    a_util::concurrency::semaphore m_semBlock;
    a_util::concurrency::semaphore m_semSignal;

public:
    cMessageSyncListener(int * pnTestVar) : m_pnTestVar(pnTestVar),
        m_semBlock()
    {
    }

public: // override cNotificationListener
    fep::Result Update(IPropertyNotification const * pNot)
    {
        m_semSignal.notify();
        m_semBlock.wait();
        *m_pnTestVar = 1;
        return ERR_NOERROR;
    }

public: // override cCommandListener
    fep::Result Update(ISetPropertyCommand const * poCommand)
    {
        m_semSignal.notify();
        m_semBlock.wait();
        *m_pnTestVar = 1;
        return ERR_NOERROR;
    }

public: // IUserDataListener
    fep::Result Update(const IPreparationDataSample* poSample)
    {
        m_semSignal.notify();
        m_semBlock.wait();
        *m_pnTestVar = 1;
        return ERR_NOERROR;
    }
};
class cAdapterDestroyer
{
public:
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
    a_util::concurrency::semaphore m_semBlock;
    a_util::concurrency::semaphore m_semFinshed;
    cTransmissionAdapter* m_pAdapter;
    int* m_pInt;
public:
    cAdapterDestroyer(cTransmissionAdapter* pAdapter, int * pInt) :
        m_pAdapter(pAdapter),
        m_pInt(pInt)
    {
        m_pThread.reset(new a_util::concurrency::thread(&cAdapterDestroyer::ThreadFunc, this));
    }

    void ThreadFunc()
    {
        m_semBlock.wait();
        m_pAdapter->Destroy();
        *m_pInt = 0;
        m_semFinshed.notify();
    }

    void DestroyAdapter()
    {
        m_semBlock.notify();
    }

    void KillThread()
    {
        m_pThread->join();
        m_pThread.reset();
    }
    ~cAdapterDestroyer()
    {
    }
};

class cMessageListener : public ICommandListener, public INotificationListener
{
public:
    cMessageListener(const char* strSender) : m_oSetPropertyCommand(cSetPropertyCommand("")),
        m_oGetPropertyCommand(cGetPropertyCommand("")),
        m_oRegPropListenerCommand(cRegPropListenerCommand("")),
        m_oUnregPropListenerCommand(cUnregPropListenerCommand("")),
        m_oControlCommand(cControlCommand("")), m_oCustomCommand(cCustomCommand("")),
        m_oDeletePropertyCommand(cDeletePropertyCommand("")),
        m_oGetSignalInfoCommand(cGetSignalInfoCommand("")),
        m_oResolveSignalTypeCommand(cResolveSignalTypeCommand("")),
        m_oSignalDescriptionCommand(cSignalDescriptionCommand("")),
        m_oMappingConfigurationCommand(cMappingConfigurationCommand("")),
        m_oNameChangeCommand(cNameChangeCommand("")),
        m_oGetScheduleCommand(cGetScheduleCommand("")),
        m_oLogNotification(cIncidentNotification("")), m_oPropertyNotification(cPropertyNotification("")),
        m_oStateNotification(cStateNotification("")),
        m_oRegPropListenerAckNotification(cRegPropListenerAckNotification("")),
        m_oUnregPropListenerAckNotification(cUnregPropListenerAckNotification("")),
        m_oPropertyChangedNotification(cPropertyChangedNotification("")),
        m_oNameChangedNotification(cNameChangedNotification("")),
        m_oSignalInfoNotification(cSignalInfoNotification("")),
        m_oSignalDescriptionNotification(cSignalDescriptionNotification("")),
        m_oResultCodeNotification(cResultCodeNotification("")),
        m_oScheduleNotification(cScheduleNotification("")),
        m_pCurrentMessage(NULL), m_szReceiveCounter(0), m_bBlock(false), m_strSenderFilter(strSender)
    {
    }

    fep::Result Update(ICustomCommand const * poCommand)
    {
        if (m_strSenderFilter == poCommand->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMilliseconds(100);
            }
            m_oCustomCommand = cCustomCommand(poCommand->ToString());
            m_pCurrentMessage = &m_oCustomCommand;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    fep::Result Update(IControlCommand const * poCommand)
    {
        if (m_strSenderFilter == poCommand->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMilliseconds(100);
            }
            m_oControlCommand = cControlCommand(poCommand->ToString());
            m_pCurrentMessage = &m_oControlCommand;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    fep::Result Update(ISetPropertyCommand const * poCommand)
    {
        if (m_strSenderFilter == poCommand->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMilliseconds(100);
            }
            m_oSetPropertyCommand = cSetPropertyCommand(poCommand->ToString());
            m_pCurrentMessage = &m_oSetPropertyCommand;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    fep::Result Update(IGetPropertyCommand const * poCommand)
    {
        if (m_strSenderFilter == poCommand->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMilliseconds(100);
            }
            m_oGetPropertyCommand = cGetPropertyCommand(poCommand->ToString());
            m_pCurrentMessage = &m_oGetPropertyCommand;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IRegPropListenerCommand const * poCommand)
    {
        if (m_strSenderFilter == poCommand->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMilliseconds(100);
            }
            m_oRegPropListenerCommand = cRegPropListenerCommand(poCommand->ToString());
            m_pCurrentMessage = &m_oRegPropListenerCommand;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IUnregPropListenerCommand const * poCommand)
    {
        if (m_strSenderFilter == poCommand->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMilliseconds(100);
            }
            m_oUnregPropListenerCommand = cUnregPropListenerCommand(poCommand->ToString());
            m_pCurrentMessage = &m_oUnregPropListenerCommand;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IDeletePropertyCommand const * poCommand)
    {
        if (m_strSenderFilter == poCommand->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMicroseconds((timestamp_t)100e3);
            }
            m_oDeletePropertyCommand = cDeletePropertyCommand(poCommand->ToString());
            m_pCurrentMessage = &m_oDeletePropertyCommand;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IGetSignalInfoCommand const * poCommand)
    {
        if (m_strSenderFilter == poCommand->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMicroseconds((timestamp_t)100e3);
            }
            m_oGetSignalInfoCommand = cGetSignalInfoCommand(poCommand->ToString());
            m_pCurrentMessage = &m_oGetSignalInfoCommand;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IResolveSignalTypeCommand const * poCommand)
    {
        if (m_strSenderFilter == poCommand->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMicroseconds((timestamp_t)100e3);
            }
            m_oResolveSignalTypeCommand = cResolveSignalTypeCommand(poCommand->ToString());
            m_pCurrentMessage = &m_oResolveSignalTypeCommand;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(ISignalDescriptionCommand const * poCommand)
    {
        if (m_strSenderFilter == poCommand->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMicroseconds((timestamp_t)100e3);
            }
            m_oSignalDescriptionCommand = cSignalDescriptionCommand(poCommand->ToString());
            m_pCurrentMessage = &m_oSignalDescriptionCommand;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IMappingConfigurationCommand const * poCommand)
    {
        if (m_strSenderFilter == poCommand->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMicroseconds((timestamp_t)100e3);
            }
            m_oMappingConfigurationCommand = cMappingConfigurationCommand(poCommand->ToString());
            m_pCurrentMessage = &m_oMappingConfigurationCommand;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(INameChangeCommand const * poCommand)
    {
        if (m_strSenderFilter == poCommand->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMicroseconds((timestamp_t)100e3);
            }
            m_oNameChangeCommand = cNameChangeCommand(poCommand->ToString());
            m_pCurrentMessage = &m_oNameChangeCommand;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IGetScheduleCommand const * poCommand)
    {
        if (m_strSenderFilter == poCommand->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMicroseconds((timestamp_t)100e3);
            }
            m_oGetScheduleCommand = cGetScheduleCommand(poCommand->ToString());
            m_pCurrentMessage = &m_oGetScheduleCommand;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IStateNotification const * pStateNotification)
    {
        if (m_strSenderFilter == pStateNotification->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMilliseconds(100);
            }
            m_oStateNotification = cStateNotification(pStateNotification->ToString());
            m_pCurrentMessage = &m_oStateNotification;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IIncidentNotification const * pLogNotification)
    {
        if (m_strSenderFilter == pLogNotification->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMilliseconds(100);
            }
            m_oLogNotification = cIncidentNotification(pLogNotification->ToString());
            m_pCurrentMessage = &m_oLogNotification;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IPropertyNotification const * pPropertyNotification)
    {
        if (m_strSenderFilter == pPropertyNotification->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMilliseconds(100);
            }
            m_oPropertyNotification = cPropertyNotification(pPropertyNotification->ToString());
            m_pCurrentMessage = &m_oPropertyNotification;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IRegPropListenerAckNotification const * pNotification)
    {
        if (m_strSenderFilter == pNotification->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMilliseconds(100);
            }
            m_oRegPropListenerAckNotification = cRegPropListenerAckNotification(pNotification->ToString());
            m_pCurrentMessage = &m_oRegPropListenerAckNotification;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IUnregPropListenerAckNotification const * pNotification)
    {
        if (m_strSenderFilter == pNotification->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMilliseconds(100);
            }
            m_oUnregPropListenerAckNotification = cUnregPropListenerAckNotification(pNotification->ToString());
            m_pCurrentMessage = &m_oUnregPropListenerAckNotification;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IPropertyChangedNotification const * pNotification)
    {
        if (m_strSenderFilter == pNotification->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMilliseconds(100);
            }
            m_oPropertyChangedNotification = cPropertyChangedNotification(pNotification->ToString());
            m_pCurrentMessage = &m_oPropertyChangedNotification;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(INameChangedNotification const * pNotification)
    {
        if (m_strSenderFilter == pNotification->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMicroseconds((timestamp_t)100e3);
            }
            m_oNameChangedNotification = cNameChangedNotification(pNotification->ToString());
            m_pCurrentMessage = &m_oNameChangedNotification;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(ISignalInfoNotification const * pNotification)
    {
        if (m_strSenderFilter == pNotification->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMicroseconds((timestamp_t)100e3);
            }
            m_oSignalInfoNotification = cSignalInfoNotification(pNotification->ToString());
            m_pCurrentMessage = &m_oSignalInfoNotification;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(ISignalDescriptionNotification const * pNotification)
    {
        if (m_strSenderFilter == pNotification->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMicroseconds((timestamp_t)100e3);
            }
            m_oSignalDescriptionNotification = cSignalDescriptionNotification(pNotification->ToString());
            m_pCurrentMessage = &m_oSignalDescriptionNotification;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IResultCodeNotification const * pNotification)
    {
        if (m_strSenderFilter == pNotification->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMicroseconds((timestamp_t)100e3);
            }
            m_oResultCodeNotification = cResultCodeNotification(pNotification->ToString());
            m_pCurrentMessage = &m_oResultCodeNotification;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IMuteSignalCommand const *)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IScheduleNotification const * pNotification)
    {
        if (m_strSenderFilter == pNotification->GetSender())
        {
            while (m_bBlock)
            {
                a_util::system::sleepMicroseconds((timestamp_t)100e3);
            }
            m_oScheduleNotification = cScheduleNotification(pNotification->ToString());
            m_pCurrentMessage = &m_oScheduleNotification;
            m_szReceiveCounter++;
        }
        return ERR_NOERROR;
    }

    virtual fep::Result Update(IRPCCommand const * pRPCommand)
    {
        return ERR_NOERROR;
    }

    cSetPropertyCommand m_oSetPropertyCommand;
    cGetPropertyCommand m_oGetPropertyCommand;
    cRegPropListenerCommand m_oRegPropListenerCommand;
    cUnregPropListenerCommand m_oUnregPropListenerCommand;
    cControlCommand m_oControlCommand;
    cCustomCommand m_oCustomCommand;
    cDeletePropertyCommand m_oDeletePropertyCommand;
    cGetSignalInfoCommand m_oGetSignalInfoCommand;
    cResolveSignalTypeCommand m_oResolveSignalTypeCommand;
    cSignalDescriptionCommand m_oSignalDescriptionCommand;
    cMappingConfigurationCommand m_oMappingConfigurationCommand;
    cNameChangeCommand m_oNameChangeCommand;
    cGetScheduleCommand m_oGetScheduleCommand;

    cIncidentNotification m_oLogNotification;
    cPropertyNotification m_oPropertyNotification;
    cStateNotification m_oStateNotification;
    cRegPropListenerAckNotification m_oRegPropListenerAckNotification;
    cUnregPropListenerAckNotification m_oUnregPropListenerAckNotification;
    cPropertyChangedNotification m_oPropertyChangedNotification;
    cNameChangedNotification m_oNameChangedNotification;
    cSignalInfoNotification m_oSignalInfoNotification;
    cSignalDescriptionNotification m_oSignalDescriptionNotification;
    cResultCodeNotification m_oResultCodeNotification;
    cScheduleNotification m_oScheduleNotification;

    IMessage * m_pCurrentMessage;
    size_t m_szReceiveCounter;
    bool m_bBlock;
    std::string m_strSenderFilter;
};

class cMultiThreadTestRegisterDataListenerThread
{
public:
    cMultiThreadTestRegisterDataListenerThread(cTransmissionAdapter* pTransmissionAdapter, handle_t RecvHandle)
        : m_pTransmissionAdapter(pTransmissionAdapter)
        , m_DataListener()
        , m_RecvHandle(RecvHandle)
        , m_oShutdown()
    {
        m_pThread.reset(new a_util::concurrency::thread(&cMultiThreadTestRegisterDataListenerThread::ThreadFunc, this));
    }
    ~cMultiThreadTestRegisterDataListenerThread()
    {
        m_oShutdown.notify();
        m_pThread->join();
    }
private:
    void ThreadFunc()
    {
        while (!m_oShutdown.is_set())
        {
            m_pTransmissionAdapter->RegisterDataListener(&m_DataListener, m_RecvHandle);
            m_pTransmissionAdapter->UnregisterDataListener(&m_DataListener, m_RecvHandle);
        }
    }
    cTransmissionAdapter* m_pTransmissionAdapter;
    cDataListener m_DataListener;
    handle_t m_RecvHandle;
    a_util::concurrency::semaphore m_oShutdown;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
};

class cBlockingListener : public IPreparationDataListener
{
public:
    cBlockingListener(a_util::concurrency::semaphore* pEvent) :
        m_pEvent(pEvent)
    {

    }
    ~cBlockingListener()
    {

    }
public: // implementing IPreparationDataListener
    fep::Result Update(const IPreparationDataSample *poPreparationSample)
    {
        m_pEvent->wait();
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

private:
    a_util::concurrency::semaphore* m_pEvent;
    std::list<fep::IUserDataListener *> m_listListeners;
};
#endif
