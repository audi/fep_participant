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
/**
* Test Case:   TestFragmentation
* Test Title:  Test fragmentation of large samples
* Description: This test tests
* Strategy:    todo
*                            
* Passed If:   End of test reached
*              
* Ticket:      -
* Requirement: FEP_SDK_xxx
*/
#include <gtest/gtest.h>
#include "transmission_adapter/fep_fragmentation.h"
#include <string>

static constexpr unsigned protocol_version = 3;

template <uint32_t fragmentation_boundary>
class TestReceiver : public fep::DefragmentingReader<protocol_version, fragmentation_boundary>
{
public:
    std::string _received_str;
    uint64_t _received_n;
    TestReceiver() : _received_n(0) {}

protected:
    void receiveSample(const void* sample, uint32_t length) noexcept final override
    {
        _received_str.assign(static_cast<const char*>(sample), length);
        _received_n = length;
    }
};

template <uint32_t fragmentation_boundary>
class TestTransmitter : public fep::FragmentingWriter<protocol_version, fragmentation_boundary>
{
    TestReceiver<fragmentation_boundary>& _recv;
    uint8_t _buffer[sizeof(fep::Fragment) + fragmentation_boundary];
public:
    TestTransmitter(TestReceiver<fragmentation_boundary>& recv, uint64_t id) :
        fep::FragmentingWriter<protocol_version, fragmentation_boundary>(id),
        _recv(recv)
    {
        this->setBuffer(&_buffer[0]);
    }

public:
    bool transmitFragment(void* fragment, uint32_t length) noexcept final override
    {
        return _recv.receiveFragment(fragment, length) == 0;
    }
};

/**
 * @req_id "FEPSDK-1513"
 */
TEST(cTransmissionAdapterTester, TestBasicFragmentation)
{
    using TestReceiver = TestReceiver<10>;
    using TestTransmitter = TestTransmitter<10>;

    uint8_t dummy[27] = "abcdefghijklmnopqrstuvwxyz";

    TestReceiver receiver;
    TestTransmitter transmitter(receiver, 1);

    ASSERT_TRUE(transmitter.transmitSample(nullptr, 42) == 0);
    ASSERT_TRUE(receiver._received_n == 0);

    ASSERT_TRUE(transmitter.transmitSample(&dummy, 0) == 0);
    ASSERT_TRUE(receiver._received_n == 0);

    ASSERT_TRUE(transmitter.transmitSample(&dummy, 10 * 65536 + 1) == 0);
    ASSERT_TRUE(receiver._received_n == 0);

    ASSERT_TRUE(transmitter.transmitSample(&dummy, 1) == 1);
    ASSERT_TRUE(receiver._received_str == "a");
    ASSERT_TRUE(receiver._received_n == 1);

    ASSERT_TRUE(transmitter.transmitSample(&dummy, 10) == 1);
    ASSERT_TRUE(receiver._received_str == "abcdefghij");
    ASSERT_TRUE(receiver._received_n == 10);

    ASSERT_TRUE(transmitter.transmitSample(&dummy, 11) == 2);
    ASSERT_TRUE(receiver._received_str == "abcdefghijk");
    ASSERT_TRUE(receiver._received_n == 11);

    ASSERT_TRUE(transmitter.transmitSample(&dummy, 26) == 3);
    ASSERT_TRUE(receiver._received_str == "abcdefghijklmnopqrstuvwxyz");
    ASSERT_TRUE(receiver._received_n == 26);
}

/**
 * @req_id "FEPSDK-1514"
 */
