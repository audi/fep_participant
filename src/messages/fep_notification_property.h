/**
 * Declaration of the Class cPropertyNotification.
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

#if !defined(EA_5F88623E_5175_4801_95AA_C973A4229073__INCLUDED_)
#define EA_5F88623E_5175_4801_95AA_C973A4229073__INCLUDED_

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_message.h"
#include "messages/fep_notification_property_intf.h"

namespace fep
{
    class IProperty;
    class cProperty;

    /**
     * Implementation of cPropertyNotification
     */
    class FEP_PARTICIPANT_EXPORT cPropertyNotification : public cMessage, public IPropertyNotification
    {
        /// d-ptr
        FEP_UTILS_D(cPropertyNotification);

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * 
         * @param [in] strPropPath  The property path.
         * @param [in] poProperty  The property that should be sent with this notification.
         */
        cPropertyNotification(char const * strPropPath, const fep::cProperty * poProperty,
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);
        
        /**
         * CTOR
         * 
         * @param [in] strPropertyNotification  A JSON string representation of the notification.
         */
        cPropertyNotification(char const * strPropertyNotification);

        /**
         * @brief cPropertyNotification Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cPropertyNotification(const cPropertyNotification& oOther);

        /**
         * @brief operator = cPropertyNotification Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cPropertyNotification &operator= (const cPropertyNotification& oOther);

        /**
        * DTOR
        */
        virtual ~cPropertyNotification();

    public: // implements IPropertyNotification
        virtual const char * GetPropertyPath() const;
        virtual IProperty * TakeProperty();
        virtual const IProperty * GetProperty() const;

    public: // implements IMessage
        virtual uint8_t GetMajorVersion() const;
        virtual uint8_t GetMinorVersion() const;
        virtual char const * GetReceiver() const;
        virtual char const * GetSender() const;
        virtual timestamp_t GetTimeStamp() const;
        virtual timestamp_t GetSimulationTime() const;
        virtual char const * ToString() const;

    private:
        /// creates the json string representation of the notification
        fep::Result CreateStringRepresentation(const fep::cProperty * poProperty);
        /// parses a received string representation
        fep::Result ParseFromStringRepresentation(const char * strRepr);
    };
}; // namespace fep

#endif // !defined(EA_5F88623E_5175_4801_95AA_C973A4229073__INCLUDED_)
