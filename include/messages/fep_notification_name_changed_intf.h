/**
 * Declaration of the Class INameChangedNotification.
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

#include "fep_notification_intf.h"

#if !defined(FEP_NOTIFICATION_NAME_CHANGED_INTF_INCLUDED)
#define FEP_NOTIFICATION_NAME_CHANGED_INTF_INCLUDED

namespace fep
{
    /**
    * Base interface of notifications in FEP
    */
    class FEP_PARTICIPANT_EXPORT INameChangedNotification : public INotification
    {

    public:
        /**
         * DTOR
         */
        virtual ~INameChangedNotification() = default;

        /**
         * The method \c GetOldParticipantName returns the old name of the sender.
         * 
         * @returns  The old participant name
         */
        virtual const char* GetOldParticipantName() const = 0;

    };
} // namespace fep
#endif // !defined(FEP_NOTIFICATION_NAME_CHANGED_INTF_INCLUDED)
