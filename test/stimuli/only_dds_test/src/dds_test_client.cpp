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

#define NOMINMAX

#include <fstream>
#include <iostream>
#include <array>

#ifdef DDS_VARIANT_IS_FASTRTPS_LL
#include "fastrtps_ll_wrap/fastrtps_ll_wrap.h"
using namespace fastrtps_ll_wrap;
static const char* s_dds_variant_id_string = "fastrtps_ll";
#endif

#ifdef DDS_VARIANT_IS_FASTRTPS_HL
#include "fastrtps_hl_wrap/fastrtps_hl_wrap.h"
using namespace fastrtps_hl_wrap;
static const char* s_dds_variant_id_string = "fastrtps_hl";
#endif

#ifdef DDS_VARIANT_IS_RTIDDS
#include "rti_wrap/rti_wrap.h"
using namespace rti_wrap;
static const char* s_dds_variant_id_string = "rtidds";
#endif

#ifdef DDS_VARIANT_IS_FEP
#include "fep_wrap/fep_wrap.h"
using namespace fep_wrap;
static const char* s_dds_variant_id_string = "FEP";
#endif

#ifdef WIN32
#include <windows.h>
void usleep(__int64 usec)
{
    static HANDLE timer= INVALID_HANDLE_VALUE;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    if (INVALID_HANDLE_VALUE == timer)
    {
        timer = CreateWaitableTimer(NULL, TRUE, NULL);
    }

    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    //CloseHandle(timer);
}
#else
#include <unistd.h>
#endif

class PerServerStatistics;
class GlobalStatistics;

template <int NUMBER_OF_SAMPLES> class StatisticsDataStorage
{
    friend class PerServerStatistics;
    friend class GlobalStatistics;

    struct StorageItem
    {
        timestamp_t T1;
        timestamp_t T2;
        timestamp_t T3;
        timestamp_t T4;
    };

public:
    StatisticsDataStorage()
        : packet_count(0)
        , storage_array()
    {
    }

    void appendTimestamps(const timestamp_t& T1, const timestamp_t& T2, const timestamp_t& T3, const timestamp_t& T4)
    {
        if (packet_count < NUMBER_OF_SAMPLES)
        {
            // Only store NUMBER_OF_SAMPLES samples
            StorageItem& storage_item = storage_array[packet_count];
            storage_item.T1 = T1;
            storage_item.T2 = T2;
            storage_item.T3 = T3;
            storage_item.T4 = T4;

            ++packet_count;
        }
    }

private:
    uint32_t packet_count;
    std::array<StorageItem, NUMBER_OF_SAMPLES> storage_array;
};


class StatisticsDataSet
{
public:
    StatisticsDataSet()
        : packet_count(0)
        , max_roundtrip(0)
        , min_roundtrip(999999)
        , sum_roundtrip(0)
    {
    }

    void appendTimestamps(const timestamp_t& T1, const timestamp_t& T2, const timestamp_t& T3, const timestamp_t& T4)
    {
        ++packet_count;

        // T1: Publisher Send
        // T2: Subscriber Receive 
        // T3: Subscriber Send 
        // T4: Publisher Receiver
        const timestamp_t roundtrip_time = (T4 - T1) - (T3 - T2);

        max_roundtrip = std::max(max_roundtrip, roundtrip_time);
        min_roundtrip = std::min(min_roundtrip, roundtrip_time);
        sum_roundtrip += roundtrip_time;
    }


public:
    timestamp_t getMinimumRoundtrip() const
    {
        return min_roundtrip;
    }

    timestamp_t getMaximumRoundtrip() const
    {
        return max_roundtrip;
    }

    timestamp_t getAverageRoundtrip() const
    {
        return sum_roundtrip / packet_count;
    }

    timestamp_t getSumRoundtrip() const
    {
        return sum_roundtrip;
    }

    uint32_t getPacketCount() const
    {
        return packet_count;
    }

private:
    uint32_t packet_count;
    timestamp_t max_roundtrip;
    timestamp_t min_roundtrip;
    timestamp_t sum_roundtrip;
};
//typedef StatisticsDataStorage<100> StatisticsDataStorageUsed;
typedef StatisticsDataStorage<10 * 1000> StatisticsDataStorageUsed;

class PerServerStatistics
{
    friend class GlobalStatistics;
public:
    PerServerStatistics()
        : old_packets(0)
        , usr_stats()
        , llv_stats()
        , rti_stats()
        , usr_data_storage(nullptr)
        , llv_data_storage(nullptr)
        , rti_data_storage(nullptr)
    {
        usr_data_storage = new StatisticsDataStorageUsed();
        llv_data_storage = new StatisticsDataStorageUsed();
        rti_data_storage = new StatisticsDataStorageUsed();
    }

