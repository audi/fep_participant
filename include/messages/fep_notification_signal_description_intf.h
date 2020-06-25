/**
 * Declaration of the Class ISignalDescriptionNotification.
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

#ifndef __FEP_NOTIFICATION_SIGNAL_DESCRIPTION_INTF_H
#define __FEP_NOTIFICATION_SIGNAL_DESCRIPTION_INTF_H

#include "messages/fep_notification_intf.h"

namespace fep
{
    /// Interface of a signal description notification
    class FEP_PARTICIPANT_EXPORT ISignalDescriptionNotification : public INotification
    {
    public:
        /// DTOR
        virtual ~ISignalDescriptionNotification() = default;

        /**
         * The method \c GetSignalDescription returns the requested signal description.
         * 
         * @returns The minimal signal description.
         */
        virtual const char * GetSignalDescription() const = 0;

    }; /* class FEP_PARTICIPANT_EXPORT ISignalDescriptionNotification */
} /* namespace fep */

#endif /* __FEP_NOTIFICATION_SIGNAL_DESCRIPTION_INTF_H */
