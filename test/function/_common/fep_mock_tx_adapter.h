/**
 * Implementation of tx adapter mockup used by FEP functional test cases!
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

#ifndef _FEP_TEST_MOCK_TX_ADAPTER_H_INC_
#define _FEP_TEST_MOCK_TX_ADAPTER_H_INC_

#include "transmission_adapter/fep_transmission.h"

using namespace fep;

class cMockTxAdapter : public fep::ITransmissionAdapter, public fep::ITransmissionAdapterPrivate
{
    virtual fep::Result GetRecentSample(handle_t hSignalHandle,
        fep::IPreparationDataSample* pSample) const
    {
            return ERR_NOERROR;
    }

    virtual fep::Result RegisterDataListener(fep::IPreparationDataListener* poDataListener,
        handle_t hSignalHandle)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result RegisterSignal(const tSignal& oSignal,
        handle_t& hSignalHandle)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result TransmitData(fep::IPreparationDataSample* poPreparationSample)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result UnregisterDataListener(fep::IPreparationDataListener* poDataListener,
        const handle_t hSignalHandle)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result UnregisterSignal(handle_t hSignalHandle)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result GetTxInfo(size_t &szCntTxSlotsFree, size_t &szCntTxSlotsUsed,
        handle_t hSignalHandle)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result TransmitCommand(ICommand* poCommand)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result RegisterCommandListener(ICommandListener * const poCommandListener)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result UnregisterCommandListener(ICommandListener * const poCommandListener)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result TransmitNotification(INotification const * pNotification)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result RegisterNotificationListener(
        INotificationListener* poNotificationListener)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result UnregisterNotificationListener(
        INotificationListener* poNotificationListener)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result MuteSignal(handle_t hSignalHandle)
    {
        return ERR_NOERROR;
    }
    
    virtual fep::Result UnmuteSignal(handle_t hSignalHandle)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetModule(fep::IModule * pModule)
    {
        return ERR_NOERROR;
    }
    virtual size_t GetMaxTransmitSize()
    {
        return 0;
    }

    virtual const cOptionsFactory* GetSignalOptionsFactory()
    {
        return NULL;
    }
};


#endif // _FEP_TEST_MOCK_TX_ADAPTER_H_INC_