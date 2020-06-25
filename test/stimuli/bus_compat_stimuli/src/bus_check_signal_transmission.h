/**
 *
 * Bus Compat Stimuli: Bus Check for Custom Command
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

#ifndef _BUS_COMPAT_STIMULI_BUS_CHECK_SIGNAL_TRANSMISSION_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_BUS_CHECK_SIGNAL_TRANSMISSION_H_INCLUDED_

#include "stdafx.h"
#include "bus_check_base.h"


class cBusCheckSignalTransmission : public cBusCheckBase, public fep::IUserDataListener
{
public:
    cBusCheckSignalTransmission();
    ~cBusCheckSignalTransmission();

protected: // implements fep::IUserDataListener
    fep::Result Update(const fep::IUserDataSample* poSample);

    fep::Result Update(fep::IGetPropertyCommand const * poCommand);
    fep::Result Update(fep::IPropertyNotification const * pPropertyNotification);

private:
    fep::Result DoSend();
    fep::Result DoReceive();
   
private:
    uint64_t m_nMyUuid;
    static const uint32_t s_nUserSamples= 10;
    fep::IUserDataSample* m_poUserSamples[s_nUserSamples];
    uint32_t m_nReceivedCount;
};

#endif // _BUS_COMPAT_STIMULI_BUS_CHECK_SIGNAL_TRANSMISSION_H_INCLUDED_