    ~PerServerStatistics()
    {
        delete usr_data_storage;
        delete llv_data_storage;
        delete rti_data_storage;
    }

    void doneReceive(const timestamp_t& usr_recv_timestamp, const timestamp_t& start_timestamp, const sSampleHeader& received_sample, const SampleInfo& sample_info)
    {
        if (received_sample.usr_pub_send_timestamp < start_timestamp)
        {
            ++old_packets;
            //std::cerr << "!!! Error: Received old sample originating from Client" << received_sample.client_id << ":" << std::endl;
            //std::cerr << "    Sample timestamp " << received_sample.send_timestamp << " is older than start time " << start_timestamp << std::endl;
        }
        else
        {
            // Statistics for user space
            {
                // T1: Publisher Send
                const timestamp_t& T1 = received_sample.usr_pub_send_timestamp;

                // T2: Subscriber Receive 
                const timestamp_t& T2 = received_sample.usr_sub_recv_timestamp;

                // T3: Subscriber Send 
                const timestamp_t& T3 = received_sample.usr_sub_send_timestamp;

                // T4: Publisher Receiver
                const timestamp_t& T4 = usr_recv_timestamp;

                usr_stats.appendTimestamps(T1, T2, T3, T4);
                usr_data_storage->appendTimestamps(T1, T2, T3, T4);
            }

            // Statistics for llv space
            {
                // T1: Publisher Send
                const timestamp_t& T1 = received_sample.usr_pub_send_timestamp;

                // T2: Subscriber Receive 
                const timestamp_t& T2 = received_sample.llv_sub_recv_timestamp;

                // T3: Subscriber Send 
                const timestamp_t& T3 = received_sample.usr_sub_send_timestamp;

                // T4: Publisher Receiver
                const timestamp_t& T4 = sample_info.llv_reception_timestamp;

                llv_stats.appendTimestamps(T1, T2, T3, T4);
                llv_data_storage->appendTimestamps(T1, T2, T3, T4);
            }

            // Statistics the RTI way
            {
                // T1: Publisher Send
                const timestamp_t& T1 = received_sample.dds_pub_send_timestamp;

                // T2: Subscriber Receive 
                const timestamp_t& T2 = received_sample.dds_sub_recv_timestamp;

                // T3: Subscriber Send 
                const timestamp_t& T3 = sample_info.dds_source_timestamp;

                // T4: Publisher Receiver
                const timestamp_t& T4 = sample_info.dds_reception_timestamp;

                rti_stats.appendTimestamps(T1, T2, T3, T4);
                rti_data_storage->appendTimestamps(T1, T2, T3, T4);
            }
        }
    }

public:
    uint32_t old_packets;
    StatisticsDataSet usr_stats;
    StatisticsDataSet llv_stats;
    StatisticsDataSet rti_stats;
    StatisticsDataStorageUsed* usr_data_storage;
    StatisticsDataStorageUsed* llv_data_storage;
    StatisticsDataStorageUsed* rti_data_storage;
};

class GlobalStatistics
{
public:
    GlobalStatistics(const int number_of_servers)
        : m_number_of_servers(number_of_servers)
        , sent_packets(0)
        , m_start_timestamp()
    {
        m_start_timestamp= Helper::getCurrentMicroseconds();
    }

public:
    uint32_t getSampleNumber() const
    {
        return sent_packets;
    }

    void doneSend()
    {
        ++sent_packets;
    }

    void doneReceive(const timestamp_t& recv_timestamp, const sSampleHeader& received_sample, const SampleInfo& sample_info)
    {
        per_server[received_sample.server_id].doneReceive(recv_timestamp, m_start_timestamp, received_sample, sample_info);
    }
     

