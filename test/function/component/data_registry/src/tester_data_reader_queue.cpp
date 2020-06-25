/**
* Implementation of the Class TimingClient.
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
#include <fep3/components/data_registry/data_reader_queue.h>
#include <fep3/base/streamtype/default_streamtype.h>
#include "./../../helper/wait_for_data.hpp"
#include "./test_helper.h"

using namespace fep;

/**
 * @req_id ""
 */
TEST(DataReaderQueueTest, checkDataReaderQueue)
{
    data_read_ptr<const IDataRegistry::IDataSample> sample_to_write = createTestSample(100, 100);

    DataReaderQueue reader_queue(1);
    ASSERT_EQ(reader_queue.capacity(), 1);

    int32_t receive_value = 0;
    DataSampleType<int32_t> receive_sample(receive_value);

    //queue is empty 
    ASSERT_EQ(reader_queue.size(), 0);

    reader_queue.onReceive(sample_to_write);

    //queue is NOT empty 
    ASSERT_EQ(reader_queue.size(), 1);

    data_read_ptr<const IDataRegistry::IDataSample> receive;
    DataSampleReceiver rece(receive);
    ASSERT_TRUE(reader_queue.readItem(rece));
    
    //queue is empty 
    ASSERT_EQ(reader_queue.size(), 0);
    reader_queue.onReceive(sample_to_write);
    //queue is NOT empty 
    ASSERT_EQ(reader_queue.size(), 1);

    ASSERT_TRUE(reader_queue.readItem(rece));
    ASSERT_EQ(reader_queue.size(), 0);
}

/**
 * @req_id ""
 */
TEST(DataReaderQueueTest, checkQueueRead)
{
    DataReaderQueue reader_queue(20);
    ASSERT_EQ(reader_queue.capacity(), 20);

    int32_t receive_value = 0;
    DataSampleType<int32_t> receive_sample(receive_value);

    //queue is empty 
    ASSERT_EQ(reader_queue.size(), 0);

    for (auto idx = 0; idx < 100; idx++)
    {
        data_read_ptr<const IDataRegistry::IDataSample> sample_to_write = createTestSample(idx, idx);
        reader_queue.onReceive(sample_to_write);
    }

    //queue is NOT empty
    //100 items received, but 20 items only exists
    ASSERT_EQ(reader_queue.size(), 20);
    ASSERT_EQ(reader_queue.capacity(), 20);

    auto current_size = 20;
    for (auto read_idx = 80; read_idx < 100; read_idx++)
    {
        data_read_ptr<const IDataRegistry::IDataSample> received_sample;
        DataSampleReceiver rece(received_sample);
        ASSERT_TRUE(reader_queue.readItem(rece));
        ASSERT_EQ(reader_queue.size(), --current_size);
        int32_t current_val;
        RawMemoryStandardType<int32_t> readval(current_val);
        ASSERT_EQ(sizeof(int32_t), received_sample->read(readval));
        ASSERT_EQ(current_val, read_idx);
    }
    
    // queue is empty now
    //100 items received, but 20 items only exists
    ASSERT_EQ(reader_queue.size(), 0);
    ASSERT_EQ(reader_queue.capacity(), 20);
}

/**
 * @req_id ""
 */
TEST(DataReaderBacklogTest, checkBackLogRead)
{
    DataReaderBacklog backlog_queue(20, StreamTypePlain<int32_t>());
    ASSERT_EQ(backlog_queue.capacity(), 20);

    int32_t receive_value = 0;
    DataSampleType<int32_t> receive_sample(receive_value);

    //queue is empty 
    ASSERT_EQ(backlog_queue.size(), 0);

    for (auto idx = 0; idx < 99; idx++)
    {
        data_read_ptr<const IDataRegistry::IDataSample> sample_to_write = createTestSample(idx, idx);
        backlog_queue.onReceive(sample_to_write);
    }

    //queue is NOT empty
    //100 items received, but 20 items only exists
    ASSERT_EQ(backlog_queue.size(), 20);
    ASSERT_EQ(backlog_queue.capacity(), 20);

    for (auto read_idx = 79; read_idx < 99; read_idx++)
    {
        data_read_ptr<const IDataRegistry::IDataSample> received_sample;
        received_sample = backlog_queue.read();
        //it is still 20 after reading
        ASSERT_EQ(backlog_queue.size(), 20);
        int32_t current_val;
        RawMemoryStandardType<int32_t> readval(current_val);
        ASSERT_EQ(sizeof(int32_t), received_sample->read(readval));
        //the last read value is always 98
        ASSERT_EQ(current_val, 98);
    }
    //we add one more
    for (auto idx = 99; idx < 100; idx++)
    {
        data_read_ptr<const IDataRegistry::IDataSample> sample_to_write = createTestSample(idx, idx);
        backlog_queue.onReceive(sample_to_write);
    }

    for (auto read_idx = 80; read_idx < 100; read_idx++)
    {
        data_read_ptr<const IDataRegistry::IDataSample> received_sample;
        received_sample = backlog_queue.read();
        //it is still 20 after reading
        ASSERT_EQ(backlog_queue.size(), 20);
        int32_t current_val;
        RawMemoryStandardType<int32_t> readval(current_val);
        ASSERT_EQ(sizeof(int32_t), received_sample->read(readval));
        //the last read value is always 99
        ASSERT_EQ(current_val, 99);
    }

    for (auto read_idx = 80; read_idx < 100; read_idx++)
    {
        data_read_ptr<const IDataRegistry::IDataSample> received_sample;
        received_sample = backlog_queue.readBefore(read_idx);
        //it is still 20 after reading
        ASSERT_EQ(backlog_queue.size(), 20);
        int32_t current_val;
        RawMemoryStandardType<int32_t> readval(current_val);
        ASSERT_EQ(sizeof(int32_t), received_sample->read(readval));
        //the last read value is now the value before (time has been set to the same as the value )
        ASSERT_EQ(current_val, read_idx);
    }
}