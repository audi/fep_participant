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
#ifndef __FepCyclicSenderBase_h__
#define __FepCyclicSenderBase_h__

class FepCyclicSenderBase : fep::cStateEntryListener
{
    protected:
        FepCyclicSenderBase(fep::IModule* pModule, uint32_t nClientId, uint32_t nServerId, FepElementMode eMode, timestamp_t nPeriod, size_t szNumberOfPacketsPerCycle, uint64_t nExpectedPackets, handle_t hRecvHandle, handle_t hSendHandle);

    public:
        virtual ~FepCyclicSenderBase();

        handle_t GetRecvHandle()
        {
            return m_hRecvHandle;
        }


protected:
    fep::IModule* m_pModule;
    uint32_t m_nClientId;
    uint32_t m_nServerId;
    timestamp_t m_nPeriod;
    size_t m_szNumberOfPacketsPerCycle;
    uint64_t m_nPacketsToBeSend;
    uint16_t m_nSignalId;
    handle_t m_hSendHandle;
    handle_t m_hRecvHandle;
    FepElementMode m_eMode;
    size_t m_szUpdateCounter;
    fep::IUserDataSample* m_pSendSample;
};

#endif // __FepCyclicSenderBase_h__
