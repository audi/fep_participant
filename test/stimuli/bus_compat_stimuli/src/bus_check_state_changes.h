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

#ifndef _BUS_COMPAT_STIMULI_BUS_CHECK_STATE_CHANGES_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_BUS_CHECK_STATE_CHANGES_H_INCLUDED_

#include "stdafx.h"
#include "bus_check_base.h"


class cBusCheckStateChanges : public cBusCheckBase
{
public:
    cBusCheckStateChanges(const fep::tControlEvent eControlEvent, const fep::tState eIntermediateState, const fep::tState eExpectedState, timestamp_t tmWaitTime);

protected:  // implements fep::cNotificationListener
    fep::Result Update(fep::IStateNotification const * poNotification); 
    fep::Result Update(fep::IControlCommand const * poNotification); 
    fep::Result Update(fep::IGetPropertyCommand const * poCommand);

private:
    fep::Result DoSend();
    fep::Result DoReceive();
    
private:
    fep::tControlEvent m_eControlEvent;
    fep::tState m_eIntermediateState;
    fep::tState m_eExpectedState;
    timestamp_t m_tmWaitTime;
    bool m_bIntermediateStateReached;
};

#endif // _BUS_COMPAT_STIMULI_BUS_CHECK_STATE_CHANGES_H_INCLUDED_
