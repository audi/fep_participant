/**
 * Implementation of the tester for the FEP Data Sample (locking)
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

/**
* Test Case:   TestThreadSafety
* Test ID:     1.5
* Test Title:  Thread Safety Tests
* Description: Test the thread safety of cDataAccess.
* Strategy:    Add samples using threads. Call the functions LockData and UnlockData while sending data.
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1536
*/

#ifdef _MSC_VER
#pragma warning (disable:4127)
#endif

#include <gtest/gtest.h>

#include "fep_participant_sdk.h"
#include "fep_test_common.h"

#include "transmission_adapter/fep_transmission.h"

#include "function/_common/fep_mock_tx_adapter.h"
#include "function/_common/fep_mock_signal_registry.h"
#include "function/_common/fep_mock_signal_mapping.h"
#include "function/_common/fep_mock_incident_handler.h"
#include "function/_common/fep_mock_property_tree.h"


#include <cmath>

using namespace fep;

#include <ddl.h>
using namespace ddl;

#include "data_access/fep_data_access.h"
#include "transmission_adapter/fep_data_sample_factory.h"

#include "fep_my_mock_tx_adapter.h"
#include "fep_my_mock_signal_registry.h"
#include "fep_my_mock_signal_mapping.h"
#include "tester_fep_data_access.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif


template <int PRODUCE_DELAY_US, int FIRST_NUMBER= 1, int LAST_NUMBER= -1, int START_TIMESTAMP= -1, int TIMESTAMP_INC= 1> class cPackageProducer
{
public:
    cPackageProducer(cTesterFepDataAccess* pTesterFepDataAccess)
        : oDataAccess(pTesterFepDataAccess->oDataAccess)
        , poFactory(pTesterFepDataAccess->poFactory)
        , hReceiverSignal(pTesterFepDataAccess->hReceiverSignal)
    {
        m_pThread.reset(new a_util::concurrency::thread(&cPackageProducer::ThreadFunc, this));
    }

    ~cPackageProducer()
    {
        stop();
    }

public:
    void start()
    {
        m_oStartup.notify();
    }

    void stop()
    {
        if (m_pThread)
        {
            m_oShutdown.notify();
            m_pThread->join();
            m_pThread.reset();
        }
    }

private:
    void ThreadFunc()
    {
        IUserDataSample* pSample;

        // Create a sample
        oDataAccess.CreateUserDataSample(pSample, hReceiverSignal);

        // Create a signal code
        ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pSample->GetPtr(), pSample->GetSize());

        timestamp_t ti = START_TIMESTAMP;

        m_oStartup.wait();
        for (int32_t i = FIRST_NUMBER; !m_oShutdown.is_set() && (LAST_NUMBER < 0 || i <= LAST_NUMBER); ++i)
        {
            ddl::access_element::set_value(oSignalCoder, "i32Val", i);

            if (ti >= 0)
            {
                ASSERT_EQ(a_util::result::SUCCESS, pSample->SetTime(ti));
                ti += TIMESTAMP_INC;
            }
            else
            {
                ASSERT_EQ(a_util::result::SUCCESS, pSample->SetTime(a_util::system::getCurrentMicroseconds()));
            }

            oDataAccess.Update(pSample);

            if (PRODUCE_DELAY_US > 0)
            {
                a_util::system::sleepMicroseconds(PRODUCE_DELAY_US);
            }
            else if (PRODUCE_DELAY_US == 0)
            {
                a_util::concurrency::this_thread::yield();
            }
        }

        delete pSample;
    }

    cDataAccess& oDataAccess;
    ddl::CodecFactory*& poFactory;
    handle_t& hReceiverSignal;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
    a_util::concurrency::semaphore m_oShutdown;
    a_util::concurrency::semaphore m_oStartup;
};

/**
 * @req_id "FEPSDK-1536"
 */
TEST_F(cTesterFepDataAccess, TestThreadSafetyWithOneBacklogSample)
{
    SetBacklogSize(1);

    cPackageProducer<100> oPackageProducer(this);
    oPackageProducer.start();

    timestamp_t tmLastTimestamp = 0;
    for (int32_t i = 0; i < 100; ++i)
    {
        a_util::system::sleepMicroseconds(100 * 2);

        const IUserDataSample* pReceiverSample = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockData(hReceiverSignal, pReceiverSample));

        timestamp_t tmThisTimestamp = pReceiverSample->GetTime();

        // Wait time is big enough, each packet must be newer
        EXPECT_GE(tmThisTimestamp, tmLastTimestamp);
        tmLastTimestamp = tmThisTimestamp;

        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));
    }
}

