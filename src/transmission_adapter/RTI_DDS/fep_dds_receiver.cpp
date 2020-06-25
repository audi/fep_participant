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
#include "../src/transmission_adapter/RTI_DDS/fep_dds_receiver.h"
#include "../src/transmission_adapter/RTI_DDS/fep_dds_driver.h"

#include <a_util/process/process.h>
#include <mutex>
#include <a_util/concurrency/fast_mutex.h>
#include <a_util/regex/regularexpression.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_format.h>
#include <a_util/strings/strings_functions.h>

#include "transmission_adapter/RTI_DDS/fep_dds_receiver.h"
#include <ndds_namespace_cpp.h>
#include <dds_c/dds_c_infrastructure.h>
#include <dds_c/dds_c_builtintypes.h>
#include <dds_c/dds_c_string.h>
#include <dds_c/dds_c_subscription.h>
#include <dds_cpp/dds_cpp_domain.h>
#include <dds_cpp/dds_cpp_topic.h>

#include "fep_errors.h"
#include "transmission_adapter/fep_signal_options.h"

class DDSDataReader;

using namespace fep::RTI_DDS;

static const char* strDDSDataType = "DDS::Octets";

cDDSReceive::cDDSReceive() :
    m_pDDSSubscriber(NULL),
    m_pDDSTopic(NULL),
    m_pDDSDataReader(NULL),
    m_pReader(NULL),
    m_bIsInitalized(false),
    m_bIsActivated(false),
    m_bIsMuted(false),
    m_bIsVariableSignalSize(false),
    m_bIsReliable(false),
    m_bUseLowLatProfile(false),
    m_szSignalSize(0),
    m_pDomainParticipant(NULL),
    m_pCallback(NULL),
    m_pCallee(NULL),
    m_szCurrentTotalSize(0),
    m_pLoggingFunc(NULL),
    m_pCalleeLogging(NULL),
    m_bDeactivateFragmentation(false)
{
}

cDDSReceive::~cDDSReceive()
{
    DestroyDDSEntities();
}

fep::Result cDDSReceive::Initialize(cSignalOptions &oOptions, DDSDomainParticipant * &pDomainParticipant, std::string strModuleName)
{
    fep::Result nResult = ERR_INVALID_ARG;
    if (pDomainParticipant)
    {
        nResult = ERR_NOERROR;
        if (oOptions.GetOption("SignalName", m_strSignalName)
            && oOptions.GetOption("SignalSize", m_szSignalSize))
        {
            if (false == oOptions.GetOption("IsReliable", m_bIsReliable))
            {
                m_bIsReliable = false;
            }

            if (false == oOptions.GetOption("IsVariableSignalSize", m_bIsVariableSignalSize))
            {
                m_bIsVariableSignalSize = false;
            }
            if (false == oOptions.GetOption("UseLowLatProfile", m_bUseLowLatProfile))
            {
                m_bUseLowLatProfile = true;
            }
            if (false == oOptions.GetOption("UseMulticast", m_strRTIMulticast))
            {
                m_strRTIMulticast = "";
            }

            m_pDomainParticipant = pDomainParticipant;
            m_strModuleName = strModuleName;

            if (fep::isOk(nResult))
            {
                m_strInstanceName = a_util::strings::format("%s::%s", strModuleName.c_str(),
                    m_strSignalName.c_str());
                nResult = CreateDDSEntities();
            }
        }
    }
    return nResult;
}

fep::Result cDDSReceive::Enable()
{
    std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
    m_bIsActivated = true;
    return ERR_NOERROR;
}

fep::Result cDDSReceive::Disable()
{
    std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
    m_bIsActivated = false;
    return ERR_NOERROR;
}

fep::Result cDDSReceive::Mute()
{
    m_bIsMuted = true;
    return ERR_NOERROR;
}


fep::Result cDDSReceive::Unmute()
{
    m_bIsMuted = false;
    return ERR_NOERROR;
}


fep::Result cDDSReceive::SetReceiver(tCallbackFuncPtr pCallback, void * pCallee)
{
    fep::Result nResult = ERR_POINTER;
    if ((NULL != pCallback && NULL != pCallee)
        || (NULL == pCallback && NULL == pCallee))
    {
        m_pCallback = pCallback;
        m_pCallee = pCallee;
        nResult = ERR_NOERROR;
    }
    return nResult;
}

