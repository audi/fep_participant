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
#if !defined(DDS_VARIANT_IS_FASTRTPS_LL) && !defined(DDS_VARIANT_IS_FASTRTPS_HL) && !defined(DDS_VARIANT_IS_RTIDDS) && !defined(DDS_VARIANT_IS_FEP)
#error "No DDS variant is defined"
#endif

#include <iostream>

#ifdef DDS_VARIANT_IS_FASTRTPS_LL
#include "fastrtps_ll_wrap/fastrtps_ll_wrap.h"
using namespace fastrtps_ll_wrap;
#endif

#ifdef DDS_VARIANT_IS_FASTRTPS_HL
#include "fastrtps_hl_wrap/fastrtps_hl_wrap.h"
using namespace fastrtps_hl_wrap;
#endif

#ifdef DDS_VARIANT_IS_RTIDDS
#include "rti_wrap/rti_wrap.h"
using namespace rti_wrap;
#endif

#ifdef DDS_VARIANT_IS_FEP
#include "fep_wrap/fep_wrap.h"
using namespace fep_wrap;
static const char* s_dds_variant_id_string = "FEP";
#endif

class ServerElement : public ISubscriberListener
{
public:
    ServerElement(const DdsConfig& dds_config, const int server_id, const std::string& name)
        : m_server_id(static_cast<const int16_t>(server_id))
    {
        m_participant= new Participant(dds_config, name);
        
        m_topic_ping = new Topic(*m_participant, "Ping");
        m_topic_pong = new Topic(*m_participant, "Pong");

        m_subscriber = new Subscriber(dds_config, this, *m_participant, *m_topic_ping);
        m_publisher = new Publisher(dds_config, *m_participant, *m_topic_pong);

        m_publisher_sample = new PublisherSample(*m_publisher, dds_config.sample_size);

        m_participant->Start();
    }

    ~ServerElement()
    {
        m_participant->Stop();

        delete m_subscriber;
        delete m_publisher;
        delete m_publisher_sample;
        delete m_topic_ping;
        delete m_topic_pong;
        delete m_participant;
    }

public:
    void receivedSample(const void* data, const int data_size, const SampleInfo& sample_info)
    {
        const timestamp_t usr_recv_timestamp= Helper::getCurrentMicroseconds();
        const sSampleHeader* input_sample_header = reinterpret_cast<const sSampleHeader*>(data);

        sSampleHeader* sample_header = m_publisher_sample->getPtr<sSampleHeader>();
        sample_header->sample_number = input_sample_header->sample_number;
        sample_header->client_id = input_sample_header->client_id;
        sample_header->server_id = m_server_id;
        sample_header->usr_pub_send_timestamp = input_sample_header->usr_pub_send_timestamp;
        sample_header->usr_sub_recv_timestamp = usr_recv_timestamp;
        sample_header->dds_pub_send_timestamp = sample_info.dds_source_timestamp;
        sample_header->dds_sub_recv_timestamp = sample_info.dds_reception_timestamp;
        sample_header->usr_sub_send_timestamp = Helper::getCurrentMicroseconds();
        sample_header->llv_sub_recv_timestamp = sample_info.llv_reception_timestamp;
        sample_header->usr_sub_send_timestamp = Helper::getCurrentMicroseconds();

        m_publisher->Transmit(m_publisher_sample);
    }

private:
    int16_t m_server_id;
    Participant* m_participant;
    Topic* m_topic_ping;
    Topic* m_topic_pong;
    Subscriber* m_subscriber;
    Publisher* m_publisher;
    PublisherSample* m_publisher_sample;
};

static void usage(const char* progname)
{
    std::cerr << "Usage: " << progname << " <size> <mode> [<server-id>]" << std::endl;
    std::cerr << "  <size> (mandatory) sample size to use. default is 1024." << std::endl;
    std::cerr << "  <mode> (mandatory) is 0 (BEST_EFFORT) or 1 (RELIABLE)" << std::endl;
    std::cerr << "  <multicast> (mandatory) is 0 (UNICAST) or 1 (MULTICAST)" << std::endl;
    std::cerr << "  <async> (mandatory) is 0 (SYNC) or 1 (ASYNC)" << std::endl;
    std::cerr << "  <server-id> (optional) is a server-id from 0 to 31" << std::endl;
}

int main(int argc, char* argv[])
{
    int server_id = 0;
    int server_id_to = -1;

    if (argc < 5 || argc > 6)
    {
        usage(argv[0]);
        return 1;
    }

    DdsConfig dds_config;
    dds_config.multicast_enable = false;
    dds_config.sample_size = atoi(argv[1]);
    int reliability_mode_num = atoi(argv[2]);
    dds_config.reliability_mode = (reliability_mode_num == 0) ? BestEffortReliabilityMode : ReliableReliabilityMode;
    int multicast_mode_num = atoi(argv[3]);
    dds_config.multicast_enable = (multicast_mode_num != 0);
    int async_mode_num = atoi(argv[4]);
    dds_config.async_mode_enable = (async_mode_num != 0);

    if (argv[5])
    {
        server_id = atoi(argv[5]);

        char* sep = strchr(argv[5], '-');
        if (sep)
        {
            server_id_to = atoi(sep + 1);
        }
    }

    if (reliability_mode_num < 0 || reliability_mode_num > static_cast<int>(MaximumNumberForReliabilityMode))
    {
        usage(argv[0]);
        return 1;
    }

    if (server_id < 0 || server_id > 31)
    {
        usage(argv[0]);
        return 1;
    }

    if (server_id_to >= 0)
    {
        // Not yet supported
#if 0
        // Starter mode ... run all servers
        const char* x_argv[4];
        std::string number_string;

        x_argv[0] = argv[0];
        x_argv[1] = argv[1];
        x_argv[3] = NULL;

        for (int i= server_id+1; i<= server_id_to; ++i)
        {
            number_string = std::to_string(i);
            x_argv[2] = number_string.c_str();
           
#ifdef WIN32
            _execv(x_argv[0], x_argv);
#else
#error TODO
#endif
        }
#endif
    }
    
    ServerElement server_element(dds_config, server_id, std::string("Server") + std::to_string(server_id));

    std::cout << "Press enter to terminate." << std::endl;
    char x;
    std::cin >> x;
    
    //while ()
    //{
    //    Sleep(1000);
    //}

    return 0;
}