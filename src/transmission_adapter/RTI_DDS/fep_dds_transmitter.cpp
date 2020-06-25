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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <a_util/process/process.h>
#include <a_util/concurrency/fast_mutex.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>
#include <a_util/strings/strings_functions.h>
#include "../src/transmission_adapter/RTI_DDS/fep_dds_driver.h"

#include "transmission_adapter/RTI_DDS/fep_dds_transmitter.h"
#include "transmission_adapter/RTI_DDS/fep_dds_header.h"
#include <ndds_namespace_cpp.h>
#include <dds_c/dds_c_builtintypes.h>
#include <dds_c/dds_c_infrastructure.h>
#include <dds_cpp/dds_cpp_domain.h>
#include <dds_cpp/dds_cpp_topic.h>

#include "fep_errors.h"
#include "fep_result_decl.h"
#include "incident_handler/fep_severity_level.h"
#include "transmission_adapter/fep_signal_options.h"
#include "transmission_adapter/fep_transmission_driver_intf.h"

using namespace fep::RTI_DDS;

static const char* strDDSDataType =  "DDS::Octets";

cDDSTransmit::cDDSTransmit(uint64_t nSenderID) : 
    m_bIsActivated(false),
    m_bIsMuted(false),
    m_bIsVariableSignalSize(false),
    m_bIsReliable(false),
    m_bUseLowLatProfile(false),
    m_bUseAsyncPubliser(false),
    m_szSignalSize(0),
    m_pDomainParticipant(NULL),
    m_pDDSPublisher(NULL),
    m_pDDSTopic(NULL),
    m_pDDSDataWriter(NULL),
    m_pWriter(NULL),
    m_pLoggingFunc(NULL),
    m_pCalleeLogging(NULL),
    m_oOutput_buffer(DDS_MAX_PACKET_SIZE),
    m_bDeactivateFragmentation(false),
    Fragmenter(nSenderID)
{
    setBuffer(m_oOutput_buffer.value);
}

cDDSTransmit::~cDDSTransmit()
{
    Destroy();
}

fep::Result cDDSTransmit::Transmit(const void *pData, size_t szSize)
{
    std::unique_lock<a_util::concurrency::fast_mutex> oSync(m_mtxTransmission);

    fep::Result nResult = ERR_NOERROR;
    std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
    if(!m_bIsActivated)
    {
        LogMessage(a_util::strings::format("%: Transmission failure - transmitter is not enabled.",
            m_strSignalName.c_str()).c_str(), fep::SL_Warning);
        nResult = ERR_INVALID_STATE;
    }
    else if(NULL == pData || 0 >= szSize)
    {
        nResult = ERR_INVALID_ARG;
    }
    else if(!m_bIsVariableSignalSize && szSize != m_szSignalSize)
    {
        nResult= ERR_INVALID_TYPE;
    }
    else if (m_bIsMuted)
    {
        // we do nothing
    }
    else
    {
        if (m_bDeactivateFragmentation)
        {
            assert(m_pWriter);
            m_pWriter->write((const unsigned char*)pData, static_cast<int>(szSize), DDS_HANDLE_NIL) == DDS::RETCODE_OK;

            if (m_strSignalName == "command")
            {
                DDSTheParticipantFactory->unregister_thread();
            }
        }
        else
        {
            if (transmitSample(pData, static_cast<uint32_t>(szSize)) == 0)
            {
                nResult = fep::ERR_FAILED;
            }
            // for performance reasons
            if (m_strSignalName == "command")
            {
                DDSTheParticipantFactory->unregister_thread();
            }
        }
        
    }

    return nResult;
}

bool cDDSTransmit::transmitFragment(void* fragment, uint32_t length) noexcept
{
    assert(m_pWriter);
    assert(fragment == m_oOutput_buffer.value);
    m_oOutput_buffer.length = length;
    return m_pWriter->write(m_oOutput_buffer, DDS_HANDLE_NIL) == DDS::RETCODE_OK;
}

fep::Result cDDSTransmit::Enable()
{
    std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
    m_bIsActivated = true;
    return ERR_NOERROR;
}

fep::Result cDDSTransmit::Disable()
{
    std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
    m_bIsActivated = false;
    return ERR_NOERROR;
}

fep::Result cDDSTransmit::Mute()
{
    m_bIsMuted = true;
    return ERR_NOERROR;
}

