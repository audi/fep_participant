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

#ifndef _BUS_COMPAT_STIMULI_BUS_CHECK_RESULT_CODE_NOTIFICATION_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_BUS_CHECK_RESULT_CODE_NOTIFICATION_H_INCLUDED_

#include "stdafx.h"
#include "bus_check_base.h"


class cBusCheckResultCodeNotification : public cBusCheckBase
{
public:
    cBusCheckResultCodeNotification(const fep::Result nResultCode, const int64_t nCommandCookie);
    ~cBusCheckResultCodeNotification();

protected:  // implements fep::cNotificationListener
    fep::Result Update(fep::IResultCodeNotification const * poNotification); 

private:
    fep::Result DoSend();
    
private:
    fep::IResultCodeNotification* m_pNotification;
    fep::Result m_nResultCode;
    int64_t m_nCommandCookie;
};

#endif // _BUS_COMPAT_STIMULI_BUS_CHECK_RESULT_CODE_NOTIFICATION_H_INCLUDED_
