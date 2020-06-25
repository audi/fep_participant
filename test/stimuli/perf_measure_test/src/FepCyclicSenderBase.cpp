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

#include <iostream>

FepCyclicSenderBase::FepCyclicSenderBase(fep::IModule* pModule, uint32_t nClientId, uint32_t nServerId, FepElementMode eMode, timestamp_t nPeriod, size_t szNumberOfPacketsPerCycle, uint64_t nExpectedPackets, handle_t hRecvHandle, handle_t hSendHandle)
    : m_pModule(pModule)
    , m_nClientId(nClientId)
    , m_nServerId(nServerId)
    , m_nPeriod(nPeriod)
    , m_szNumberOfPacketsPerCycle(szNumberOfPacketsPerCycle)
    , m_nPacketsToBeSend(nExpectedPackets)
    , m_nSignalId(0)
    , m_hSendHandle(hSendHandle)
    , m_hRecvHandle(hRecvHandle)
    , m_eMode(eMode)
    , m_szUpdateCounter(0)
    , m_pSendSample(NULL)
{
    if(NULL != m_pModule && NULL != m_hSendHandle)
    {
        fep::Result nResult= ERR_NOERROR;

        nResult = m_pModule->GetUserDataAccess()->CreateUserDataSample(m_pSendSample, m_hSendHandle);
        t_Ping* pPing = reinterpret_cast<t_Ping*>(m_pSendSample->GetPtr());
        (void) pPing;
        
        if (fep::isFailed(nResult))
        {
            std::cerr << "Error: Failed to create user data sample" << std::endl;
        }
        else
        {
            nResult |= m_pModule->GetStateMachine()->RegisterStateEntryListener(this);
            if (fep::isFailed(nResult))
            {
                std::cerr << "Error: Failed to register state entry listener" << std::endl;
            }
        }
    }
}

FepCyclicSenderBase::~FepCyclicSenderBase()
{
    fep::Result nResult = ERR_NOERROR;

    nResult |= m_pModule->GetStateMachine()->UnregisterStateEntryListener(this);
    if (fep::isFailed(nResult))
    {
        std::cerr << "Error: Failed to unregister state entry listener" << std::endl;
    }
    delete m_pSendSample;
}