    friend std::ostream& operator<<(std::ostream& os, const GlobalStatistics& gs)
    {
        os << "Id;Sent;Received;Lost;Usr-MinRT;Usr-AvgRT;Usr-MaxRT;Llv-MinRT;Llv-AvgRT;Llv-MaxRT;Dds-MinRT;Dds-AvgRT;Dds-MaxRT;Errors" << std::endl;

        uint32_t sum_sent_packets = 0;
        uint32_t sum_received_packets = 0;
        uint32_t sum_lost_packets = 0;

        timestamp_t usr_total_min_roundtrip = 999999;
        timestamp_t usr_total_max_roundtrip = 0;
        timestamp_t usr_total_sum_roundtrip = 0;

        timestamp_t llv_total_min_roundtrip = 999999;
        timestamp_t llv_total_max_roundtrip = 0;
        timestamp_t llv_total_sum_roundtrip = 0;

        timestamp_t rti_total_min_roundtrip = 999999;
        timestamp_t rti_total_max_roundtrip = 0;
        timestamp_t rti_total_sum_roundtrip = 0;

        for (int i= 0; i< 32; ++i)
        {
            const PerServerStatistics& pss= gs.per_server[i];

            uint32_t sent_packets = gs.sent_packets;
            uint32_t received_packets = pss.usr_stats.getPacketCount();
            uint32_t lost_packets = sent_packets - received_packets;

            if (i < gs.m_number_of_servers)
            {
                sum_sent_packets += sent_packets;
                sum_received_packets += received_packets;
                sum_lost_packets += lost_packets;

                if (received_packets)
                {
                    os
                        << "Client" << i << ";"
                        << sent_packets << ";"
                        << received_packets << ";"
                        << lost_packets << ";";


                    // User statistics
                    {
                        const timestamp_t& min_roundtrip = pss.usr_stats.getMinimumRoundtrip();
                        const timestamp_t& avg_roundtrip = pss.usr_stats.getAverageRoundtrip();
                        const timestamp_t& max_roundtrip = pss.usr_stats.getMaximumRoundtrip();

                        usr_total_min_roundtrip = std::min(usr_total_min_roundtrip, min_roundtrip);
                        usr_total_max_roundtrip = std::max(usr_total_max_roundtrip, max_roundtrip);
                        usr_total_sum_roundtrip += pss.usr_stats.getSumRoundtrip();

                        os
                            << min_roundtrip << ";"
                            << avg_roundtrip << ";"
                            << max_roundtrip << ";";
                    }

                    // Low-Level statistics
                    {
                        const timestamp_t& min_roundtrip = pss.llv_stats.getMinimumRoundtrip();
                        const timestamp_t& avg_roundtrip = pss.llv_stats.getAverageRoundtrip();
                        const timestamp_t& max_roundtrip = pss.llv_stats.getMaximumRoundtrip();

                        llv_total_min_roundtrip = std::min(llv_total_min_roundtrip, min_roundtrip);
                        llv_total_max_roundtrip = std::max(llv_total_max_roundtrip, max_roundtrip);
                        llv_total_sum_roundtrip += pss.llv_stats.getSumRoundtrip();

                        os
                            << min_roundtrip << ";"
                            << avg_roundtrip << ";"
                            << max_roundtrip << ";";
                    }

                    // DDS statistics
                    {
                        const timestamp_t& min_roundtrip = pss.rti_stats.getMinimumRoundtrip();
                        const timestamp_t& avg_roundtrip = pss.rti_stats.getAverageRoundtrip();
                        const timestamp_t& max_roundtrip = pss.rti_stats.getMaximumRoundtrip();

                        rti_total_min_roundtrip = std::min(rti_total_min_roundtrip, min_roundtrip);
                        rti_total_max_roundtrip = std::max(rti_total_max_roundtrip, max_roundtrip);
                        rti_total_sum_roundtrip += pss.rti_stats.getSumRoundtrip();

                        os
                            << min_roundtrip << ";"
                            << avg_roundtrip << ";"
                            << max_roundtrip << ";";
                    }

                    if (pss.old_packets)
                    {
                        os << "!!! Reception of " << pss.old_packets << " old packets (From previous run) !!!";
                    }
                    else
                    {
                        //os << "None";
                    }
                    os    << std::endl;
                }
                else
                {
                    os
                        << "Client" << i << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << "!!! Nothing Received (Server might not be running) !!!"
                        << std::endl;
                }
            }
            else
            {
                if (received_packets)
                {
                    os
                        << "Client" << i << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << ";"
                        << "!!! Reception of " << received_packets << " was not expected !!!"
                        << std::endl;
                }
            }
        }

        os
            << "Summary" << ";"
            << sum_sent_packets << ";"
            << sum_received_packets << ";"
            << sum_lost_packets << ";"
            << usr_total_min_roundtrip << ";"
            << (usr_total_sum_roundtrip / sum_received_packets) << ";"
            << usr_total_max_roundtrip << ";"
            << llv_total_min_roundtrip << ";"
            << (llv_total_sum_roundtrip / sum_received_packets) << ";"
            << llv_total_max_roundtrip << ";"
            << rti_total_min_roundtrip << ";"
            << (rti_total_sum_roundtrip / sum_received_packets) << ";"
            << rti_total_max_roundtrip << ";"
            << std::endl;

        return os;
    }

