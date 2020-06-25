/**
 * Implementation of the Class FepElement.
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

#include "stdafx.h"
#include "perfmeasure/perfmeasure.h"
#include <algorithm>

FepCyclicSender::FepCyclicSender(fep::IModule* pModule, uint32_t nClientId, uint32_t nServerId, FepElementMode eMode, timestamp_t nPeriod, size_t szNumberOfPacketsPerCycle, uint32_t nExpectedPackets, handle_t hRecvHandle, handle_t hSendHandle)
  : FepCyclicSenderBase(pModule, nClientId, nServerId, eMode, nPeriod, szNumberOfPacketsPerCycle, nExpectedPackets, hRecvHandle, hSendHandle)
  , m_tStatistic(nServerId+1, nExpectedPackets)
{
    if(NULL != m_pModule)
    {
        fep::Result nResult= ERR_NOERROR;

        switch (m_eMode)
        {
            case ReceiverMode:
            case ServerMode:
                assert(m_hRecvHandle);
                nResult |= m_pModule->GetUserDataAccess()->RegisterDataListener(this, m_hRecvHandle);
                break;
            case SenderMode:
                assert(hSendHandle);
                m_oTimer.setCallback(&FepCyclicSender::RunCyclic, *this);
                m_oTimer.setPeriod(m_nPeriod);
                break;
            case  ClientMode:
                assert(m_hRecvHandle);
                nResult |= m_pModule->GetUserDataAccess()->RegisterDataListener(this, m_hRecvHandle);
                assert(hSendHandle);
                m_oTimer.setCallback(&FepCyclicSender::RunCyclic, *this);
                m_oTimer.setPeriod(m_nPeriod);
                break;
        }
        if(fep::isFailed(nResult))
        {
            std::cerr << "Something went wrong when creating FepCyclicSender" << std::endl;
        }
    }
}

FepCyclicSender::~FepCyclicSender()
{
    if(NULL != m_pModule)
    {
        fep::Result nResult= ERR_NOERROR;

        switch (m_eMode)
        {
            case ReceiverMode:
            case ServerMode:
                assert(m_hRecvHandle);
                nResult |= m_pModule->GetUserDataAccess()->UnregisterDataListener(this, m_hRecvHandle);
                break;
            case SenderMode:
                break;
            case ClientMode:
                assert(m_hRecvHandle);
                nResult |= m_pModule->GetUserDataAccess()->UnregisterDataListener(this, m_hRecvHandle);
                break;
        }
        if(fep::isFailed(nResult))
        {
            std::cerr << "Something went wrong when destroying FepCyclicSender" << std::endl;
        }
    }
}

void FepCyclicSender::RunCyclic()
{
    fep::Result nResult= ERR_NOERROR;

    if(m_nPacketsToBeSend > m_tStatistic.m_nSentPacketCount)
    {
        for (size_t l = 0; l < m_szNumberOfPacketsPerCycle; ++l)
        {

            // Sending data in regular intervals
            switch (m_eMode)
            {
                case ClientMode:
                case SenderMode:
                {
                    size_t nSeqNr= (m_tStatistic.m_nSentPacketCount)++;

                    t_Ping* pPing = reinterpret_cast<t_Ping*>(m_pSendSample->GetPtr());
                    a_util::memory::set(pPing, sizeof(t_Ping), 0, sizeof(t_Ping));
                    pPing->nMagic = PING_MAGIC_NUMBER;
                    pPing->nSeqNr = static_cast<uint32_t>(nSeqNr);
                    pPing->nClientId = m_nClientId;
                    pPing->nServerId = static_cast<uint32_t>(-1); // Initialize with invalid value
                    pPing->tm01ClientSend= GetHighResTime();

                    MEASURE_POINT_SET(IndexOfSendPacket, nSeqNr);
                    MEASURE_POINT(ElementTransmitCalled);

                    nResult = m_pModule->GetUserDataAccess()->TransmitData(m_pSendSample, true);
                    pPing->tm01ClientSendFinish = GetHighResTime();

                    timestamp_t before_transmit = pPing->tm01ClientSend;
                    timestamp_t transmit_time = pPing->tm01ClientSendFinish - before_transmit;

                    for (std::size_t j = 0; j < m_tStatistic.m_poPerServerStats.size(); ++j)
                    {
                        impl::FepPerServerStats*& poPerServerStats = m_tStatistic.m_poPerServerStats[j];

                        assert(nSeqNr < poPerServerStats->m_nPingPackets);
                        if (nSeqNr < poPerServerStats->m_nPingPackets)
                        {
                            a_util::memory::copy(&(poPerServerStats->m_pPingPackets[nSeqNr]), sizeof(t_Ping), pPing, sizeof(t_Ping));
                        }
                    }


                    m_tStatistic.m_nTransmitTimeTotal += transmit_time;
                    m_tStatistic.m_nTransmitTimeMin = std::min(m_tStatistic.m_nTransmitTimeMin, transmit_time);
                    m_tStatistic.m_nTransmitTimeMax = std::max(m_tStatistic.m_nTransmitTimeMax, transmit_time);
                    if (fep::isFailed(nResult))
                    {
                        std::cerr << "Something went wrong when transmittig data" << std::endl;
                        return;
                    }
                }
                break;
            default:
                break;
            }
        }
    }
}

fep::Result FepCyclicSender::Update(const fep::IUserDataSample* poSample)
{
    fep::Result nResult = ERR_NOERROR;

    timestamp_t tmCurrent = GetHighResTime(); // A_UTILS_NS::cSystem::GetTime();

    // Sending data in regular intervals
    switch (m_eMode)
    {
        case ServerMode:
            {
                // FIXME: This is extremly ugly, but do not see another way to convert the sample
                const fep::IPreparationDataSample* pSyncSource = reinterpret_cast<const fep::IPreparationDataSample*>(poSample);
                bool bSyncFlag = pSyncSource->GetSyncFlag();

                //++(m_tStatistic.m_nReceivedPacketCount);

                const t_Ping* pPingSource = reinterpret_cast<const t_Ping*>(poSample->GetPtr());
                t_Ping* pPingDest = reinterpret_cast<t_Ping*>(m_pSendSample->GetPtr());

                // *pPingDest= *pPingSource;
                a_util::memory::copy(pPingDest, sizeof(t_Ping), pPingSource, sizeof(t_Ping));

                ++(m_tStatistic.m_nSentPacketCount);

                pPingDest->nClientId = pPingSource->nClientId;
                pPingDest->nServerId = m_nServerId;
                pPingDest->tm02ServerRecv= tmCurrent;
                pPingDest->tm03ServerSend= GetHighResTime(); // A_UTILS_NS::cSystem::GetTime();

                nResult = m_pModule->GetUserDataAccess()->TransmitData(m_pSendSample, bSyncFlag);
                //CHECK_ERROR("Transmit Data");
            }
            break;
        case ClientMode:
            {
                MEASURE_POINT(ElementReceivedCalled);
                const t_Ping* pPingSource= reinterpret_cast<const t_Ping*>(poSample->GetPtr());

                size_t nSeqNr= pPingSource->nSeqNr;
                MEASURE_POINT_SET(IndexOfReceivedPacket, nSeqNr);

                assert(pPingSource->nServerId < m_tStatistic.m_poPerServerStats.size());
                impl::FepPerServerStats*& poPerServerStats = m_tStatistic.m_poPerServerStats[pPingSource->nServerId];
                assert(nSeqNr < poPerServerStats->m_nPingPackets);
                t_Ping* pPing= &(poPerServerStats->m_pPingPackets[nSeqNr]);

                //std::cerr << "Received sequence Nr " << nSeqNr << " Current value " << pPing->tm04ClientRecv << std::endl;
                //if (0 && pPing->tm04ClientRecv) // FIXME: Currently there is always a value in there
                //{
                //    //FIXME: write something here --> outsource VERBOSE method
                //    //VERBOSE(1, 1 << " Duplicated packet. Ignoring it.");
                //}
                //else
                //{
                    const timestamp_t ti_send_finish = pPing->tm01ClientSendFinish;
                    a_util::memory::copy(pPing, sizeof(t_Ping), pPingSource, sizeof(t_Ping));
                    pPing->tm01ClientSendFinish = ti_send_finish;

                    pPing->tm04ClientRecv= tmCurrent;
                    ++(poPerServerStats->m_nReceivedPacketCount);
                //}
            }
            break;
        default:
            break;
    }

    return nResult;
}

fep::Result FepCyclicSender::ProcessRunningEntry(const fep::tState eOldState)
{
    m_oTimer.start();
    return fep::ERR_NOERROR;
}

fep::Result FepCyclicSender::ProcessIdleEntry(const fep::tState eOldState)
{
    m_oTimer.stop();
    return fep::ERR_NOERROR;
}
