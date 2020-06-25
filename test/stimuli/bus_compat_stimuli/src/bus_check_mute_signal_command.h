/**
*
* Bus Compat Stimuli: Bus Check for Mute Signal Command
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

#ifndef _BUS_COMPAT_STIMULI_BUS_CHECK_MUTE_SIGNAL_COMMAND_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_BUS_CHECK_MUTE_SIGNAL_COMMAND_H_INCLUDED_

#include "stdafx.h"
#include "bus_check_base.h"
#include "a_util/concurrency.h"


class cBusCheckMuteSignalCommand : public cBusCheckBase
{
public:
    cBusCheckMuteSignalCommand(std::string strSigName, fep::tSignalDirection eDirection, bool bMuteStatus);
    ~cBusCheckMuteSignalCommand();

protected: // implements fep::cCommandListener
    fep::Result Update(fep::IMuteSignalCommand const * poCommand);
    fep::Result Update(fep::IResultCodeNotification const * poCommand);

private:
    fep::Result DoSend();
    fep::Result DoReceive();

private:
    bool m_bMuteStatus;
    std::string m_strSignalName;
    fep::tSignalDirection m_eDirection;
    bool m_bMuteStatusReceived;
    std::string m_strSignalNameReceived;
    fep::tSignalDirection m_eDirectionReceived;
    a_util::concurrency::semaphore m_semUpdate;
};

#endif // _BUS_COMPAT_STIMULI_BUS_CHECK_MUTE_SIGNAL_COMMAND_H_INCLUDED_