fep::Result cDDSTransmit::Unmute()
{
    m_bIsMuted = false;
    return ERR_NOERROR;
}

fep::Result cDDSTransmit::Initialize(cSignalOptions oOptions, DDSDomainParticipant * &pDomainParticipant, std::string strModuleName)
{
    fep::Result nResult = ERR_INVALID_ARG;
    if(NULL != pDomainParticipant && !strModuleName.empty())
    {
        if(oOptions.GetOption("SignalName", m_strSignalName)
            && oOptions.GetOption("SignalSize", m_szSignalSize))
        {
            if(false == oOptions.GetOption("IsReliable", m_bIsReliable))
            {
                m_bIsReliable = false;
            }

            if(false == oOptions.GetOption("IsVariableSignalSize", m_bIsVariableSignalSize))
            {
                m_bIsVariableSignalSize = false;
            }
            if (false == oOptions.GetOption("UseAsyncPublisherMode", m_bUseAsyncPubliser))
            {
                m_bUseAsyncPubliser = false;
            }
            if (false == oOptions.GetOption("UseLowLatProfile", m_bUseLowLatProfile))
            {
                m_bUseLowLatProfile = true;
            }

            m_strModuleName = strModuleName;
            m_pDomainParticipant = pDomainParticipant;
            m_strInstanceName = a_util::strings::format("%s::%s", m_strModuleName.c_str(), m_strSignalName.c_str());
            nResult = CreateDDSEntities();
        }
    }
    return nResult;
}

fep::Result cDDSTransmit::CreateDDSEntities()
{
    fep::Result nResult = ERR_NOERROR;
    nResult = CreatePublisher();
    if(fep::isOk(nResult))
    {
        nResult = RegisterType();
    }
    if(fep::isOk(nResult))
    {
        nResult = CreateTopic();
    }
    if(fep::isOk(nResult))
    {
        nResult = CreateDataWriter(); 
    }
    if(fep::isOk(nResult))
    {
        std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
        m_bIsActivated = true;
    }
    return nResult;
}

fep::Result cDDSTransmit::CreatePublisher()
{   
    fep::Result nResult = ERR_NOERROR;
    DDS::PublisherQos sPQos;
    m_pDomainParticipant->get_default_publisher_qos(sPQos);
    sPQos.publisher_name.name = DDS::String_dup(m_strInstanceName.c_str());

    m_pDDSPublisher = m_pDomainParticipant->create_publisher(
        sPQos,
        NULL,
        DDS::STATUS_MASK_NONE);
    if (NULL == m_pDDSPublisher)
    {
        nResult = ERR_FAILED;
    }
    return nResult;
}

fep::Result cDDSTransmit::RegisterType()
{
    fep::Result nResult = ERR_NOERROR;
    if (DDS::RETCODE_OK != DDS::OctetsTypeSupport::register_type(
        m_pDomainParticipant, strDDSDataType))
    {
        nResult = ERR_FAILED;
    }
    return nResult;
}

fep::Result cDDSTransmit::CreateTopic()
{
    fep::Result nResult = ERR_NOERROR;
    // We must check, whether the topic already exists, i.e. has been created during the
    // initialization of another signal
    DDSTopicDescription * pDescription = 
        m_pDomainParticipant->lookup_topicdescription(m_strSignalName.c_str());

    if (NULL != pDescription)
    {
        // Topic exists. Lets check the type name.
        if (!a_util::strings::isEqual(pDescription->get_type_name(), strDDSDataType))
        {
            nResult = ERR_FAILED;
        }
        else
        {
            m_pDDSTopic = DDSTopic::narrow(pDescription);
        }
    }
    else
    {
        std::string m_strSignalNameWithoutParameter;
        auto parameter = cDDSDriver::GetUrlParamter(m_strSignalName, m_strSignalNameWithoutParameter);
        if (parameter.count("profile") != 0 &&
            parameter.count("library") != 0)
        {
            m_pDDSTopic = m_pDomainParticipant->create_topic_with_profile(m_strSignalNameWithoutParameter.c_str(),
                strDDSDataType,
                parameter["library"].c_str(),
                parameter["profile"].c_str(),
                NULL,
                DDS::STATUS_MASK_NONE);
            m_bDeactivateFragmentation = true;
        }
        else if (a_util::process::getEnvVar("FEP3_RTI_DDS", "NO") != "NO")
        {
            DDS::TopicQos sTopicQos;
            m_pDomainParticipant->get_default_topic_qos(sTopicQos);
            m_pDDSTopic = m_pDomainParticipant->create_topic(m_strSignalNameWithoutParameter.c_str(),
                strDDSDataType,
                sTopicQos,
                NULL,
                DDS::STATUS_MASK_NONE);
            m_bDeactivateFragmentation = true;
        }
        else
        {
            DDS::TopicQos sTopicQos;
            m_pDomainParticipant->get_default_topic_qos(sTopicQos);

            ModifyTopicQos(sTopicQos);
            m_pDDSTopic = m_pDomainParticipant->create_topic(m_strSignalNameWithoutParameter.c_str(),
                strDDSDataType,
                sTopicQos,
                NULL,
                DDS::STATUS_MASK_NONE);
            m_bDeactivateFragmentation = false;
        }
    }
    // However we got the topic, it should be valid by now
    if (NULL == m_pDDSTopic)
    {
        nResult = ERR_FAILED;
    }
    return nResult;
}

