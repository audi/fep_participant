/**
 * Declaration of the Class INotificationAccess.
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

#ifndef _FEP_NOTIFICATION_ACCESS_INTF_H_
#define _FEP_NOTIFICATION_ACCESS_INTF_H_

#include "fep_notification_intf.h"
#include "fep_notification_listener_intf.h"

namespace fep
{
    /**
     * This interface can be used to access 
     */
    class FEP_PARTICIPANT_EXPORT INotificationAccess
    {

    public:
        virtual ~INotificationAccess() = default;

        /**
         * The method \c TransmitNotification will send a state notification over the bus.
         * 
         * @param [in] pNotification  The notification.
         * @returns  Standard result code.
         * @retval ERR_POINTER  pNotification is NULL
         * @retval ERR_NOT_INITIALISED  Notification Access is not initialised
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result TransmitNotification(INotification const * pNotification) = 0;

        /**
         * The method \c RegisterNotificationListener registers a new status
         * listener.
         *
         * @param [in] poNotificationListener Pointer to the status listener
         *
         * @retval ERR_NOERROR Everything went fine.
         * @retval ERR_POINTER Null-pointer committed.
         * @retval ERR_UNEXPECTED Duplicated listener detected.
         *
         * @remarks The registration method automatically removes duplicated listeners.
         */
        virtual fep::Result RegisterNotificationListener(
            INotificationListener* poNotificationListener) =0;

        /**
         * The method \c UnregisterNotificationListener will unregister a previously registered
         * listener
         * 
         * @param [in] poNotificationListener  The listener
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        virtual fep::Result UnregisterNotificationListener(
            INotificationListener* poNotificationListener) =0;
    };
}
#endif //_FEP_NOTIFICATION_ACCESS_INTF_H_
