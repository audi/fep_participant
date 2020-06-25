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
#pragma once

#include <string>
#include "ReliabilityMode.h"

#include "fastrtps/rtps/writer/WriterListener.h"
#include "fastrtps/rtps/common/CacheChange.h"
#include "fastrtps/rtps/history/WriterHistory.h"

namespace fastrtps_ll_wrap
{
    class Topic;
    class Pafastrtpscipant;
    class Publisher;

    class PublisherSample
    {
        friend class Publisher;

    public:
        PublisherSample(Publisher& publisher, const std::size_t sample_size);
        ~PublisherSample();

    public:
        template <typename T> T* getPtr()
        {
            return reinterpret_cast<T*>(m_data);
        }

    private:
        static eprosima::fastrtps::rtps::CacheChange_t* InternalCreateCacheChange(Publisher& my_publisher, const uint32_t sample_size);
        eprosima::fastrtps::rtps::CacheChange_t* CreateCacheChange(Publisher& my_publisher) const;

    private:
        void* m_data;
        std::size_t m_data_size;
    };

    class Publisher : public eprosima::fastrtps::rtps::WriterListener
    {
        friend class PublisherSample;

    public:
        Publisher(const DdsConfig& dds_config, Participant& my_participant, Topic& my_topic);
        ~Publisher();

    public:
        void Transmit(PublisherSample* data_sample);

    private: // Implements WriterListener
        void onWriterMatched(eprosima::fastrtps::rtps::RTPSWriter* writer, eprosima::fastrtps::rtps::MatchingInfo& info) override;

    private:
        Participant& m_my_participant;
        Topic& m_my_topic;
        //DDSPublisher* m_dds_publisher;
        //DDSDataWriter* m_dds_data_writer;
        //DDSOctetsDataWriter * m_dds_octet_data_writer;
        eprosima::fastrtps::rtps::RTPSWriter* m_rtps_writer;
        eprosima::fastrtps::rtps::WriterHistory* m_writer_history;
        //MatchingFunc m_matching_func;
    };
}