TEST(cTransmissionAdapterTester, TestFragmentationLostFragments)
{
    using TestReceiver = TestReceiver<10>;

    // we need a custom transmitter here
    class TestTransmitter : public fep::FragmentingWriter<protocol_version, 10>
    {
        TestReceiver& _recv;
        uint8_t _buffer[sizeof(fep::Fragment) + 10];
    public:
        TestTransmitter(TestReceiver& recv, uint64_t id) :
            fep::FragmentingWriter<protocol_version, 10>(id),
            _recv(recv)
        {
            setBuffer(&_buffer[0]);
        }

    public:
        int fragments_to_transport = -1;
        uint64_t expected_lost_fragments = 0;
        bool transmitFragment(void* fragment, uint32_t length) noexcept final override
        {
            if (fragments_to_transport > 0)
            {
                fragments_to_transport--;
                bool ret = _recv.receiveFragment(fragment, length) == expected_lost_fragments;
                expected_lost_fragments = 0;
                return ret;
            }
            return true; // simulate dropped fragment
        }
    };

    uint8_t dummy[27] = "abcdefghijklmnopqrstuvwxyz";

    TestReceiver receiver;
    TestTransmitter transmitter(receiver, 1);

    // transmit one full sample first
    transmitter.fragments_to_transport = 3;
    ASSERT_TRUE(transmitter.transmitSample(&dummy, 26) == 3);
    ASSERT_TRUE(receiver._received_str == "abcdefghijklmnopqrstuvwxyz");
    ASSERT_TRUE(receiver._received_n == 26);
    ASSERT_TRUE(transmitter.fragments_to_transport == 0);

    receiver._received_str.clear();
    receiver._received_n = 0;

    // now transmit two fragments of sample n2
    transmitter.fragments_to_transport = 2;
    ASSERT_TRUE(transmitter.transmitSample(&dummy, 26) == 3);
    ASSERT_TRUE(transmitter.fragments_to_transport == 0);

    // sample should not have been received of course
    ASSERT_TRUE(receiver._received_str.empty());
    ASSERT_TRUE(receiver._received_n == 0);

    // now transport a new sample without dropping
    transmitter.fragments_to_transport = 3;
    transmitter.expected_lost_fragments = 1;
    ASSERT_TRUE(transmitter.transmitSample(&dummy, 26) == 3);

    // the complete sample should have been received
    ASSERT_TRUE(receiver._received_str == "abcdefghijklmnopqrstuvwxyz");
    ASSERT_TRUE(receiver._received_n == 26);
}

/**
 * @req_id "FEPSDK-1513"
 */
TEST(cTransmissionAdapterTester, TestFragmentationMultipleSenders)
{
    using TestReceiver = TestReceiver<10>;
    TestReceiver receiver;
    TestTransmitter<10> transmitter1(receiver, 1);
    TestTransmitter<10> transmitter2(receiver, 2);

    uint8_t dummy[27] = "abcdefghijklmnopqrstuvwxyz";

    ASSERT_TRUE(transmitter1.transmitSample(&dummy[0], 5) == 1);
    ASSERT_EQ(receiver._received_str, "abcde");

    ASSERT_TRUE(transmitter2.transmitSample(&dummy[5], 5) == 1);
    ASSERT_EQ(receiver._received_str, "fghij");

    ASSERT_TRUE(transmitter2.transmitSample(&dummy[10], 5) == 1);
    ASSERT_EQ(receiver._received_str, "klmno");

    ASSERT_TRUE(transmitter1.transmitSample(&dummy[0], 26) == 3);
    ASSERT_EQ(receiver._received_str, "abcdefghijklmnopqrstuvwxyz");

    ASSERT_TRUE(transmitter2.transmitSample(&dummy[10], 16) == 2);
    ASSERT_EQ(receiver._received_str, "klmnopqrstuvwxyz");
}

/**
 * @req_id "FEPSDK-1513"
 */
TEST(cTransmissionAdapterTester, TestFragmentationBenchmark)
{
    using TestReceiver = TestReceiver<63000 - sizeof(fep::Fragment)>;
    using TestTransmitter = TestTransmitter<63000 - sizeof(fep::Fragment)>;

    // 40 MB sample data
    std::string sample = std::string(40 * 1024 * 1024, '\0');
    char val = 0;
    for (char& c : sample)
    {
        c = val++;
    }

    TestReceiver receiver;
    TestTransmitter transmitter(receiver, 1);

    // upper_bound(40 * 1024 * 1024 / 62974) == 667
    ASSERT_EQ(transmitter.transmitSample(sample.c_str(), static_cast<uint32_t>(sample.size())), 667);
    ASSERT_TRUE(receiver._received_n == sample.size());
    ASSERT_TRUE(receiver._received_str == sample);
}
