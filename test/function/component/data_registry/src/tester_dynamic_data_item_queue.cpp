/**
* Tests for DynamicDataReaderQueue.
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

#include <gtest/gtest.h>

#include <fep_participant_sdk.h>
#include <fep3/components/data_registry/dynamic_data_item_queue.h>
#include <fep3/base/streamtype/default_streamtype.h>
#include "./../../helper/wait_for_data.hpp"
#include "./test_helper.h"

using namespace fep::detail;

struct DataItemReceiver : public DynamicDataItemQueue<const IDataRegistry::IDataSample>::IDataItemReceiver
{
    DataItemReceiver(data_read_ptr<const IDataRegistry::IDataSample>& sample) : _sample(sample)
    {
    }
    void onReceive(const data_read_ptr<const IDataRegistry::IDataSample>& sample) override
    {
        _sample = sample;
    }
    void onReceive(const data_read_ptr<const IStreamType>& stream_type) override
    {
    }

public:
    data_read_ptr <const IDataRegistry::IDataSample>& _sample;
};

/**
 * @req_id ""
 */
TEST(DynamicDataItemQueueTest, pushPopSampleDynamicDataItemQueue)
{
    timestamp_t current_top_time{ 0 };
    size_t current_queue_size{ 0 }, repetitions{ 10 };
    DynamicDataItemQueue<> dynamic_data_item_queue(0);
    ASSERT_EQ(std::make_pair(current_queue_size, INVALID_timestamp_t_fep), std::make_pair(dynamic_data_item_queue.size(), dynamic_data_item_queue.topTime()));

    // Push data samples to the queue and check for:
    // * queue size increases
    // * no samples are being dropped
    // * top time does not change
    for (int i = 0; i < repetitions; i++)
    {
        data_read_ptr<const IDataRegistry::IDataSample> sample_to_push = createTestSample(i, i);
        dynamic_data_item_queue.push(sample_to_push, i);
        ASSERT_EQ(std::make_pair(++current_queue_size, current_top_time), std::make_pair(dynamic_data_item_queue.size(), dynamic_data_item_queue.topTime()));
    }

    // Pop data samples from the queue and check for:
    // * queue size decreases
    // * top time changes as expected -> the oldest sample is popped
    for (int i = 0; i < repetitions; i++)
    {
        dynamic_data_item_queue.pop();
        current_queue_size--;
        current_top_time++;

        ASSERT_EQ(dynamic_data_item_queue.size(), current_queue_size);
        if (0 == current_queue_size)
        {
            ASSERT_EQ(std::make_pair(current_queue_size, INVALID_timestamp_t_fep), std::make_pair(dynamic_data_item_queue.size(), dynamic_data_item_queue.topTime()));
        }
        else
        {
            ASSERT_EQ(std::make_pair(current_queue_size, current_top_time), std::make_pair(dynamic_data_item_queue.size(), dynamic_data_item_queue.topTime()));
        }
    }
}

/**
 * @req_id ""
 */
TEST(DynamicDataItemQueueTest, pushPopReceiverSampleDynamicDataItemQueue)
{
    timestamp_t current_top_time{ 0 };
    size_t current_queue_size{ 0 }, repetitions{ 10 };
    int32_t current_value{ 0 };
    DynamicDataItemQueue<> dynamic_data_item_queue(0);
    ASSERT_EQ(std::make_pair(current_queue_size, INVALID_timestamp_t_fep), std::make_pair(dynamic_data_item_queue.size(), dynamic_data_item_queue.topTime()));

    // Push data samples to the queue and check for:
    // * queue size increases
    // * no samples are being dropped
    // * top time does not change
    for (int i = 0; i < repetitions; i++)
    {
        data_read_ptr<const IDataRegistry::IDataSample> sample_to_push = createTestSample(i, i);
        dynamic_data_item_queue.push(sample_to_push, i);

        ASSERT_EQ(std::make_pair(++current_queue_size, current_top_time), std::make_pair(dynamic_data_item_queue.size(), dynamic_data_item_queue.topTime()));
    }

    data_read_ptr<const IDataRegistry::IDataSample> received_sample;
    DataItemReceiver data_sample_receiver(received_sample);

    // Use a receiver to pop data samples from the queue and check for:
    // * queue size decreases
    // * top time changes as expected -> the oldest sample is popped
    // * sample timestamp matches our expectation
    for (int i = 0; i < repetitions; i++)
    {
        dynamic_data_item_queue.pop(data_sample_receiver);
        current_queue_size--;

        RawMemoryStandardType<int32_t> read_value(current_value);
        ASSERT_EQ(sizeof(int32_t), received_sample->read(read_value));
        ASSERT_EQ(current_value, i);

        ASSERT_EQ(received_sample->getTime(), current_top_time);

        current_top_time++;

        ASSERT_EQ(dynamic_data_item_queue.size(), current_queue_size);
        if (0 == current_queue_size)
        {
            ASSERT_EQ(std::make_pair(current_queue_size, INVALID_timestamp_t_fep), std::make_pair(dynamic_data_item_queue.size(), dynamic_data_item_queue.topTime()));

        }
        else
        {
            ASSERT_EQ(std::make_pair(current_queue_size, current_top_time), std::make_pair(dynamic_data_item_queue.size(), dynamic_data_item_queue.topTime()));
        }
    }
}