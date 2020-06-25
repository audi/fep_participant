/**
 * Declaration of the Class IPropertyNotification.
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

#if !defined(EA_6E8A4CFF_C2A9_4C22_B19C_903BE5D40595__INCLUDED_)
#define EA_6E8A4CFF_C2A9_4C22_B19C_903BE5D40595__INCLUDED_

#include "fep_notification_intf.h"

namespace fep
{
    class IProperty;

    /**
     * Interface of a property notification in FEP
     */
    class FEP_PARTICIPANT_EXPORT IPropertyNotification : public INotification
    {

    public:
        /**
         * DTOR
         */
        virtual ~IPropertyNotification() = default;

        /**
         * The method \c GetPropertyPath returns the path of the property.
         * 
         * @returns The path.
         */
        virtual const char * GetPropertyPath() const = 0;

        /**
         * The method \c TakeProperty returns the property contained in the
         * notification. Ownership is transfered to the caller and no copy
         * will remain after this call!
         * 
         * @returns The property.
         */
        virtual IProperty * TakeProperty() = 0;

        /**
         * The method \c GetProperty returns the property contained in the
         * notification. Ownership remains with the notification and is not transfered!
         * 
         * @returns The property as a const pointer.
         */
        virtual const IProperty * GetProperty() const = 0;
    };
} // namespace fep
#endif // !defined(EA_6E8A4CFF_C2A9_4C22_B19C_903BE5D40595__INCLUDED_)
