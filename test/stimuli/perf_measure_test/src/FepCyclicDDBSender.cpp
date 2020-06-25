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
#include "a_util/system.h"

FepCyclicDDBSender::FepCyclicDDBSender(fep::IModule* pModule, uint32_t nClientId, uint32_t nServerId, FepElementMode eMode, uint16_t nPeriod, size_t szNumberOfPacketsPerCycle, uint32_t nExpectedPackets, handle_t hRecvHandle, handle_t hSendHandle, fep::IDDBAccess* poDDBAccess)
    : FepCyclicSenderBase(pModule, nClientId, nServerId, eMode, nPeriod, szNumberOfPacketsPerCycle, nExpectedPackets, hRecvHandle, hSendHandle)
    , m_poDDBAccess(poDDBAccess), m_tStatistic(nExpectedPackets)
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
            case ClientMode:
            case SenderMode:
                assert(m_hSendHandle);
                m_oTimer.setCallback(&FepCyclicDDBSender::RunCyclic, *this);
                m_oTimer.setPeriod(m_nPeriod);
                nResult |= m_poDDBAccess->RegisterSyncListener(this);
                break;
        }
        if(fep::isFailed(nResult))
        {
            std::cerr << "Something went wrong when creating FepCyclicSender" << std::endl;
        }
    }
}

FepCyclicDDBSender::~FepCyclicDDBSender()
{
    if(NULL != m_pModule)
    {
        fep::Result nResult= ERR_NOERROR;

        switch (m_eMode)
        {
            case ReceiverMode:
            case ServerMode:
                nResult |= m_pModule->GetUserDataAccess()->UnregisterDataListener(this, m_hRecvHandle);
                break;
            case ClientMode:
            case SenderMode:
                nResult |= m_poDDBAccess->UnregisterSyncListener(this);
                break;
        }
        if(fep::isFailed(nResult))
        {
            std::cerr << "Something went wrong when creating FepCyclicSender" << std::endl;
        }
    }

}

void FepCyclicDDBSender::RunCyclic()
{
    fep::Result nResult= ERR_NOERROR;
    if(m_nPacketsToBeSend > m_tStatistic.m_nSentFrameCount)
    {
        for (size_t l = 0; l < m_szNumberOfPacketsPerCycle; ++l)
        {

            // Sending data in regular intervals
            switch (m_eMode)
            {
            case ReceiverMode:
            case ServerMode:
                break;
            case ClientMode:
            case SenderMode:
                {
                    if (0 == l)
                    {
                        size_t nFrameNr = m_tStatistic.m_nSentFrameCount;

                        assert(nFrameNr < m_tStatistic.m_nFrames);
                        m_tStatistic.m_pFrames[nFrameNr].tm01ClientSend = GetHighResTime();
                    }

                    t_Ping* pPing = reinterpret_cast<t_Ping*>(m_pSendSample->GetPtr());
                    a_util::memory::set(pPing, sizeof(t_Ping), 0, sizeof(t_Ping));

                    pPing->nSeqNr = static_cast<uint32_t>(m_tStatistic.m_nSentFrameCount);
                    pPing->nClientId = m_nClientId;
                    pPing->nServerId = static_cast<uint32_t>(-1); // Initialize with invalid value

                    bool bSyncFlag= ((m_szNumberOfPacketsPerCycle - 1) == l);

                    pPing->bSyncFlag = bSyncFlag;
                    nResult = m_pModule->GetUserDataAccess()->TransmitData(m_pSendSample, bSyncFlag);

                    if (bSyncFlag)
                    {
                        //std::cerr << "Sending seq " << m_tStatistic.m_nSentFrameCount << std::endl;
                        ++(m_tStatistic.m_nSentFrameCount);
                    }
                }
                break;
            }
            a_util::system::sleepMilliseconds(1);
        }
    }
}

fep::Result FepCyclicDDBSender::ProcessDDBSync(handle_t const hSignal, const fep::IDDBFrame& oDDBFrame)
{
    switch(m_eMode)
    {
    case ReceiverMode:
    case ServerMode:
        break;
    case ClientMode:
    case SenderMode:
        {
            t_Ping* pPing = NULL;
            timestamp_t tmCurrent = GetHighResTime();

            for(uint32_t i = 0; i < m_szNumberOfPacketsPerCycle; ++i)
            {
                if(true == oDDBFrame.IsValidSample(i))
                {
                    pPing = reinterpret_cast<t_Ping*> (oDDBFrame.GetSample(i)->GetPtr());
                    break;
                }
            }
            if (pPing != NULL)
            {
                pPing->nClientId = m_nClientId;
                pPing->nServerId = static_cast<uint32_t>(-1); // Initialize with invalid value

                uint32_t nFrameNr = pPing->nSeqNr;
                //std::cerr << "Frame: " << m_tStatistic.m_nReceivedFrameCount << " Received sync for " << nFrameNr << " (max is " << m_tStatistic.m_nFrames << ")" << std::endl;

                assert(nFrameNr < m_tStatistic.m_nFrames);
                m_tStatistic.m_pFrames[nFrameNr].tm02ClientRecv = tmCurrent;
                m_tStatistic.m_pFrames[nFrameNr].bIsComplete = oDDBFrame.IsComplete();
                m_tStatistic.m_pFrames[nFrameNr].nValidSampleCount = static_cast<uint32_t>(oDDBFrame.GetValidCount());
                (m_tStatistic.m_nReceivedFrameCount)++;
            }
        }
    }

    //VERBOSE(1, "received sync after " << nSampleCnt << " packets");
    //TODO: find something to do here
    return ERR_NOERROR;
}

fep::Result FepCyclicDDBSender::Update(const fep::IUserDataSample *poSample)
{
    fep::Result nResult = ERR_NOERROR;
    switch(m_eMode)
    {
    case ClientMode:
    case SenderMode:
        break;
    case ReceiverMode:
    case ServerMode:
        {

            const t_Ping* pSource = reinterpret_cast<const t_Ping*>(poSample->GetPtr());
            t_Ping* pDest = reinterpret_cast<t_Ping*>(m_pSendSample->GetPtr());

            pDest->nClientId = pSource->nClientId;
            pDest->nServerId = static_cast<uint32_t>(-1); // Initialize with invalid value
            pDest->nSeqNr = m_nServerId;
            pDest->bSyncFlag = pSource->bSyncFlag;

            if(true == pDest->bSyncFlag)
            {
                (m_tStatistic.m_nReceivedFrameCount)++;
                (m_tStatistic.m_nSentFrameCount)++;
            }

            nResult = m_pModule->GetUserDataAccess()->TransmitData(m_pSendSample, pDest->bSyncFlag);
            if (fep::isFailed(nResult))
            {
                std::cerr << "Something went wrong when transmittig data" << std::endl;
                return nResult;
            }
        }
        break;
    }
    return nResult;
}

fep::Result FepCyclicDDBSender::ProcessRunningEntry(const fep::tState eOldState)
{
    m_oTimer.start();
    return fep::ERR_NOERROR;
}

fep::Result FepCyclicDDBSender::ProcessIdleEntry(const fep::tState eOldState)
{
    m_oTimer.stop();
    return fep::ERR_NOERROR;
}
