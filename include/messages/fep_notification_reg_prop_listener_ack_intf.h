/**
 * Declaration of the Class IRegPropListenerAckNotification.
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

#if !defined(EA_89821C72_E4CC_4FEF_9886_4302F8751430__INCLUDED_)
#define EA_89821C72_E4CC_4FEF_9886_4302F8751430__INCLUDED_

#include "messages/fep_notification_intf.h"

namespace fep
{
    /**
     * Interface of a register property listener acknowledge notification in FEP
     */
    class FEP_PARTICIPANT_EXPORT IRegPropListenerAckNotification : public INotification
    {

    public:
        /**
         * DTOR
         */
        virtual ~IRegPropListenerAckNotification() = default;

        /**
         * The method \c GetPropertyPath returns the path of the property.
         * 
         * @returns The path.
         */
        virtual char const * GetPropertyPath() const = 0;

        /**
        * The method \c TakeProperty() returns the property contained in this notfication.
        * Note that the property ownership is passed to the caller and no copy will remain!
        *
        * @returns The property.
        */
        virtual IProperty * TakeProperty() = 0;
    };
} // namespace fep
#endif // !defined(EA_89821C72_E4CC_4FEF_9886_4302F8751430__INCLUDED_)