fep::Result cDDSTransmit::CreateDataWriter()
{
    fep::Result nResult = ERR_NOERROR;

    std::string m_strSignalNameWithoutParameter;
    auto parameter = cDDSDriver::GetUrlParamter(m_strSignalName, m_strSignalNameWithoutParameter);
       
    if (parameter.count("profile") != 0 &&
        parameter.count("library") != 0)
    {
        m_pDDSDataWriter = m_pDDSPublisher->create_datawriter_with_profile(m_pDDSTopic,
            parameter["library"].c_str(),
            parameter["profile"].c_str(),
            NULL,
            DDS_STATUS_MASK_NONE);
        m_bDeactivateFragmentation = true;
    }
    else if (a_util::process::getEnvVar("FEP3_RTI_DDS", "NO") != "NO")
    {
        DDS::DataWriterQos sDataWriterQos;
        m_pDDSPublisher->get_default_datawriter_qos(sDataWriterQos);
        m_pDDSDataWriter = m_pDDSPublisher->create_datawriter(
            m_pDDSTopic,
            sDataWriterQos,
            NULL,
            DDS_STATUS_MASK_NONE);
        m_bDeactivateFragmentation = true;
    }
    else
    {
        DDS::DataWriterQos sDataWriterQos;
        m_pDDSPublisher->get_default_datawriter_qos(sDataWriterQos);
        ModifyDataWriterQos(sDataWriterQos);
        m_pDDSDataWriter = m_pDDSPublisher->create_datawriter(
            m_pDDSTopic,
            sDataWriterQos,
            NULL,
            DDS_STATUS_MASK_NONE);
        m_bDeactivateFragmentation = false;
    }

    if (NULL == m_pDDSDataWriter)
    {
        nResult = ERR_FAILED;
    }
    else
    {
        m_pWriter = DDS::OctetsDataWriter::narrow(m_pDDSDataWriter);
    }
    if (NULL == m_pWriter)
    {
        nResult = ERR_FAILED;
    }

    return nResult;
}

