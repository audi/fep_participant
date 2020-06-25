/**
 * Declaration of the Class FepElement.
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
#ifndef __Common_h__
#define __Common_h__

#include "a_util/memory.h"

// Check for C++11 features
#if __cplusplus >= 201103L
#define CPP_DECL_OVERRIDE override
#else
#define CPP_DECL_OVERRIDE
#endif

#define PING_MAGIC_NUMBER 0xBEAF4FEE
#pragma pack(push,1)
struct t_Ping
{
    uint32_t nMagic;
    uint32_t nSeqNr;
    uint32_t nSize;
    bool bSyncFlag;
    bool bDummy1;
    bool bDummy2;
    bool bDummy3;
    uint32_t nClientId;
    uint32_t nServerId;
    timestamp_t tm01ClientSend;
    timestamp_t tm02ServerRecv;
    timestamp_t tm03ServerSend;
    timestamp_t tm04ClientRecv;
    timestamp_t tm01ClientSendFinish;
    unsigned char data[1];
};
#pragma pack(pop)

#pragma pack(push,1)
struct t_Frame
{
    uint32_t nFrameNr;
    timestamp_t tm01ClientSend;
    timestamp_t tm02ClientRecv;
    bool bIsComplete;
    uint32_t nValidSampleCount;
};
#pragma pack(pop)

#define INVALID_SEQ_NR static_cast<size_t>(-1)

namespace impl {
    struct FepElementPrivate;
    struct FepElementConfig;

    struct FepPerServerStats
    {
        size_t m_nReceivedPacketCount;
        size_t m_nPingPackets;
        t_Ping* m_pPingPackets;
    public:
        FepPerServerStats(uint32_t nExpectedPackets)
            : m_nReceivedPacketCount(0)
        {
            m_nPingPackets = nExpectedPackets;
            m_pPingPackets = new t_Ping[m_nPingPackets];
            for (size_t i = 0; i< m_nPingPackets; ++i)
            {

                t_Ping* pPing = &(m_pPingPackets[i]);
                a_util::memory::set(pPing, sizeof(t_Ping), 0, sizeof(t_Ping));
            }
        }
        ~FepPerServerStats()
        {
            delete[] m_pPingPackets;
        }

    };

    struct FepElementStats
    {
        size_t m_nSentPacketCount;
        timestamp_t m_nTransmitTimeTotal;
        timestamp_t m_nTransmitTimeMin;
        timestamp_t m_nTransmitTimeMax;
        uint32_t m_nServerCount;
        std::vector<FepPerServerStats*> m_poPerServerStats;

   public:
        FepElementStats(uint32_t nServerCount, uint32_t nExpectedPackets)
            : m_nSentPacketCount(0)
            , m_nTransmitTimeTotal(0)
            , m_nTransmitTimeMin(999999)
            , m_nTransmitTimeMax(0)
            , m_nServerCount(0)
        {
            m_poPerServerStats.resize(nServerCount);
            for (uint32_t i= 0; i< nServerCount; ++i)
            {
                m_poPerServerStats[i] = new FepPerServerStats(nExpectedPackets);
            }
        }
        ~FepElementStats()
        {
            for (uint32_t i = 0; i< m_poPerServerStats.size(); ++i)
            {
                delete m_poPerServerStats[i];
            }
        }
    };
    struct FepElementDDBStats
    {
        size_t m_nSentFrameCount;
        size_t m_nReceivedFrameCount;
        size_t m_nFrames;
        t_Frame* m_pFrames;
    public:
        FepElementDDBStats(uint32_t nExpectedFrames)
            : m_nSentFrameCount(0)
            , m_nReceivedFrameCount(0)
        {
            m_nFrames = nExpectedFrames;
            m_pFrames = new t_Frame[m_nFrames];
            for (size_t i = 0; i < m_nFrames; ++i)
            {
                t_Frame* pFrame = &(m_pFrames[i]);
                a_util::memory::set(pFrame, sizeof(t_Frame), 0, sizeof(t_Frame));
            }
        }
        ~FepElementDDBStats()
        {
            delete[] m_pFrames;
        }
    };

    struct FepSignalConfig;
} // namespace impl

enum FepElementMode {
    ReceiverMode,
    SenderMode,
    ClientMode,
    ServerMode
};

timestamp_t GetHighResTime();

#endif // __Common_h__

