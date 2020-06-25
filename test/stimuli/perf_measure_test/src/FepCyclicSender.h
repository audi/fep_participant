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
#ifndef __FepCyclicSender_h__
#define __FepCyclicSender_h__

#include "stdafx.h"


class FepCyclicSender: public FepCyclicSenderBase, public fep::IUserDataListener
{
public:
    FepCyclicSender(fep::IModule* pModule, uint32_t nClientId, uint32_t nServerId, FepElementMode eMode, timestamp_t nPeriod, size_t szNumberOfPacketsPerCycle, uint32_t nExpectedPackets, handle_t hRecvHandle, handle_t hSendHandle);
    ~FepCyclicSender();

public:
    ::impl::FepElementStats* GetStatistics()
    {
        return &m_tStatistic;
    }

    fep::Result ProcessRunningEntry(const fep::tState eOldState);
    fep::Result ProcessIdleEntry(const fep::tState eOldState);

public:
    void RunCyclic();

public: // implements IUserDataListener
    fep::Result Update(const fep::IUserDataSample* poSample);
private:
    ::impl::FepElementStats m_tStatistic;
    a_util::system::Timer m_oTimer;
};


#endif // __FepCyclicSender_h__