/**
 * @req_id "FEPSDK-1537"
 */
TEST_F(cTesterFepDataAccess, TestThreadSafety2ThreadProduce)
{
    SetBacklogSize(100);

    // Let two producers produce 100 packets quite fast
    // Result should be alternating packages:
    //
    // packet | timestamp |  value 
    // ---------------------------
    //      1 |         1 |   1000
    //      2 |         2 |   2000
    //      3 |         3 |   1001
    //      4 |         4 |   2001
    // ...
    //     98 |        98 |   1049
    //     99 |        99 |   2049
    //
    cPackageProducer<0, 1000, 1049, 1, 2> oPackageProducer1(this);
    cPackageProducer<0, 2000, 2049, 2, 2> oPackageProducer2(this);

    oPackageProducer1.start();
    oPackageProducer2.start();

    a_util::system::sleepMicroseconds(100000);

    for (int32_t i = 1; i <= 100; ++i)
    {
        const IUserDataSample* pReceiverSample = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pReceiverSample, i, IUserDataAccess::SS_NEAREST_SAMPLE));
        
        ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSample->GetPtr(), pReceiverSample->GetSize());
        
        int32_t v = 1000 + ((i-1) % 2) * 1000 + ((i-1) / 2);
        //std::cerr << i << " " << pReceiverSample->GetTime() << " : " << ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32() << " == " << v << std::endl;
        
        EXPECT_EQ(i, pReceiverSample->GetTime());
        EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), v);
    }
}

/**
 * @req_id "FEPSDK-1536"
 */
TEST_F(cTesterFepDataAccess, TestThreadSafety4ThreadProduce)
{
    SetBacklogSize(200);

    // Let two producers produce 200 packets quite fast
    // Result should be alternating packages:
    //
    // packet | timestamp |  value 
    // ---------------------------
    //      1 |         1 |   1000
    //      2 |         2 |   2000
    //      3 |         3 |   1001
    //      4 |         4 |   2001
    // ...
    //     98 |        98 |   1049
    //     99 |        99 |   2049
    //
    cPackageProducer<0, 1000, 1049, 1, 4> oPackageProducer1(this);
    cPackageProducer<0, 2000, 2049, 2, 4> oPackageProducer2(this);
    cPackageProducer<0, 3000, 3049, 3, 4> oPackageProducer3(this);
    cPackageProducer<0, 4000, 4049, 4, 4> oPackageProducer4(this);

    oPackageProducer1.start();
    oPackageProducer2.start();
    oPackageProducer3.start();
    oPackageProducer4.start();

    a_util::system::sleepMicroseconds(100000);

    for (int32_t i = 1; i <= 200; ++i)
    {
        const IUserDataSample* pReceiverSample = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockDataAt(hReceiverSignal, pReceiverSample, i, IUserDataAccess::SS_NEAREST_SAMPLE));

        ddl::StaticCodec oSignalCoder = poFactory->makeStaticCodecFor(pReceiverSample->GetPtr(), pReceiverSample->GetSize());

        int32_t v = 1000 + ((i - 1) % 4) * 1000 + ((i - 1) / 4);
        //std::cerr << i << " " << pReceiverSample->GetTime() << " : " << ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32() << " == " << v << std::endl;

        EXPECT_EQ(i, pReceiverSample->GetTime());
        EXPECT_EQ(ddl::access_element::get_value(oSignalCoder, "i32Val").asInt32(), v);
    }
}

/**
 * @req_id "FEPSDK-1536"
 */
TEST_F(cTesterFepDataAccess, TestThreadSafetyReadWhileProduce)
{
    SetBacklogSize(10);

    // Let two producers produce 200 packets 
    cPackageProducer<0, 1000> oPackageProducer1(this);
    cPackageProducer<0, 2000> oPackageProducer2(this);

    oPackageProducer1.start();
    oPackageProducer2.start();

    for (int32_t i = 1; i <= 1000; ++i)
    {
        const IUserDataSample* pReceiverSamples[10];

        // Lock all samples
        for (int j = 0; j < 10; ++j)
        {
            const IUserDataSample*& pReceiverSample= pReceiverSamples[j];
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.LockData(hReceiverSignal, pReceiverSample));
            a_util::concurrency::this_thread::yield();
        }

        for (int j = 0; j < 10; ++j)
        {
            const IUserDataSample*& pReceiverSample = pReceiverSamples[j];
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.UnlockData(pReceiverSample));
            a_util::concurrency::this_thread::yield();
        }
    }

    oPackageProducer1.stop();
    oPackageProducer2.stop();
}
