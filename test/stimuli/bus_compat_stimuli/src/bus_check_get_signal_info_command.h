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

#ifndef _BUS_COMPAT_STIMULI_BUS_CHECK_GET_SIGNAL_INFO_COMMAND_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_BUS_CHECK_GET_SIGNAL_INFO_COMMAND_H_INCLUDED_

#include "stdafx.h"
#include "bus_check_base.h"


class cBusCheckGetSignalInfoCommand : public cBusCheckBase
{
public:
    cBusCheckGetSignalInfoCommand();
    ~cBusCheckGetSignalInfoCommand();

protected: // implements fep::cCommandListener
    fep::Result Update(fep::IGetSignalInfoCommand const * poCommand);

protected:  // implements fep::cNotificationListener
    fep::Result Update(fep::ISignalInfoNotification const * pSignalInfoNotification); 

private:
    fep::Result DoSend();
    
private:
    fep::IGetSignalInfoCommand* m_pCommand;

    bool m_bGetSignalInfoCommand;
    bool m_bSignalInfoNotification;
};

#endif // _BUS_COMPAT_STIMULI_BUS_CHECK_GET_SIGNAL_INFO_COMMAND_H_INCLUDED_
