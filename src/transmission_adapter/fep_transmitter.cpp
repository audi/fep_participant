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
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <a_util/concurrency/fast_mutex.h>
#include <a_util/memory/memory.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>
#include <codec/codec.h>
#include <ddlrepresentation/ddldescription.h>
#include <ddlrepresentation/ddlimporter.h>
#include <ddlrepresentation/ddlprinter.h>
#include <ddlrepresentation/ddlversion.h>
#include <serialization/serialization.h>

#include "_common/fep_optional.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/components/legacy/property_tree/fep_module_header_config.h"
#include "fep_errors.h"
#include "fep_sdk_participant_version.h"
#include "fep_transmission_adapter_common.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler.h"
#include "incident_handler/fep_severity_level.h"
#include "signal_registry/fep_signal_struct.h"
#include "transmission_adapter/fep_options_factory.h"
#include "transmission_adapter/fep_preparation_data_sample_intf.h"
#include "transmission_adapter/fep_serialization_helpers.h"
#include "transmission_adapter/fep_signal_serialization.h"
#include "transmission_adapter/fep_transmission_driver_intf.h"
#include "transmission_adapter/fep_transmit_intf.h"
#include "transmission_adapter/fep_transmitter.h"

using namespace fep;
using namespace ddl;

#define DEFAULT_SIZE_FOR_RAW_SIGNALS 62 * 1024 

a_util::concurrency::fast_mutex cTransmitter::ms_oStaticDDLSync;

cTransmitter::cTransmitter():
    m_pSendSample{ 0, nullptr },
    m_pDriver(NULL),
    m_pDriverTransmitter(NULL),
    m_bDisableDdlSerialization(false),
    m_bRaw(false),
    m_szSignalSize(0),
    m_pPropertyTreePrivate(NULL),
    m_pIncidentInvocationHandler(NULL)
{
}

cTransmitter::~cTransmitter()
{
    m_oSerializedSample.reset();
    if (NULL != m_pSendSample.pData)
    {
        ::free(m_pSendSample.pData);
    }
#ifndef NDEBUG
    m_pSendSample.pData= reinterpret_cast<void*>((uintptr_t)0xDEADBEEF);
#endif
    if (NULL != m_pDriverTransmitter)
    {
        m_pDriver->DestroyTransmitter(m_pDriverTransmitter);
    }
}

fep::Result cTransmitter::Create(ITransmissionDriver* pDriver,
    fep::IPropertyTree* pPropertyTreePrivate,
    fep::IIncidentInvocationHandler* pIncidentInvocationHandler,
    const tSignal& oSignal)
{
    if (!pPropertyTreePrivate || !pIncidentInvocationHandler)
    { 
        return ERR_POINTER; 
    }

    fep::Result nResult = ERR_NOERROR;
    m_pDriver = pDriver;
    m_pPropertyTreePrivate = pPropertyTreePrivate;
    m_pIncidentInvocationHandler = pIncidentInvocationHandler;

    m_strSignalName = oSignal.strSignalName;
    m_bDisableDdlSerialization = (oSignal.eSerialization == fep::SER_Raw);
    m_bRaw = oSignal.bIsRaw.GetValue();
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

        m_pSendSample.szSize = sizeof(cFepDataHeader) + m_szSignalSize;
        m_pSendSample.pData = malloc(m_pSendSample.szSize);
        if (NULL == m_pSendSample.pData)
        {
            nResult = ERR_MEMORY;
        }
        m_oSerializedSample.attach(static_cast<char *>(m_pSendSample.pData) + sizeof(cFepDataHeader), m_szSignalSize);

        if (!m_bDisableDdlSerialization)
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
            a_util::memory::unique_ptr<DDLDescription> pDefaultDescription(DDLDescription::createDefault(DDLVersion::getDefaultVersion(), 0));
            DDLImporter oImporter;
            oImporter.setXML(oSignal.strSignalDesc);
            oImporter.createPartial(pDefaultDescription.get(), DDLVersion::getDefaultVersion());
            ddl::DDLDescription* pDescription = oImporter.getDDL();
            DDLPrinter oDDLPrinter;
            oDDLPrinter.visitDDL(pDescription);

            m_oCodecFactory = ddl::CodecFactory(oSignal.strSignalType.c_str(), oDDLPrinter.getXML().c_str());
            nResult = m_oCodecFactory.isValid().getErrorCode();

            oImporter.destroyDDL();
        }
        if (fep::isOk(nResult))
        {
            nResult = m_pDriver->CreateTransmitter(m_pDriverTransmitter, m_oSignalOptions);
        }
    }
    return nResult;
}