fep::Result cDDSTransmit::ModifyDataWriterQos(DDS::DataWriterQos & sQos)
{
    fep::Result nResult = ERR_NOERROR;

    DDS::PropertyQosPolicyHelper::add_property(sQos.property,
        "dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size", "32768", true);

    if (m_bUseAsyncPubliser)
    {
        sQos.publish_mode.kind = DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS;
    }

    if(m_bIsReliable)
    {
        // Reliable as defined in DefaultLibrary: connext_dds/qos/reliable.xml : Reliable
        sQos.history.kind = DDS_KEEP_ALL_HISTORY_QOS;
        sQos.reliability.kind= DDS_RELIABLE_RELIABILITY_QOS;
        sQos.reliability.max_blocking_time = DDS_Duration_t::from_seconds(5);
        sQos.resource_limits.max_samples = 32;
        sQos.protocol.rtps_reliable_writer.low_watermark = 5;
        sQos.protocol.rtps_reliable_writer.high_watermark = 15;
        sQos.protocol.rtps_reliable_writer.heartbeat_period = DDS_Duration_t::from_nanos(100000000); // 100 ms
        sQos.protocol.rtps_reliable_writer.fast_heartbeat_period = DDS_Duration_t::from_nanos(10000000); // 10 ms
        sQos.protocol.rtps_reliable_writer.late_joiner_heartbeat_period = DDS_Duration_t::from_nanos(10000000);// 10 ms
        sQos.protocol.rtps_reliable_writer.max_heartbeat_retries = 500;
        sQos.protocol.rtps_reliable_writer.min_nack_response_delay = DDS_Duration_t::from_nanos(0);
        sQos.protocol.rtps_reliable_writer.max_nack_response_delay = DDS_Duration_t::from_nanos(0);
        sQos.protocol.rtps_reliable_writer.min_send_window_size = 32;
        sQos.protocol.rtps_reliable_writer.max_send_window_size = 32;

        if (m_bUseLowLatProfile)
        {
            // Reliable as defined in DefaultLibrary: connext_dds/qos/low_latency.xml : LowLatency
            // ... only applied if Reliable as LowLatency is based on Reliable
            sQos.resource_limits.max_samples = DDS_LENGTH_UNLIMITED;
            sQos.resource_limits.initial_samples = 100;
            sQos.protocol.rtps_reliable_writer.low_watermark = 10;
            sQos.protocol.rtps_reliable_writer.high_watermark = 100;
            sQos.protocol.rtps_reliable_writer.heartbeats_per_max_samples = 1000;
            sQos.protocol.rtps_reliable_writer.heartbeat_period = DDS_Duration_t::from_nanos(10000000); // 10 ms
            sQos.protocol.rtps_reliable_writer.fast_heartbeat_period = DDS_Duration_t::from_nanos(1000000); // 1 ms
        }
    }

    return nResult;
}

fep::Result cDDSTransmit::ModifyTopicQos(DDS::TopicQos & sQos)
{
    return ERR_NOERROR;
}

fep::Result cDDSTransmit::Destroy()
{
    // wait for any transmissions to return
    std::unique_lock<a_util::concurrency::fast_mutex> oSync(m_mtxTransmission);

    DestroyDDSEntities();

    return ERR_NOERROR;
}

fep::Result cDDSTransmit::DestroyDDSEntities()
{
    {
        std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
        m_bIsActivated = false;
    }

    if (NULL != m_pDDSPublisher)
    {
        if (DDS::RETCODE_OK != m_pDDSPublisher->delete_contained_entities())
        {
            //COULD NOT DELETE ENTITIES!
            //there is nothing we can do here...sorry!

            LogMessage(a_util::strings::format("%: Failed to delete dds entities contained inside publisher.",
                m_strSignalName.c_str()).c_str(), fep::SL_Critical_Local);
        }
    }

    if (NULL != m_pDomainParticipant)
    {

        if (DDS::RETCODE_OK != m_pDomainParticipant->delete_publisher(m_pDDSPublisher))
        {
            //COULD NOT DELETE ENTITIES!
            //there is nothing we can do here...sorry!

            LogMessage(a_util::strings::format("%: Failed to delete publisher dds entities.",
                m_strSignalName.c_str()).c_str(), fep::SL_Critical_Local);
        }

        // We will NOT delete the topic here, since it might be used by other signals here.
        // The topic will be deleted when m_pDDSDomainParticipant is cleaned up outside.
    }

    m_pDDSDataWriter = NULL;
    m_pWriter = NULL;
    m_pDDSTopic = NULL;
    m_pDDSPublisher = NULL;
    m_pDomainParticipant = NULL;
    return ERR_NOERROR;
}

fep::Result cDDSTransmit::RegisterLogging(ITransmissionDriver::tLoggingFuncPtr pLoggingFunc, void * pCallee)
{
    fep::Result nResult = ERR_INVALID_ARG;
    if( NULL != pLoggingFunc && NULL != pCallee)
    {
        m_pCalleeLogging = pCallee;
        m_pLoggingFunc = pLoggingFunc;
        nResult = ERR_NOERROR;
    }

    return nResult;
}

void cDDSTransmit::LogMessage(const char* strMessage, fep::tSeverityLevel eServLevel)
{
    if( NULL != m_pLoggingFunc && NULL != m_pCalleeLogging)
    {
        m_pLoggingFunc(m_pCalleeLogging, strMessage, eServLevel);
    }
}
