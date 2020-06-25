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
/// Someone should add a header here some time

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <sys/types.h>
#include <a_util/concurrency/fast_mutex.h>
#include <a_util/memory/memory.h>
#include <a_util/memory/memorybuffer.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>
#include <codec/codec.h>
#include <codec/struct_element.h>
#include <ddlrepresentation/ddldescription.h>
#include <ddlrepresentation/ddlimporter.h>
#include <ddlrepresentation/ddlprinter.h>
#include <ddlrepresentation/ddlversion.h>
#include <serialization/serialization.h>

#include "_common/fep_optional.h"
#include "fep3/components/legacy/property_tree/fep_module_header_config.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep_errors.h"
#include "fep_queue_manager.h"
#include "fep_sdk_participant_version.h"
#include "fep_transmission_adapter_common.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler.h"
#include "incident_handler/fep_severity_level.h"
#include "signal_registry/fep_signal_struct.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include "transmission_adapter/fep_options_factory.h"
#include "transmission_adapter/fep_preparation_data_listener_intf.h"
#include "transmission_adapter/fep_receive_intf.h"
#include "transmission_adapter/fep_serialization_helpers.h"
#include "transmission_adapter/fep_signal_serialization.h"
#include "transmission_adapter/fep_transmission_driver_intf.h"
#include "transmission_adapter/fep_transmission_sample_intf.h"
#include "transmission_adapter/fep_receiver.h"

#define DEFAULT_SIZE_FOR_RAW_SIGNALS 62 * 1024
using namespace ddl;

