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
#include "fastrtps_ll_wrap.h"

#include "fastrtps/rtps/writer/RTPSWriter.h"
#include "fastrtps/rtps/attributes/WriterAttributes.h"
#include "fastrtps/attributes/TopicAttributes.h"
#include "fastrtps/rtps/RTPSDomain.h"
#include "fastrtps/qos/WriterQos.h"
#include "fastrtps/rtps/participant/RTPSParticipant.h"

#include <assert.h>

using namespace fastrtps_ll_wrap;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps;

PublisherSample::PublisherSample(Publisher& publisher,  const std::size_t sample_size)
    : m_data_size(sample_size)
{
    m_data = ::malloc(m_data_size);
}

PublisherSample::~PublisherSample()
{
    ::free(m_data);
}

CacheChange_t* PublisherSample::InternalCreateCacheChange(Publisher& my_publisher, const uint32_t sample_size)
{
    my_publisher.m_rtps_writer->remove_older_changes(/*1*/);
    eprosima::fastrtps::rtps::CacheChange_t* cache_change= my_publisher.m_rtps_writer->new_change([sample_size]() -> uint32_t { return sample_size; }, ALIVE);
    assert(cache_change);
    cache_change->serializedPayload.length = sample_size;

    return cache_change;
}

CacheChange_t* PublisherSample::CreateCacheChange(Publisher& my_publisher) const
{
    CacheChange_t* cache_change = InternalCreateCacheChange(my_publisher, static_cast<const uint32_t>(m_data_size));

    ::memcpy(cache_change->serializedPayload.data, m_data, m_data_size);

    return cache_change;
}


Publisher::Publisher(const DdsConfig& dds_config, Participant& my_participant, Topic& my_topic)
    : m_my_participant(my_participant)
    , m_my_topic(my_topic)
    , m_rtps_writer(nullptr)
    , m_writer_history(nullptr)
{
    //CREATE WRITERHISTORY
    HistoryAttributes history_attributes;
    history_attributes.payloadMaxSize = static_cast<uint32_t>(dds_config.sample_size); // MAX_PAYLOAD_SIZE;
    //history_attributes.memoryPolicy = PREALLOCATED_MEMORY_MODE;
    //history_attributes.initialReservedCaches = 4; // 16; // 500;
    //history_attributes.maximumReservedCaches = 4; // 16; //  500;
    m_writer_history = new WriterHistory(history_attributes);

    //CREATE WRITER
    WriterAttributes writer_attributes;

    if (dds_config.multicast_enable)
    {
        eprosima::fastrtps::rtps::Locator_t locator;
        locator.set_IP4_address(239, 255, 0, 1);
        locator.port = 22222;
        writer_attributes.endpoint.multicastLocatorList.push_back(locator);
    }

    switch (dds_config.reliability_mode)
    {
    case BestEffortReliabilityMode:
        writer_attributes.endpoint.reliabilityKind = BEST_EFFORT;
        break;
    case ReliableReliabilityMode:
        writer_attributes.endpoint.reliabilityKind = RELIABLE;
        break;
    }
    
    writer_attributes.endpoint.durabilityKind = VOLATILE; // TRANSIENT_LOCAL;

#if 0
    writer_attributes.times.heartbeatPeriod = Duration_t(0, 0);
    writer_attributes.times.initialHeartbeatDelay = Duration_t(0, 0);
    writer_attributes.times.nackResponseDelay = Duration_t(0, 0);
    writer_attributes.times.nackSupressionDuration = Duration_t(0, 0);

    writer_attributes.disableHeartbeatPiggyback = false; // true;
#endif

    m_rtps_writer = RTPSDomain::createRTPSWriter(m_my_participant.m_rtps_participant, writer_attributes, m_writer_history, this);
    assert(m_rtps_writer);

    // REGISTER WRITER
    WriterQos writer_qos;
    switch (dds_config.reliability_mode)
    {
    case BestEffortReliabilityMode:
        writer_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
        break;
    case ReliableReliabilityMode:
        writer_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        break;
    }
 
    writer_qos.m_durability.kind = VOLATILE_DURABILITY_QOS; // TRANSIENT_LOCAL_DURABILITY_QOS; // VOLATILE_DURABILITY_QOS;

    if (dds_config.async_mode_enable)
    {
        writer_qos.m_publishMode.kind = ASYNCHRONOUS_PUBLISH_MODE;
    }

    bool res = m_my_participant.m_rtps_participant->registerWriter(m_rtps_writer, my_topic.m_topic_attributes, writer_qos);
    assert(res == true);
}

Publisher::~Publisher()
{
    RTPSDomain::removeRTPSWriter(m_rtps_writer);
}

void Publisher::Transmit(PublisherSample* data_sample)
{
    //DDS_Octets* send_octet= DDSOctetsTypeSupport::create_data(data_sample->m_dds_octet->length);

    // FIXME: Need to copy
    //send_octet->length = data_sample->m_dds_octet->length;

    //m_dds_octet_data_writer->write(*send_octet, DDS_HANDLE_NIL);
    //m_dds_octet_data_writer->write(*(data_sample->m_dds_octet), DDS_HANDLE_NIL);

    bool res = m_writer_history->add_change(data_sample->CreateCacheChange(*this));
    assert(res);
}

void Publisher::onWriterMatched(eprosima::fastrtps::rtps::RTPSWriter* writer, eprosima::fastrtps::rtps::MatchingInfo& info)
{
    // Ignore ... nothing to do
}