fep::Result cTransmitter::TransmitData(IPreparationDataSample const * pSample)
{
    fep::Result nResult = ERR_NOERROR;
    nResult = FillFepDataHeader(pSample);

    a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oSync(m_mtxTransmission);      

    if(false == m_bDisableDdlSerialization)
    {
        ddl::Decoder oDec =
            m_oCodecFactory.makeDecoderFor(pSample->GetPtr(), pSample->GetSize());
        ddl::serialization::transform_to_buffer(oDec, m_oSerializedSample);
    }
    else
    {
        // FillDataHeader already reallocated to correct size if needed
        a_util::memory::copy(static_cast<char*>(m_pSendSample.pData) + sizeof(cFepDataHeader), m_pSendSample.szSize,
            pSample->GetPtr(), pSample->GetSize());
    }

    if(fep::isOk(nResult))
    {
        if (fep::isFailed(m_pDriverTransmitter->Transmit(static_cast<void*>(m_pSendSample.pData),m_pSendSample.szSize)))
        {
            INVOKE_INCIDENT(m_pIncidentInvocationHandler,
                fep::FSI_TRANSM_DATA_TX_FAILED,
                fep::SL_Critical_Local, a_util::strings::format(
                "Failed to write data to bus (Instance %s::%s)",
                GetModuleName(), m_strSignalName.c_str()).c_str());
            nResult = ERR_FAILED;
        }
    }
    return nResult;
}

fep::Result cTransmitter::FillFepDataHeader(IPreparationDataSample const * pSample)
{
    fep::Result nResult = ERR_NOERROR;

    //In case of a signal with ddl the sizes must match
    if((sizeof(cFepDataHeader) + pSample->GetSize() != m_pSendSample.szSize) )
    {
        if(m_bRaw)
        {
            m_pSendSample.szSize = 0;
            ::free(m_pSendSample.pData);
            m_pSendSample.pData = malloc(pSample->GetSize() + sizeof(cFepDataHeader));
            if(NULL == m_pSendSample.pData)
            {
                nResult = ERR_MEMORY;
            }
            else
            {
                m_pSendSample.szSize = pSample->GetSize() + sizeof(cFepDataHeader);
            }
        }
        else
        {
            INVOKE_INCIDENT(m_pIncidentInvocationHandler,
                fep::FSI_TRANSM_TX_WRONG_SAMPLE_SIZE,
                fep::SL_Critical_Local, a_util::strings::format(
                "Failed to transmit sample. Sample size does not match registered signal size. (Instance %s::%s)",
                GetModuleName(), m_strSignalName.c_str()).c_str());

            nResult = ERR_NOT_SUPPORTED;
        }
    }

    if(fep::isOk(nResult))
    {
        //fill header
        cFepDataHeader* pFepDataHeader = reinterpret_cast<cFepDataHeader*>(m_pSendSample.pData);
        pFepDataHeader->m_nMajorVersion = FEP_SDK_PARTICIPANT_VERSION_MAJOR;
        uint8_t nSerAndByteOrderFlags = 0x00;
        if(true == m_bDisableDdlSerialization)
        {
            nSerAndByteOrderFlags |= header::SERIALIZATION_RAW;
        }
        else
        {
            nSerAndByteOrderFlags |= header::SERIALIZATION_DDL;
        }
        nSerAndByteOrderFlags |= header::GetLocalSystemByteorder();

        pFepDataHeader->m_nSerAndByteOrderFlags = nSerAndByteOrderFlags;
        pFepDataHeader->m_nSync= pSample->GetSyncFlag();
        pFepDataHeader->m_nUnused = 0x00;
        pFepDataHeader->m_nSampleNumber= pSample->GetSampleNumberInFrame();
        pFepDataHeader->m_nFrameId= pSample->GetFrameId();
        pFepDataHeader->m_nSendTimeStamp = pSample->GetTime();

    }
    return nResult;
}

fep::Result cTransmitter::Enable()
{
    return m_pDriverTransmitter->Enable();
}

fep::Result cTransmitter::Disable()
{
    return m_pDriverTransmitter->Disable();
}

fep::Result cTransmitter::Mute()
{
    return m_pDriverTransmitter->Mute();
}
fep::Result cTransmitter::Unmute()
{
    return m_pDriverTransmitter->Unmute();
}

const char* cTransmitter::GetModuleName()
{
    const char* strModuleName = NULL;
    m_pPropertyTreePrivate->GetPropertyValue(g_strElementHeaderPath_strElementName, strModuleName);
    return strModuleName;
}

fep::Result cTransmitter::GatherSignalOptions(cSignalOptions & oDriverSignalOptions, const tSignal &oSignal)
{
    fep::Result nResult = ERR_NOERROR;

    // Mandatory Options! (Every Driver must support them)
    // offset in size for data header
    if (!(oDriverSignalOptions.SetOption("SignalName", oSignal.strSignalName)
        && oDriverSignalOptions.SetOption("SignalSize", m_szSignalSize + sizeof(cFepDataHeader))))
    {
        nResult = ERR_FAILED;
    }
    else //These are QoS-Settings (only signal error when they were actively set)
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

        if (!oDriverSignalOptions.SetOption("UseAsyncPublisherMode", oSignal.bRTIAsyncPub.GetValue()))
        {
            if (oSignal.bRTIAsyncPub.IsSet())
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
    }
    return nResult;
}