fep::Result cDDSReceive::CreateDDSEntities()
{
    fep::Result nResult = ERR_NOERROR;
    nResult = CreateSubscriber();
    if (fep::isOk(nResult))
    {
        nResult = RegisterType();
    }
    if (fep::isOk(nResult))
    {
        nResult = CreateTopic();
    }
    if (fep::isOk(nResult))
    {
        nResult = CreateDataReader();
    }
    return nResult;
}

fep::Result cDDSReceive::CreateSubscriber()
{
    DDS::SubscriberQos sSQos;
    m_pDomainParticipant->get_default_subscriber_qos(sSQos);
    sSQos.subscriber_name.name = DDS::String_dup(m_strInstanceName.c_str());
    m_pDDSSubscriber = m_pDomainParticipant->create_subscriber(
        sSQos,
        NULL,
        DDS::STATUS_MASK_NONE);
    if (NULL == m_pDDSSubscriber)
    {
        return ERR_FAILED;
    }
    return ERR_NOERROR;
}

fep::Result cDDSReceive::RegisterType()
{
    fep::Result nResult = ERR_NOERROR;

    if (DDS::RETCODE_OK != DDS::OctetsTypeSupport::register_type(m_pDomainParticipant,
        DDS_DATA_TYPE))
    {
        nResult = ERR_FAILED;
    }

    return nResult;
}

fep::Result cDDSReceive::CreateTopic()
{
    // We must check, whether the topic already exists, i.e. has been created during the
    // initialization of another signal
    DDSTopicDescription * pDescription = m_pDomainParticipant->lookup_topicdescription(m_strSignalName.c_str());
    if (NULL != pDescription)
    {
        // Topic exists. Lets check the type name.
        if (!a_util::strings::isEqual(pDescription->get_type_name(), strDDSDataType))
        {
            //Error while creating topic: topic exists with different type
            return ERR_FAILED;
        }
        m_pDDSTopic = DDSTopic::narrow(pDescription);
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
        else if(a_util::process::getEnvVar("FEP3_RTI_DDS", "NO") != "NO")
        {
            DDS::TopicQos sTopicQos;
            m_pDomainParticipant->get_default_topic_qos(sTopicQos);
            m_pDDSTopic = m_pDomainParticipant->create_topic(m_strSignalNameWithoutParameter.c_str(),
                DDS_DATA_TYPE,
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
                DDS_DATA_TYPE,
                sTopicQos,
                NULL,
                DDS::STATUS_MASK_NONE);
            m_bDeactivateFragmentation = false;
        }

    }

    

    // However we got the topic, it should be valid by now
    if (NULL == m_pDDSTopic)
    {
        return ERR_FAILED;
    }
    return ERR_NOERROR;
}

fep::Result cDDSReceive::CreateDataReader()
{
    std::string m_strSignalNameWithoutParameter;
    auto parameter = cDDSDriver::GetUrlParamter(m_strSignalName, m_strSignalNameWithoutParameter);

    if (parameter.count("profile") != 0 &&
        parameter.count("library") != 0)
    {
        m_bDeactivateFragmentation = true;

        m_pDDSDataReader = m_pDDSSubscriber->create_datareader_with_profile(m_pDDSTopic,
            parameter["library"].c_str(),
            parameter["profile"].c_str(),
            this,
            DDS_DATA_AVAILABLE_STATUS | DDS_SUBSCRIPTION_MATCHED_STATUS);

    }
    else if (a_util::process::getEnvVar("FEP3_RTI_DDS", "NO") != "NO")
    {
        m_bDeactivateFragmentation = true;

        DDS::DataReaderQos sDataReaderQos;
        m_pDDSSubscriber->get_default_datareader_qos(sDataReaderQos);
        m_pDDSDataReader = m_pDDSSubscriber->create_datareader(
            m_pDDSTopic,
            sDataReaderQos,
            this,
            DDS_DATA_AVAILABLE_STATUS | DDS_SUBSCRIPTION_MATCHED_STATUS);
    }
    else
    {
        m_bDeactivateFragmentation = false;

        DDS::DataReaderQos sDataReaderQos;
        m_pDDSSubscriber->get_default_datareader_qos(sDataReaderQos);
        fep::Result nResult = ModifyDataReaderQos(sDataReaderQos);
        if (isFailed(nResult))
        {
            return nResult;
        }
        m_pDDSDataReader = m_pDDSSubscriber->create_datareader(
            m_pDDSTopic,
            sDataReaderQos,
            this,
            DDS_DATA_AVAILABLE_STATUS | DDS_SUBSCRIPTION_MATCHED_STATUS);
    }

    if (NULL == m_pDDSDataReader)
    {
        //Could not create reader on DDS bus
        return ERR_FAILED;
    }
    m_pReader = DDS::OctetsDataReader::narrow(m_pDDSDataReader);
    if (NULL == m_pReader)
    {
        //Could not narrow writer on DDS bus
        return ERR_FAILED;
    }

    return ERR_NOERROR;
}

