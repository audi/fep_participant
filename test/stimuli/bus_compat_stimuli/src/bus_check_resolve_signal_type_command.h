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

#ifndef _BUS_COMPAT_STIMULI_BUS_CHECK_RESOLVE_SIGNAL_TYPE_COMMAND_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_BUS_CHECK_RESOLVE_SIGNAL_TYPE_COMMAND_H_INCLUDED_

#include "stdafx.h"
#include "bus_check_base.h"


class cBusCheckResolveSignalTypeCommand : public cBusCheckBase
{
public:
    cBusCheckResolveSignalTypeCommand(const char* strSignalType);
    ~cBusCheckResolveSignalTypeCommand();

protected: // implements fep::cCommandListener
    fep::Result Update(fep::IResolveSignalTypeCommand const * poCommand);

protected:  // implements fep::cNotificationListener
    fep::Result Update(fep::ISignalDescriptionNotification const * pSignalDescriptionNotification); 

private:
    fep::Result DoSend();
    
private:
    fep::IResolveSignalTypeCommand* m_pCommand;
    std::string m_strSignalType;

    bool m_bResolveSignalTypeCommand;
    bool m_bSignalDescriptionNotification;
};

#endif // _BUS_COMPAT_STIMULI_BUS_CHECK_RESOLVE_SIGNAL_TYPE_COMMAND_H_INCLUDED_
