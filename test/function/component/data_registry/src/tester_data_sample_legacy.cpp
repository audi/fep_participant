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
#include <fep3/components/data_registry/data_registry_fep2/data_sample_fep2.h>
#include <fep3/base/streamtype/default_streamtype.h>
#include "./../../helper/wait_for_data.hpp"

using namespace fep;

data_read_ptr<const IDataRegistry::IDataSample> createFep2TestSample(timestamp_t time,
                                                                     int32_t value,
                                                                     IUserDataAccess& data_access)
{
    fep::IUserDataSample* sample_fep2;
    data_access.CreateUserDataSample(sample_fep2);
    data_read_ptr<DataSampleFEP2> sample;
    sample.reset(new DataSampleFEP2(sample_fep2));
    *sample = DataSampleType<int32_t>(value);
    sample->setTime(time);
    return sample;
}

/**
 * @req_id ""
 */
TEST(DataSampleFEP2Wrappup, checkBasics)
{
    //to test an "old" data sample we need a module 
    cModule test_module;
    test_module.Create("name");
    
    //prepareSample
    fep::IUserDataSample* sample_fep2;
    test_module.GetUserDataAccess()->CreateUserDataSample(sample_fep2);
    ASSERT_EQ(sample_fep2->GetCapacity(), 0);
    ASSERT_EQ(sample_fep2->GetSize(), 0);

    int32_t value_attached_to_fep_2 = 100;
    sample_fep2->Attach(&value_attached_to_fep_2, sizeof(value_attached_to_fep_2));
    ASSERT_EQ(sample_fep2->GetCapacity(), 4);
    ASSERT_EQ(sample_fep2->GetSize(), 4);

    int32_t value_attached_to_a_sample = 200;
    DataSampleType<int32_t> sample(value_attached_to_a_sample);
    
    //Attach the FEP 2 User Sample to read it
    DataSampleFEP2 attached_sample(sample_fep2);
    ASSERT_EQ(attached_sample.capacity(), 4);
    ASSERT_EQ(attached_sample.size(), 4);
    ASSERT_EQ(attached_sample.getSize(), 4);

    ASSERT_EQ(value_attached_to_a_sample, 200);
    //we copy the sample
    sample = attached_sample;
    ASSERT_EQ(value_attached_to_a_sample, 100);

    //now other way 
    value_attached_to_a_sample = 400;
    attached_sample = sample;
    ASSERT_EQ(attached_sample.capacity(), 4);
    ASSERT_EQ(attached_sample.size(), 4);
    ASSERT_EQ(attached_sample.getSize(), 4);

    ASSERT_EQ(sample_fep2->GetCapacity(), 4);
    ASSERT_EQ(sample_fep2->GetSize(), 4);

    //the value has bee changed (we have attached memory)
    ASSERT_EQ(value_attached_to_fep_2, 400);

    //prepare another sample and try to allocate while copying
    //prepareSample
    fep::IUserDataSample* sample_fep2_empty_before;
    test_module.GetUserDataAccess()->CreateUserDataSample(sample_fep2_empty_before);
    ASSERT_EQ(sample_fep2_empty_before->GetCapacity(), 0);
    ASSERT_EQ(sample_fep2_empty_before->GetSize(), 0);

    //Attach the FEP 2 User Sample to read it
    DataSampleFEP2 attached_sample_empty_before(sample_fep2_empty_before);
    attached_sample_empty_before = sample;
    ASSERT_EQ(sample_fep2_empty_before->GetCapacity(), 4);
    ASSERT_EQ(sample_fep2_empty_before->GetSize(), 4);

    int32_t* value_to_check = static_cast<int32_t*>(sample_fep2_empty_before->GetPtr());
    ASSERT_EQ(*value_to_check, 400);

    value_attached_to_fep_2 = 500;
    attached_sample_empty_before = *static_cast<IDataRegistry::IDataSample*>(&attached_sample);

    value_to_check = static_cast<int32_t*>(sample_fep2_empty_before->GetPtr());
    ASSERT_EQ(*value_to_check, 500);

}

/**
 * @req_id ""
 */
TEST(DataReaderBacklogTestLegcy, checkBackLogRead)
{
    //to test an "old" data sample we need a module 
    cModule test_module;
    test_module.Create("name");

    DataReaderBacklog backlog_queue(20, StreamTypePlain<int32_t>());
    ASSERT_EQ(backlog_queue.capacity(), 20);

    int32_t receive_value = 0;
    DataSampleType<int32_t> receive_sample(receive_value);

    //queue is empty 
    ASSERT_EQ(backlog_queue.size(), 0);

    for (auto idx = 0; idx < 99; idx++)
    {
        data_read_ptr<const IDataRegistry::IDataSample> sample_to_write = createFep2TestSample(idx, idx, *test_module.GetUserDataAccess());
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
        data_read_ptr<const IDataRegistry::IDataSample> sample_to_write = createFep2TestSample(idx, idx, *test_module.GetUserDataAccess());
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