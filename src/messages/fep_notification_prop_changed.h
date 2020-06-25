/**
 * Declaration of the Class cPropertyChangedNotification.
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

#if !defined(EA_4D121F73_AA4F_4E91_A316_5CC6EBEBCD41__INCLUDED_)
#define EA_4D121F73_AA4F_4E91_A316_5CC6EBEBCD41__INCLUDED_

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_message.h"
#include "messages/fep_notification_prop_changed_intf.h"

namespace fep
{
    /**
     * Implementation of cPropertyChangedNotification
     */
    class FEP_PARTICIPANT_EXPORT cPropertyChangedNotification : public cMessage, public IPropertyChangedNotification
    {
        /// d-ptr
        FEP_UTILS_D(cPropertyChangedNotification);

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * 
         * @param [in] poProperty   The property.
         * @param [in] ceEvent      The change event type.
         * @param [in] strPropPath  The property path.
         */
        cPropertyChangedNotification(const fep::IProperty * poProperty,
            IPropertyChangedNotification::tChangeEvent ceEvent, char const * strPropPath,
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         * 
         * @param [in] strNotification  A JSON string representation of the notification.
         */
        cPropertyChangedNotification(char const * strNotification);

        /**
         * @brief cPropertyChangedNotification Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cPropertyChangedNotification(const cPropertyChangedNotification& oOther);

        /**
         * @brief operator = cPropertyChangedNotification Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cPropertyChangedNotification &operator= (const cPropertyChangedNotification& oOther);

        /**
        * DTOR
        */
        virtual ~cPropertyChangedNotification();

    public: // implements IPropertyChangedNotification
        virtual const char * GetPropertyPath() const;
        virtual IPropertyChangedNotification::tChangeEvent GetEvent() const;
        virtual const fep::IProperty * GetProperty() const;

    public: // implements IMessage
        virtual uint8_t GetMajorVersion() const;
        virtual uint8_t GetMinorVersion() const;
        virtual char const * GetReceiver() const;
        virtual char const * GetSender() const;
        virtual timestamp_t GetTimeStamp() const;
        virtual timestamp_t GetSimulationTime() const;
        virtual char const * ToString() const;

    private:
        /// creates a string representation of the notification
        fep::Result CreateStringRepresentation();
        /// Parse a string representation
        fep::Result ParseFromStringRepresentation(const char * strRepr);
    };
}; // namespace fep

#endif // !defined(EA_4D121F73_AA4F_4E91_A316_5CC6EBEBCD41__INCLUDED_)
