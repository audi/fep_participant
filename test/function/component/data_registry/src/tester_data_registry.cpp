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
#include <fep3/components/data_registry/raw_memory.h>

#include "./../../helper/run_module.hpp"
#include "./../../helper/wait_for_data.hpp"

#include <iostream>
#include <fstream>

using namespace fep;

namespace fep {
    class MyDataReceiveListener : public IDataRegistry::IDataReceiver
    {
    public:
        int cntStream;
        int cntSample;

        MyDataReceiveListener()
        {
            cntStream = 0;
            cntSample = 0;
        }
        virtual void    onReceive   (const data_read_ptr<const IStreamType>& type)
        {
            cntStream++;
        };
        virtual void    onReceive   (const data_read_ptr<const IDataRegistry::IDataSample>& sample)
        {
            cntSample++;
        };
    };
}

/**
 * @req_id ""
 */
TEST(DataRegistryTest, checkBaseImpl)
{
    std::string sent_testval = "testvalue";
    std::string received_testval = "";

    cModule test_receiver;
    ASSERT_TRUE(fep::Result() == test_receiver.Create(cModuleOptions("t_receiver", eTimingSupportDefault::timing_FEP_30)));
    IDataRegistry* datareg_receiver = getComponent<IDataRegistry>(test_receiver);
    ASSERT_TRUE(datareg_receiver != nullptr);
    MyDataReceiveListener datarec_listener;
    ASSERT_TRUE(fep::Result() == datareg_receiver->registerDataIn("test_signal_raw", StreamTypeRaw()));
    ASSERT_TRUE(fep::Result() == datareg_receiver->registerDataReceiveListener("test_signal_raw", datarec_listener));

    cModule test_sender;
    ASSERT_TRUE(fep::Result() == test_sender.Create(cModuleOptions("t_sender", eTimingSupportDefault::timing_FEP_30)));
    IDataRegistry* datareg_sender = getComponent<IDataRegistry>(test_sender);
    ASSERT_TRUE(datareg_sender != nullptr);
    ASSERT_TRUE(fep::Result() == datareg_sender->registerDataOut("test_signal_raw", StreamTypeRaw()));

    std::unique_ptr<IDataRegistry::IDataWriter> writer_signalraw = datareg_sender->getWriter("test_signal_raw", 1);
    ASSERT_TRUE(writer_signalraw);
    std::unique_ptr<IDataRegistry::IDataReader> reader_signalraw = datareg_receiver->getReader("test_signal_raw");
    ASSERT_TRUE(reader_signalraw);

    *writer_signalraw << sent_testval; //we will chack for exception ?? when it throws

    ASSERT_TRUE(isOk(runModule(test_sender)));
    ASSERT_TRUE(isOk(runModule(test_receiver)));

    *writer_signalraw << sent_testval;
    writer_signalraw->flush();
    ASSERT_TRUE(wait_for_data_change_busy(*reader_signalraw, received_testval, received_testval, 1000));
    ASSERT_EQ(received_testval, sent_testval);
    EXPECT_EQ(datarec_listener.cntStream, 0);
    EXPECT_EQ(datarec_listener.cntSample, 1);

    // also check that unregister works (FEPSDK-2271)
    ASSERT_TRUE(fep::Result() == datareg_receiver->unregisterDataReceiveListener("test_signal_raw", datarec_listener));
    ASSERT_TRUE(fep::Result() == datareg_receiver->unregisterDataIn("test_signal_raw"));
    ASSERT_TRUE(fep::Result() == datareg_sender->unregisterDataOut("test_signal_raw"));
}
