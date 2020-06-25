/**
 * Declaration of the Class IPropertyChangedNotification.
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

#if !defined(EA_39817F8C_778D_479B_8213_3D512065A05A__INCLUDED_)
#define EA_39817F8C_778D_479B_8213_3D512065A05A__INCLUDED_

#include "messages/fep_notification_intf.h"

namespace fep
{
    /**
     * Interface of a property changed notification in FEP
     */
    class FEP_PARTICIPANT_EXPORT IPropertyChangedNotification : public INotification
    {
    public:
        /// Property ChangeEvent Type
        enum tChangeEvent
        /* see "rules for changing an enumeration" (#27200) before doing any change! */
        {
            CE_New = 0,
            CE_Delete = 1,
            CE_Change = 2
        };

        /**
         * DTOR
         */
        virtual ~IPropertyChangedNotification() = default;

        /**
         * The method \c GetPropertyPath returns the path of the property.
         * 
         * @returns The path.
         */
        virtual const char * GetPropertyPath() const = 0;

        /**
         * The method \c GetEvent returns the event type of the notification.
         * 
         * @returns The change event type.
         */
        virtual tChangeEvent GetEvent() const = 0;

        /**
         * The method \c GetProperty returns a property instance which contains the
         * received value.
         * \note The received property is used to store the value, nothing else. It
         * is therefore a dummy instance with no meaningful path and name.
         * 
         * @returns  The property containing the received value.
         */
        virtual const fep::IProperty * GetProperty() const = 0;
    };
} // namespace fep
#endif // !defined(EA_39817F8C_778D_479B_8213_3D512065A05A__INCLUDED_)
