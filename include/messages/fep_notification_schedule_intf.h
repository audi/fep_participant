/**
* Declaration of the Class IScheduleNotification.
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

#ifndef __FEP_NOTIFICATION_SCHEDULE_INTF_H
#define __FEP_NOTIFICATION_SCHEDULE_INTF_H

#include "messages/fep_notification_intf.h"
#include "_common/fep_schedule_list_intf.h"

namespace fep
{
    /// Interface of a schedule  notification
    class FEP_PARTICIPANT_EXPORT IScheduleNotification : public INotification
    {
    public:
        /// DTOR
        virtual ~IScheduleNotification() = default;
        /// Takes the schedule list out of the notification
        /// @param [out] poScheduleList       The list
        /// @retval Standard error
        virtual fep::Result TakeScheduleList(IScheduleList *& poScheduleList) = 0;
    }; /* class FEP_PARTICIPANT_EXPORT IScheduleNotification */
} /* namespace fep */
#endif /* __FEP_NOTIFICATION_SCHEDULE_INTF_H */