    void writeDetailsAsCSV(std::ostream& os)
    {
        os << "Server-Id;Sample-Index;Usr-T1;Usr-T2;Usr-T3;Usr-T4;Llv-T1;Llv-T2;Llv-T3;Llv-T4;Rti-T1;Rti-T2;Rti-T3;Rti-T4;" << std::endl;

        // Packet count is equal for all storage (by definition)
        uint32_t packet_count = per_server[0].usr_data_storage->packet_count;

        for (std::size_t i = 0; i< packet_count; ++i)
        {
            for (int server_id = 0; server_id < m_number_of_servers; ++server_id)
            {
                const PerServerStatistics& pss = per_server[server_id];
                const StatisticsDataStorageUsed& usr_data_storage = *pss.usr_data_storage;
                const StatisticsDataStorageUsed& llv_data_storage = *pss.llv_data_storage;
                const StatisticsDataStorageUsed& rti_data_storage = *pss.rti_data_storage;
              
                const StatisticsDataStorageUsed::StorageItem& usr_storage_item = usr_data_storage.storage_array[i];
                const StatisticsDataStorageUsed::StorageItem& llv_storage_item = llv_data_storage.storage_array[i];
                const StatisticsDataStorageUsed::StorageItem& rti_storage_item = rti_data_storage.storage_array[i];

                os
                    << server_id << ";"
                    << i << ";"
                    << usr_storage_item.T1 << ";"
                    << usr_storage_item.T2 << ";"
                    << usr_storage_item.T3 << ";"
                    << usr_storage_item.T4 << ";"
                    << llv_storage_item.T1 << ";"
                    << llv_storage_item.T2 << ";"
                    << llv_storage_item.T3 << ";"
                    << llv_storage_item.T4 << ";"
                    << rti_storage_item.T1 << ";"
                    << rti_storage_item.T2 << ";"
                    << rti_storage_item.T3 << ";"
                    << rti_storage_item.T4 << ";"
                    << std::endl;
            }
        }
    }

private:
    int m_number_of_servers;
    uint32_t sent_packets;
    timestamp_t m_start_timestamp;
    PerServerStatistics per_server[32];
};

class ClientElement : public ISubscriberListener
{
public:
    ClientElement(const DdsConfig& dds_config, const int client_id, const int number_of_servers, const std::string& name)
        : m_client_id(static_cast<int16_t>(client_id))
        , m_global_statistics(number_of_servers)
    {
        m_participant = new Participant(dds_config, name);

        m_topic_ping = new Topic(*m_participant, "Ping");
        m_topic_pong = new Topic(*m_participant, "Pong");

        m_subscriber = new Subscriber(dds_config, this, *m_participant, *m_topic_pong);
        m_publisher = new Publisher(dds_config, *m_participant, *m_topic_ping);

        m_publisher_sample = new PublisherSample(*m_publisher, dds_config.sample_size);

        m_participant->Start();
    }

    ~ClientElement()
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
    void sendSample()
    {
        sSampleHeader* sample_header = m_publisher_sample->getPtr<sSampleHeader>();
        sample_header->sample_number = m_global_statistics.getSampleNumber();
        sample_header->client_id = m_client_id;
        sample_header->server_id = static_cast<uint16_t>(-1);
        sample_header->usr_pub_send_timestamp = Helper::getCurrentMicroseconds();
        
        m_publisher->Transmit(m_publisher_sample);
        m_global_statistics.doneSend();
    }

public:
    void receivedSample(const void* data, const int data_size, const SampleInfo& sample_info)
    {
        timestamp_t usr_recv_timestamp = Helper::getCurrentMicroseconds();
        const sSampleHeader* input_sample_header = reinterpret_cast<const sSampleHeader*>(data);

        m_global_statistics.doneReceive(usr_recv_timestamp, *input_sample_header, sample_info);
    }

    friend std::ostream& operator<<(std::ostream& os, const ClientElement& ce)
    {
        os << ce.m_global_statistics;

        return os;
    }

    void writeDetailsAsCSV(std::ostream& os)
    {
        m_global_statistics.writeDetailsAsCSV(os);
    }

private:
    int16_t m_client_id;
    Participant* m_participant;
    Topic* m_topic_ping;
    Topic* m_topic_pong;
    Subscriber* m_subscriber;
    Publisher* m_publisher;
    PublisherSample* m_publisher_sample;
    GlobalStatistics m_global_statistics;
};