fep::Result cDDSReceive::ModifyDataReaderQos(DDS::DataReaderQos & sQos)
{
    fep::Result nResult = ERR_NOERROR;
    if (!m_strRTIMulticast.empty())
    {
        tMCIP cMCIp = ParseIpAddressString(m_strRTIMulticast);
        if (cMCIp.bValid)
        {
            // Setup to use multicast
            sQos.multicast.kind = DDS_AUTOMATIC_TRANSPORT_MULTICAST_QOS;

            DDS_TransportMulticastSettingsSeq_initialize(&sQos.multicast.value);
            DDS_TransportMulticastSettingsSeq_ensure_length(&sQos.multicast.value, 1, 1);

            DDS_TransportMulticastSettings_t* multicast_settings_seq = DDS_TransportMulticastSettingsSeq_get_reference(&sQos.multicast.value, 0);
            multicast_settings_seq->receive_address = DDS_String_dup(cMCIp.strAddress.c_str());
            multicast_settings_seq->receive_port = cMCIp.dPort;
        }
        else
        {
            nResult = ERR_INVALID_ARG;
        }
    }

    if (m_bIsReliable)
    {
        // Reliable as defined in DefaultLibrary: connext_dds/qos/reliable.xml : Reliable
        sQos.history.kind = DDS_KEEP_ALL_HISTORY_QOS;
        sQos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        sQos.protocol.rtps_reliable_reader.min_heartbeat_response_delay = DDS_Duration_t::from_nanos(0);
        sQos.protocol.rtps_reliable_reader.max_heartbeat_response_delay = DDS_Duration_t::from_nanos(0);

        if (m_bUseLowLatProfile)
        {
            // Reliable as defined in DefaultLibrary: connext_dds/qos/low_latency.xml : LowLatency
            // ... only applied if Reliable as LowLatency is based on Reliable
            sQos.resource_limits.max_samples = 100;
            sQos.resource_limits.initial_samples = 100;
            sQos.protocol.rtps_reliable_reader.heartbeat_suppression_duration = DDS_Duration_t::from_nanos(0);
        }
    }

    return nResult;
}

fep::Result cDDSReceive::ModifyTopicQos(DDS::TopicQos & sQos)
{
    return ERR_NOERROR;
}

fep::Result cDDSReceive::DestroyDDSEntities()
{
    // just to be safe that we wont receive anything anymore
    {
        std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
        m_bIsActivated = false;
    }
    if (NULL != m_pDDSSubscriber)
    {
        if (DDS::RETCODE_OK != m_pDDSSubscriber->delete_contained_entities())
        {
            //there is nothing we can do here...sorry!
            LogMessage(a_util::strings::format("%: Failed to delete dds entities contained inside subscriber.",
                m_strSignalName.c_str()).c_str(), fep::SL_Critical_Local);
        }
    }

    if (NULL != m_pDomainParticipant)
    {
        if (DDS::RETCODE_OK != m_pDomainParticipant->delete_subscriber(m_pDDSSubscriber))
        {
            //there is nothing we can do here...sorry!
            LogMessage(a_util::strings::format("%: Failed to delete subscriber dds entities.",
                m_strSignalName.c_str()).c_str(), fep::SL_Critical_Local);
        }
        // We will NOT delete the topic here, since it might be used by other signals here.
        // The topic will be deleted when m_pDDSDomainParticipant is cleaned up outside.
    }

    m_pDDSDataReader = NULL;
    m_pReader = NULL;
    m_pDDSTopic = NULL;
    m_pDDSSubscriber = NULL;
    m_pDomainParticipant = NULL;
    return ERR_NOERROR;
}

