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
#include <fep3/components/data_registry/data_reader.h>
#include <fep3/components/data_registry/data_writer.h>

#include "./../../helper/run_module.hpp"
#include "./../../helper/wait_for_data.hpp"

using namespace fep;


/**
 * @req_id ""
 */
TEST(DataReaderTest, checkBaseImplDataReader)
{
    /*
    cModule sender_mod;
    cModule receiver_mod;
    sender_mod.Create(cModuleOptions("sender", eTimingSupportDefault::timing_FEP_30));
    receiver_mod.Create(cModuleOptions("receiver", eTimingSupportDefault::timing_FEP_30));

    DataReader reader("test", StreamTypeRaw());

    ASSERT_TRUE(isOk(addToDataRegistry(receiver_mod, reader)));

    DataWriter writer("test", StreamTypeRaw());
    ASSERT_TRUE(isOk(addToDataRegistry(sender_mod, writer)));

    ASSERT_TRUE(isOk(runModule(sender_mod)));
    ASSERT_TRUE(isOk(runModule(receiver_mod)));

    std::string sender_string = "this is the sent string";
    std::string receiver_string = "";

    //make sure the string comes thru
    ASSERT_TRUE(isOk(writer.writeByType(sender_string)));
    ASSERT_TRUE(wait_for_data_change_busy(reader, receiver_string, receiver_string, 1000));
    ASSERT_EQ(sender_string, receiver_string);

    //make sure the time comes thru
    sender_string = "make sure the time comes thru";
    ASSERT_TRUE(isOk(writer.write(1234, sender_string.c_str(), sender_string.length() + 1)));
    ASSERT_TRUE(wait_for_data_change_busy(reader, receiver_string, receiver_string, 1000));
    data_read_ptr<const IDataRegistry::IDataSample> sample_to_check = reader.read();
    ASSERT_EQ(sample_to_check->getTime(), 1234);
    //check data again
    ASSERT_EQ(sender_string, receiver_string);

    //make sure we can also use teh streaming operators
    sender_string = "make sure the >> and << works";
    writer << sender_string;
    ASSERT_TRUE(wait_for_data_change_busy(reader, receiver_string, receiver_string, 1000));
    ASSERT_EQ(sender_string, receiver_string);
    //read with streaming op
    std::string receiver_string_with_streaming_op;
    reader >> receiver_string_with_streaming_op;
    ASSERT_EQ(sender_string, receiver_string_with_streaming_op);
    ASSERT_EQ(receiver_string, receiver_string_with_streaming_op);
    */
}