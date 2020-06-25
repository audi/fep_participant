/**
 * Declaration of the Class IUnregPropListenerAckNotification.
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

#if !defined(EA_65AB66D2_4402_4F56_B80D_7AAC8B1B7C12__INCLUDED_)
#define EA_65AB66D2_4402_4F56_B80D_7AAC8B1B7C12__INCLUDED_

#include "messages/fep_notification_intf.h"

namespace fep
{
    /**
     * Interface of a unregister property listener acknowledge notification in FEP
     */
    class FEP_PARTICIPANT_EXPORT IUnregPropListenerAckNotification : public INotification
    {

    public:
        /**
         * DTOR
         */
        virtual ~IUnregPropListenerAckNotification() = default;

        /**
         * The method \c GetPropertyPath returns the path of the property.
         * 
         * @returns The path.
         */
        virtual const char * GetPropertyPath() const = 0;
    };
} // namespace fep
#endif // !defined(EA_65AB66D2_4402_4F56_B80D_7AAC8B1B7C12__INCLUDED_)