namespace fep {
class IPreparationDataSample;

a_util::concurrency::fast_mutex cDataReceiver::ms_oStaticDDLSync;
static uint32_t s_nDefaultSampleAllocationCount = 256;
static uint32_t s_nDefaultRawSampleAllocationCount = 20;

cDataReceiver::cDataReceiver() :
    m_pPropertyTree(NULL),
    m_pIncidentInvocationHandler(NULL),
    m_pDriver(NULL),
    m_pDriverReceiver(NULL),
    m_pQueueManager(NULL),
    m_pCurrentDataSample(NULL),
    m_bDisableDdlSerialization(false),
    m_szSignalSize(0),
    m_bRaw(false)
{
}

cDataReceiver::~cDataReceiver()
{
    //we need to lock the job BEFORE the Listener Mutex is locked
    //otherwise we have a deadlock within the DoJob Call where the job is locked and the listeners later
    a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> joblock(m_mtxJob);
    // wait for any receives to return
    a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oSync(m_mtxListener);
    if (NULL != m_pDriverReceiver)
    {
        m_pDriver->DestroyReceiver(m_pDriverReceiver);
    }
    if (NULL != m_pCurrentDataSample)
    {
        delete m_pCurrentDataSample;
        m_pCurrentDataSample = NULL;
    }
    FlushQueue(true);

    sDataContainer* pDataItem;

    while(m_qPreAllocQueue.TryDequeue(pDataItem))
    {
        ::free(pDataItem->pData);
        delete pDataItem;
    }
}

fep::Result cDataReceiver::Create(ITransmissionDriver* pDriver, fep::IPropertyTree* pPropertyTreePrivate,
    fep::IIncidentInvocationHandler* pIncidentInvocationHandler, cQueueManager* pQueueManager,
    const tSignal& oSignal)
{
    if (!pDriver || !pPropertyTreePrivate ||!pIncidentInvocationHandler)
    {
        return ERR_POINTER;
    }
    fep::Result nResult = ERR_NOERROR;

    m_pPropertyTree = pPropertyTreePrivate;
    m_pIncidentInvocationHandler = pIncidentInvocationHandler;
    m_pDriver = pDriver;
    m_pQueueManager = pQueueManager;

    m_strSignalName = oSignal.strSignalName;
    m_bDisableDdlSerialization = (oSignal.eSerialization == fep::SER_Raw);
    m_bRaw = oSignal.bIsRaw.GetValue();
    m_strSignalType = oSignal.strSignalType;
    m_szSignalSize = oSignal.szSampleSize;
    if (0 == m_szSignalSize)
    {
        m_szSignalSize = DEFAULT_SIZE_FOR_RAW_SIGNALS;
    }

    //Create options for the driver
    cOptionsFactory oSignalOptionsFactory;
    oSignalOptionsFactory.Initialize(m_pDriver);
    m_oSignalOptions = oSignalOptionsFactory.GetSignalOptions();

    nResult = GatherSignalOptions(m_oSignalOptions, oSignal);

    if (isOk(nResult))
    {
        uint32_t preallocated_samples_count = 0;
        if (!m_bRaw && !(m_bDisableDdlSerialization && m_szSignalSize > 0))
        {
            // Warning: THIS MUST BE STATIC since DDLDescription is using static calls and
            // static members internally!
            a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oSync(ms_oStaticDDLSync);

            using namespace fep::helpers;

            a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oSync2(s_oMediaCoderMutex);
            // Create the media description for the signal including all additional information from
            // preparation and transmission fep layer.
            // Create a description object from the signal description

            // Move this code to default "InitializeSampleWithDefaultValues"
            ddl::DDLVersion default_ddl_version = ddl::DDLVersion::getDefaultVersion();
            DDLDescription* pDefaultDescription = DDLDescription::createDefault(default_ddl_version, 0);
            DDLImporter oImporter;
            oImporter.setXML(oSignal.strSignalDesc);
            oImporter.createPartial(pDefaultDescription, default_ddl_version);
            ddl::DDLDescription* pDescription = oImporter.getDDL();
            DDLPrinter oDDLPrinter;
            oDDLPrinter.visitDDL(pDescription);


            m_oCodecFactory = ddl::CodecFactory(oSignal.strSignalType.c_str(), oDDLPrinter.getXML().c_str());
            nResult = m_oCodecFactory.isValid().getErrorCode();
            if (fep::isOk(nResult))
            {
                cDataSampleFactory::CreateSample(&m_pCurrentDataSample);
                /* To ensure size is set to size of user data (without any sync-flags,
                * etc.) a new cMediaCoder and IMediaSerializer is created and used
                * for user data only.
                */

                {
                    m_pCurrentDataSample->SetSize(m_szSignalSize);


                    fep::Result nCurrentResult =
                        InitializeSampleWithDefaultValues(m_pCurrentDataSample, pDescription,
                            m_pIncidentInvocationHandler, oSignal.strSignalType, oSignal.strSignalDesc.c_str());
                    delete pDefaultDescription;
                    oImporter.destroyDDL();

                    if (fep::isFailed(nCurrentResult))
                    {
                        /* InitializeSampleWithDefaultValues returns a result value
                        * indicating the overall success of all initializations.
                        * The result of this operation is logged on erros, but not
                        * handled further.
                        * The reason for this behavior is:
                        * The default initialization was introduced lately and
                        * should not break existing code.
                        */
                        INVOKE_INCIDENT(m_pIncidentInvocationHandler,
                            FSI_GENERAL_INFORMATION, SL_Critical_Local,
                            a_util::strings::format(
                                "Failed to initialize element \"%s\" in signal %s "
                                "with default values: Result code %d.",
                                GetModuleName(), m_strSignalName.c_str(),
                                nCurrentResult.getErrorCode()).c_str())
                    }
                    m_pCurrentDataSample->SetSignalHandle(this);
                }
            }
            preallocated_samples_count = s_nDefaultSampleAllocationCount;
        }
        else
        {
            cDataSampleFactory::CreateSample(&m_pCurrentDataSample);
            nResult = m_pCurrentDataSample->SetSize(m_szSignalSize);
            m_pCurrentDataSample->SetSignalHandle(this);
            preallocated_samples_count = s_nDefaultRawSampleAllocationCount;
        }
        if (fep::isOk(nResult))
        {
            for (unsigned int i = 0; i < preallocated_samples_count; ++i)
            {
                sDataContainer* pDataContainer = new sDataContainer;
                pDataContainer->szCapacity = m_szSignalSize + sizeof(cFepDataHeader);
                pDataContainer->szSize = m_szSignalSize + sizeof(cFepDataHeader);
                pDataContainer->pData = ::malloc(pDataContainer->szCapacity);
                if (NULL == pDataContainer->pData)
                {
                    nResult = ERR_MEMORY;
                    break;
                }
                else
                {
                    m_qPreAllocQueue.Enqueue(pDataContainer);
                }
            }
        }
        if (fep::isOk(nResult))
        {
            nResult = m_pDriver->CreateReceiver(m_pDriverReceiver, m_oSignalOptions);
        }
        if (fep::isOk(nResult))
        {
            nResult = m_pDriverReceiver->SetReceiver(cDataReceiver::EnqueueReceivedData, reinterpret_cast<void*>(this));
        }
    }
    return nResult;
}

fep::Result cDataReceiver::GatherSignalOptions(cSignalOptions& oDriverSignalOptions, const tSignal& oSignal)
{
    fep::Result nResult = ERR_NOERROR;
    // offset for data header
    if (!(oDriverSignalOptions.SetOption("SignalName", oSignal.strSignalName)
        && oDriverSignalOptions.SetOption("SignalSize", m_szSignalSize + sizeof(cFepDataHeader))))
    {
        nResult = ERR_FAILED;
    }
    else
    {
        if (!oDriverSignalOptions.SetOption("IsVariableSignalSize", oSignal.bIsRaw.GetValue()))
        {
            if (oSignal.bIsRaw.IsSet())
            {
                nResult = ERR_NOT_SUPPORTED;
            }
        }

        if (!oDriverSignalOptions.SetOption("IsReliable", oSignal.bIsReliable.GetValue()))
        {
            if (oSignal.bIsReliable.IsSet())
            {
                nResult = ERR_NOT_SUPPORTED;
            }
        }

        if (!oDriverSignalOptions.SetOption("UseLowLatProfile", oSignal.bRTILowLat.GetValue()))
        {
            if (oSignal.bRTILowLat.IsSet())
            {
                nResult = ERR_NOT_SUPPORTED;
            }
        }

        if (!oDriverSignalOptions.SetOption("UseMulticast", oSignal.strRTIMulticast.GetValue()))
        {
            if (oSignal.strRTIMulticast.IsSet())
            {
                nResult = ERR_NOT_SUPPORTED;
            }
        }
    }
    return nResult;
}

fep::Result cDataReceiver::Enable()
{
    return m_pDriverReceiver->Enable();
}

fep::Result cDataReceiver::Disable()
{
    return m_pDriverReceiver->Disable();
}

fep::Result cDataReceiver::Mute()
{
    return m_pDriverReceiver->Mute();
}

fep::Result cDataReceiver::Unmute()
{
    return m_pDriverReceiver->Unmute();
}

fep::Result cDataReceiver::FlushQueue(bool no_lock)
{
    //we need to do this because this is called within DTOR where the JOB Must be locked BEFORE the Listener Mutex is locked
    //otherwise we have a deadlock!!
    if (!no_lock)
    {
        m_mtxJob.lock();
    }
    sDataContainer* pDataItem;
    while(m_qReceiveQueue.TryDequeue(pDataItem))
    {
        m_qPreAllocQueue.Enqueue(pDataItem);
    }
    if (!no_lock)
    {
        m_mtxJob.unlock();
    }
    return ERR_NOERROR;
}

void cDataReceiver::EnqueueReceivedData(void* pInstance, const void* pData, size_t szSize)
{
    cDataReceiver* pReceiver = reinterpret_cast<cDataReceiver*>(pInstance);
    sDataContainer* pDataContainer;
    if(pReceiver->m_qPreAllocQueue.TryDequeue(pDataContainer))
    {   
        if(pReceiver->m_bRaw)
        {
            if(szSize > pDataContainer->szCapacity)
            {
                pDataContainer->pData = realloc(pDataContainer->pData, szSize);
                pDataContainer->szCapacity = szSize;
            }
        }
        else
        {
            if(szSize > pReceiver->m_szSignalSize + sizeof(cFepDataHeader))
            {

                INVOKE_INCIDENT(pReceiver->m_pIncidentInvocationHandler,
                    fep::FSI_TRANSM_RX_WRONG_SAMPLE_SIZE, fep::SL_Critical_Local,
                    a_util::strings::format("Sample has unexpected size (Expected %d, got %d). (Instance %s::%s)",
                    pReceiver->m_szSignalSize + sizeof(cFepDataHeader), szSize,
                    pReceiver->GetModuleName(), pReceiver->m_strSignalName.c_str()).c_str());
                return;
            }
        }

        if(a_util::memory::copy(pDataContainer->pData, pDataContainer->szCapacity, pData, szSize))
        {
            pDataContainer->szSize = szSize;

            pReceiver->m_qReceiveQueue.Enqueue(pDataContainer);
            pReceiver->m_pQueueManager->EnqueueJob(pReceiver);
        }
        else
        {
            pReceiver->m_qPreAllocQueue.Enqueue(pDataContainer);
        }
    }
}

fep::Result cDataReceiver::RegisterListener(IPreparationDataListener* pListener)
{
    fep::Result nResult = ERR_NOERROR;
    a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oSync(m_mtxListener);

    if (NULL != pListener)
    {
        if(std::find(m_lstListener.begin(),m_lstListener.end(), pListener) == m_lstListener.end())
        {
            m_lstListener.push_back(pListener);
        }
        else
        {
            nResult = ERR_FAILED;
        }
    }
    return nResult;
}

fep::Result cDataReceiver::UnregisterListener(IPreparationDataListener* pListener)
{
    a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oSync(m_mtxListener);

    bool bFound = false;
    for (tListenerList::iterator pIter = m_lstListener.begin();
        pIter != m_lstListener.end(); pIter++)
    {
        if (*pIter == pListener)
        {
            m_lstListener.erase(pIter);
            bFound = true;
            break;
        }
    }
    fep::Result nRes = bFound;
    if (fep::isFailed(nRes))
    {
        nRes = ERR_NOT_FOUND;
    }
    return nRes;
}

void cDataReceiver::DoJob()
{
    //try to enter processing in this reader
    if (m_mtxJob.try_lock())
    {
        sDataContainer* pDataItem;
        //process until queue is empty
        while (m_qReceiveQueue.TryDequeueAndUnlockGuardIfEmpty(pDataItem, m_mtxJob))
        {
            Process(pDataItem->pData, pDataItem->szSize);
            m_qPreAllocQueue.Enqueue(pDataItem);
        }
    }
    else
    {
        //Receiver is locked by another worker so do nothing
    }
}

fep::Result cDataReceiver::Process(void *pData, size_t szSize)
{
    bool bSync = false;
    uint64_t nFrameId = 0;
    uint64_t nSampleNumberInFrame = 0;
    int64_t nSendTimeStamp = 0;

    {
        const cFepDataHeader* pFepDataHeader = reinterpret_cast<const cFepDataHeader*>
            (pData);

        uint8_t nMajorVersion = pFepDataHeader->m_nMajorVersion;
        bSync = (0 != pFepDataHeader->m_nSync) ? true : false;
        if(nMajorVersion != FEP_SDK_PARTICIPANT_VERSION_MAJOR )
        {
            INVOKE_INCIDENT(m_pIncidentInvocationHandler,
                fep::FSI_TRANSM_SAMPLE_VERSION_FAILED, fep::SL_Critical_Local,
                a_util::strings::format(
                "Received a package with wrong FEP SDK major version (Expected %d, but got %d). (Instance %s::%s)",
                FEP_SDK_PARTICIPANT_VERSION_MAJOR, nMajorVersion,
                GetModuleName(), m_strSignalName.c_str()).c_str());
            return ERR_INVALID_VERSION;
        }

        uint8_t nSerAndByteOrderFlags = pFepDataHeader->m_nSerAndByteOrderFlags;
        header::ByteOrderAndSerialization nByteOrderFlag = header::GetByteOrderFlag(nSerAndByteOrderFlags);
        header::ByteOrderAndSerialization nSerializationFlag= header::GetSerializationFlag(nSerAndByteOrderFlags);

        if(0 == nByteOrderFlag || 0 == nSerializationFlag)
        {
            INVOKE_INCIDENT(m_pIncidentInvocationHandler,
                fep::FSI_TRANSM_FEP_PROTO_CORRUPT_HEADER, fep::SL_Critical_Local,
                a_util::strings::format("Received a package with corrupt header. (Instance %s::%s)",
                GetModuleName(), m_strSignalName.c_str()).c_str());
            return ERR_INVALID_FLAGS;
        }

        // we check the byteorder and convert if necessary
        nFrameId = header::ConvertToCorrectByteorder(pFepDataHeader->m_nFrameId, nByteOrderFlag);
        nSampleNumberInFrame = header::ConvertToCorrectByteorder(pFepDataHeader->m_nSampleNumber,
            nByteOrderFlag);
        nSendTimeStamp = header::ConvertToCorrectByteorder(pFepDataHeader->m_nSendTimeStamp, nByteOrderFlag);


        if(sizeof(cFepDataHeader) + m_pCurrentDataSample->GetSize() != szSize)
        {
            if(m_bRaw)
            {
                m_pCurrentDataSample->AdaptSize(szSize-sizeof(cFepDataHeader));
            }
            else
            {
                INVOKE_INCIDENT(m_pIncidentInvocationHandler,
                    fep::FSI_TRANSM_FEP_PROTO_CORRUPT_HEADER, fep::SL_Critical_Local,
                    a_util::strings::format("Sample has unexpected size (Expected %d, got %d). (Instance %s::%s)",
                    sizeof(cFepDataHeader) + m_pCurrentDataSample->GetSize(), szSize,
                    GetModuleName(), m_strSignalName.c_str()).c_str());
                return ERR_INVALID_FLAGS;
            }
        }

        if(nSerializationFlag == header::SERIALIZATION_DDL)
        {
            if(m_bDisableDdlSerialization)
            {
                // just throw an incident and continue
                INVOKE_INCIDENT(m_pIncidentInvocationHandler,
                    fep::FSI_TRANSM_FEP_PROTO_WRONG_SER_MODE, fep::SL_Critical_Local,
                    a_util::strings::format(
                    "Received a serialized package while serialization is turned off. Dropping "
                    "package! (Instance %s::%s)",
                    GetModuleName(), m_strSignalName.c_str()).c_str());
                return ERR_INVALID_FLAGS;

            }
            else
            {
                ddl::Decoder oDec = m_oCodecFactory.makeDecoderFor(static_cast<char*>(pData) + sizeof(cFepDataHeader),
                    szSize - sizeof(cFepDataHeader), ddl::serialized);
                a_util::memory::MemoryBuffer oDest(m_pCurrentDataSample->GetPtr(), m_pCurrentDataSample->GetCapacity());
                ddl::serialization::transform_to_buffer(oDec, oDest);
            }
        }
        else if(nSerializationFlag == header::SERIALIZATION_RAW)
        {
            if (m_bDisableDdlSerialization && (nByteOrderFlag == header::GetLocalSystemByteorder()))
            {
                cMutexGuard oLockGuard(m_oCurrentSampleMutex);
                a_util::memory::copy(m_pCurrentDataSample->GetPtr(), m_pCurrentDataSample->GetCapacity(), static_cast<char*>(pData) + sizeof(cFepDataHeader),
                     szSize-sizeof(cFepDataHeader));
            }
            else if(false == m_bDisableDdlSerialization)
            {
                INVOKE_INCIDENT(m_pIncidentInvocationHandler,
                    fep::FSI_TRANSM_FEP_PROTO_WRONG_SER_MODE, fep::SL_Critical_Local,
                    a_util::strings::format(
                    "Received an unserialized package while serialization is turned on. Dropping "
                    "package! Instance %s::%s)",
                GetModuleName(), m_strSignalName.c_str()).c_str());
                return ERR_INVALID_FLAGS;
            }
            else if (nByteOrderFlag != header::GetLocalSystemByteorder())
            {
                //byteorder is different! throw incident and return!
                INVOKE_INCIDENT(m_pIncidentInvocationHandler,
                    fep::FSI_TRANSM_FEP_PROTO_WRONG_SER_MODE, fep::SL_Critical_Local,
                    a_util::strings::format(
                    "Received a package with wrong byteorder while serialization is turned off. Dropping "
                    "package! Instance %s::%s)",
                GetModuleName(), m_strSignalName.c_str()).c_str());
                return ERR_INVALID_FLAGS;
            }
        }
    }

    m_pCurrentDataSample->SetSyncFlag(bSync);
    m_pCurrentDataSample->SetFrameId(nFrameId);
    m_pCurrentDataSample->SetSampleNumberInFrame(static_cast<uint16_t>(nSampleNumberInFrame));
    m_pCurrentDataSample->SetTime(nSendTimeStamp);

    cMutexGuard oLockGuard(m_oCurrentSampleMutex);
    return UpdateListeners(m_pCurrentDataSample);
}

fep::Result cDataReceiver::UpdateListeners(IPreparationDataSample* poSample)
{
    a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oSync(m_mtxListener);

    for (tListenerList::iterator pIter = m_lstListener.begin();
        pIter != m_lstListener.end(); pIter++)
    {
        (*pIter)->Update(poSample);
    }
    return ERR_NOERROR;
}

fep::Result cDataReceiver::GetCurrentSample(fep::IPreparationDataSample* pSample)
{
    cMutexGuard m_oLockGuard(m_oCurrentSampleMutex);
    return m_pCurrentDataSample->CopyTo(*pSample);
}

const char* cDataReceiver::GetModuleName()
{
    const char* strModuleName = NULL;
    m_pPropertyTree->GetPropertyValue(g_strElementHeaderPath_strElementName, strModuleName);
    return strModuleName;
}

}  // namespace fep
