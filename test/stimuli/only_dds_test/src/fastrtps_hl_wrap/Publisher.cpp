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
#include "fastrtps_hl_wrap.h"

#include "fastrtps/rtps/writer/RTPSWriter.h"
#include "fastrtps/rtps/attributes/WriterAttributes.h"
#include "fastrtps/attributes/PublisherAttributes.h"
#include "fastrtps/attributes/TopicAttributes.h"
#include "fastrtps/rtps/RTPSDomain.h"
#include "fastrtps/qos/WriterQos.h"
#include "fastrtps/rtps/participant/RTPSParticipant.h"
#include "fastrtps/Domain.h"

#include <assert.h>

using namespace fastrtps_hl_wrap;
//using namespace eprosima::fastrtps::rtps;
//using namespace eprosima::fastrtps;

PublisherSample::PublisherSample(fastrtps_hl_wrap::Publisher& publisher, const std::size_t sample_size)
    : m_octet_type()
{
    m_octet_type.data().resize(sample_size);
}

PublisherSample::~PublisherSample()
{
}

fastrtps_hl_wrap::Publisher::Publisher(const DdsConfig& dds_config, fastrtps_hl_wrap::Participant& my_participant, Topic& my_topic)
    : m_my_participant(my_participant)
    , m_my_topic(my_topic)
    , m_fastrtps_publisher(nullptr)
{
    //CREATE WRITER
    eprosima::fastrtps::PublisherAttributes publisher_attributes;
    switch (dds_config.reliability_mode)
    {
    case BestEffortReliabilityMode:
        publisher_attributes.qos.m_reliability.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
        break;
    case ReliableReliabilityMode:
        publisher_attributes.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
        break;
    }

    if (dds_config.async_mode_enable)
    {
        publisher_attributes.qos.m_publishMode.kind = ASYNCHRONOUS_PUBLISH_MODE;
    }

    publisher_attributes.topic = my_topic.m_topic_attributes;
    
    m_fastrtps_publisher = eprosima::fastrtps::Domain::createPublisher(m_my_participant.m_fastrtps_participant, publisher_attributes, this);
    assert(m_fastrtps_publisher);
}

fastrtps_hl_wrap::Publisher::~Publisher()
{
    Domain::removePublisher(m_fastrtps_publisher);
}

void fastrtps_hl_wrap::Publisher::Transmit(PublisherSample* data_sample)
{
    bool res = m_fastrtps_publisher->write(reinterpret_cast<void*>(&(data_sample->m_octet_type)));
    assert(res);
}