static void usage(const char* progname)
{
    std::cerr << "Usage: " << progname << " <size> <mode> <multicast> <async> <number-of-servers> <frequency> [<name-of-measurement>]" << std::endl;
    std::cerr << "  <size> (mandatory) sample size to use. default is 1024." << std::endl;
    std::cerr << "  <mode> (mandatory) is 0 (BEST_EFFORT) or 1 (RELIABLE)" << std::endl;
    std::cerr << "  <multicast> (mandatory) is 0 (UNICAST) or 1 (MULTICAST)" << std::endl;
    std::cerr << "  <async> (mandatory) is 0 (SYNC) or 1 (ASYNC)" << std::endl;
    std::cerr << "  <number-of-servers> (mandatory) number of servers expected 1 to 32" << std::endl;
    std::cerr << "  <frequency> (mandatory) is number 1 ... 1000000" << std::endl;
    std::cerr << "  <name-of-measurement> (optional) append this text to measure file" << std::endl;
}

int main(int argc, char* argv[])
{
    int client_id = 0;
    int number_of_servers = 1;
    int runtime_s = 10;

    if (argc < 7 || argc > 8)
    {
        usage(argv[0]);
        return 1;
    }

    DdsConfig dds_config;
    dds_config.multicast_enable = false;
    dds_config.sample_size = atoi(argv[1]);
    int reliability_mode_num= atoi(argv[2]);
    dds_config.reliability_mode = (reliability_mode_num == 0) ? BestEffortReliabilityMode : ReliableReliabilityMode;
    int multicast_mode_num = atoi(argv[3]);
    dds_config.multicast_enable = (multicast_mode_num != 0);
    int async_mode_num = atoi(argv[4]);
    dds_config.async_mode_enable = (async_mode_num != 0);

    number_of_servers = atoi(argv[5]);

    int frequency = atoi(argv[6]);

    std::string optional_name = "";
    if (argv[7])
    {
        optional_name = argv[7];
    }

    if (frequency < 0 || frequency > 1000000)
    {
        usage(argv[0]);
        return 1;
    }

    if (reliability_mode_num < 0 || reliability_mode_num > static_cast<int>(MaximumNumberForReliabilityMode))
    {
        usage(argv[0]);
        return 1;
    }

    if (number_of_servers < 1 || number_of_servers > 32)
    {
        usage(argv[0]);
        return 1;
    }

    int32_t sleep_time_us = 1000000 / frequency;
    int number_of_samples = runtime_s * frequency;
    
    std::string output_summary_filename = "";
    std::string output_details_filename = "";

    if (!optional_name.empty())
    {
        std::string output_filename_base = 
            std::string() +
#if RTI_DDS_USE_WAITSET
            "results_waitset"
#else
            "results_queue"
#endif
#if DDS_WITH_MULTICAST_ENABLED
            + "_" + "mc" +
#else
            + "_" + "uc" +
#endif
            + "_" + "s" + std::to_string(number_of_servers)
            + "_" + "m" + std::to_string(reliability_mode_num)
            + "_" + "f" + std::to_string(frequency)
            + "_" + s_dds_variant_id_string
            + "_" + optional_name;

        output_summary_filename = output_filename_base + ".csv";
        output_details_filename = output_filename_base + "-details.csv";
    }

    ClientElement client_element(dds_config, client_id, number_of_servers, std::string("Client") + std::to_string(client_id));

    // Initial sleep ... wait to settle
    usleep(1 * 1000000);

    timestamp_t next_sample_timestamp= Helper::getCurrentMicroseconds();
    for (int i= 0; i< number_of_samples; ++i)
    {
        timestamp_t diff_timestamp = next_sample_timestamp - Helper::getCurrentMicroseconds();
        if (diff_timestamp > 0)
        {
            usleep(diff_timestamp);
        }
        next_sample_timestamp+= sleep_time_us;

        client_element.sendSample();
    }

    // Final sleep ... receive last packets. 1 Second must be enough.
    usleep(1 * 1000000);

    std::cout << client_element << std::endl;

    if (!output_summary_filename.empty())
    {
        std::ofstream ofs;
        ofs.open(output_summary_filename, std::ofstream::out | std::ofstream::trunc);
        ofs << client_element << std::endl;
        ofs.close();
    }

    if (!output_details_filename.empty())
    {
        std::ofstream ofs;
        ofs.open(output_details_filename, std::ofstream::out | std::ofstream::trunc);
        client_element.writeDetailsAsCSV(ofs);
        ofs.close();
    }

    return 0;
}