void cDDSReceive::on_data_available(DDSDataReader *reader)
{
    tDataItem_Struct pDataItem;
    DDS::ReturnCode_t nReturnCode;
    nReturnCode = m_pReader->take(pDataItem.oDataSeq,
        pDataItem.oInfoSeq,
        DDS_LENGTH_UNLIMITED,
        DDS::ANY_SAMPLE_STATE,
        DDS::ANY_VIEW_STATE,
        DDS::ANY_INSTANCE_STATE);
    if (nReturnCode == DDS::RETCODE_OK)
    {
        std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
        if (!m_bIsActivated)
        {
            // wrong state --> report and return loan
            std::string strMsg = a_util::strings::format("%s : Received data while not enabled - "
                "dropping packet.", m_strSignalName.c_str());
            LogMessage(strMsg.c_str(), fep::SL_Warning);
            m_pReader->return_loan(pDataItem.oDataSeq, pDataItem.oInfoSeq);
        }
        else if (m_bIsMuted)
        {
            // reception is muted --> just return the loan
            m_pReader->return_loan(pDataItem.oDataSeq, pDataItem.oInfoSeq);
        }
        else
        {
            for (int32_t i = 0; i < pDataItem.oInfoSeq.length(); ++i)
            {
                if ((pDataItem.oInfoSeq[i]).valid_data)
                {

                    if (NULL != m_pCallback && NULL != m_pCallee)
                    {
                        if (m_bDeactivateFragmentation)
                        {
                            m_pCallback(m_pCallee, (pDataItem.oDataSeq[i]).value, (pDataItem.oDataSeq[i]).length);
                        }
                        else
                        {
                            uint64_t lost_samples = receiveFragment((pDataItem.oDataSeq[i]).value, (pDataItem.oDataSeq[i]).length);

#ifdef DEBUG
                            if (lost_samples > 0)
                            {
                                std::string strMsg = a_util::strings::format("%s : Lost %lu sample(s) due to fragment packet loss",
                                    m_strSignalName.c_str(), lost_samples);
                                LogMessage(strMsg.c_str(), fep::SL_Warning);
                            }
#else
                            (void)lost_samples;
#endif
                        }
                    }

                }
            }
            m_pReader->return_loan(pDataItem.oDataSeq, pDataItem.oInfoSeq);
        }
    }
    else
    {
        // could not take samples --> nothing to do here
    }
}

void cDDSReceive::receiveSample(const void* sample, uint32_t length) noexcept
{
    m_pCallback(m_pCallee, sample, length);
}

fep::Result cDDSReceive::RegisterLogging(ITransmissionDriver::tLoggingFuncPtr pLoggingFunc, void * pCallee)
{
    fep::Result nResult = ERR_INVALID_ARG;
    if (NULL != pLoggingFunc && NULL != pCallee)
    {
        m_pCalleeLogging = pCallee;
        m_pLoggingFunc = pLoggingFunc;
        nResult = ERR_NOERROR;
    }

    return nResult;
}

void cDDSReceive::LogMessage(const char* strMessage, fep::tSeverityLevel eServLevel)
{
    if (NULL != m_pLoggingFunc && NULL != m_pCalleeLogging)
    {
        m_pLoggingFunc(m_pCalleeLogging, strMessage, eServLevel);
    }
}

cDDSReceive::tMCIP cDDSReceive::ParseIpAddressString(const std::string &strMulticastIpString)
{
    tMCIP cMulticastIp{ "", 0, false };

    a_util::regex::RegularExpression regexIpAddrOnlyPattern("^(239\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})$");
    a_util::regex::RegularExpression regexIpAddrPortPattern("^(239\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})\\:(\\d{1,5})$");

    std::string arg1;
    std::string arg2;
    if (regexIpAddrPortPattern.fullMatch(strMulticastIpString, arg1, arg2))
    {
        cMulticastIp.strAddress = arg1;
        cMulticastIp.dPort = a_util::strings::toUInt16(arg2);
        cMulticastIp.bValid = true;
    }
    else if (regexIpAddrOnlyPattern.fullMatch(strMulticastIpString, arg1))
    {
        cMulticastIp.strAddress = arg1;
        cMulticastIp.dPort = 0;
        cMulticastIp.bValid = true;
    }
    else
    {
        //Nothing to do here.
    }

    return cMulticastIp;
}

