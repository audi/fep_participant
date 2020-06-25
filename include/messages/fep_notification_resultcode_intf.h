/**
 * Declaration of the Class IResultCodeNotification.
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

#ifndef __FEP_NOTIFICATION_RESULTCODE_INTF_H
#define __FEP_NOTIFICATION_RESULTCODE_INTF_H

#include "messages/fep_notification_intf.h"

namespace fep
{
    /// Interface of a result code notification
    class FEP_PARTICIPANT_EXPORT IResultCodeNotification : public INotification
    {
    public:
        /// DTOR
        virtual ~IResultCodeNotification() = default;

        /**
         * Returns the command cookie, tracing this notification to a command
         * @retval The cookie
         */
        virtual int64_t GetCommandCookie() const = 0;

        /**
         * Returns the encapsulated result code.
         * @retval The result code
         */
        virtual fep::Result GetResultCode() const = 0;
    }; /* class FEP_PARTICIPANT_EXPORT IResultCodeNotification */
} /* namespace fep */

#endif /* __FEP_NOTIFICATION_RESULTCODE_INTF_H */
