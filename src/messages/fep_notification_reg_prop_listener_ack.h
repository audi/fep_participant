/**
 * Declaration of the Class cRegPropListenerAckNotification.
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

#if !defined(EA_0B467365_F89C_437A_8FCA_F12D37353DD3__INCLUDED_)
#define EA_0B467365_F89C_437A_8FCA_F12D37353DD3__INCLUDED_

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_message.h"
#include "messages/fep_notification_reg_prop_listener_ack_intf.h"

namespace fep
{
    class cProperty;
    class IProperty;

    /**
     * Implementation of cRegPropListenerAckNotification
     */
    class FEP_PARTICIPANT_EXPORT cRegPropListenerAckNotification : public cMessage, public IRegPropListenerAckNotification
    {
        /// d-ptr
        FEP_UTILS_D(cRegPropListenerAckNotification);

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * 
         * @param [in] strPropPath  The property path.
         * @param [in] poProperty The property contained in this notification.
         */
        cRegPropListenerAckNotification(char const * strPropPath, cProperty * poProperty,
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);
        
        /**
         * CTOR
         * 
         * @param [in] strNotification  A JSON string representation of the notification.
         */
        cRegPropListenerAckNotification(char const * strNotification);

        /**
         * @brief cRegPropListenerAckNotification Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cRegPropListenerAckNotification(const cRegPropListenerAckNotification& oOther);

        /**
         * @brief operator = cRegPropListenerAckNotification Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cRegPropListenerAckNotification &operator= (const cRegPropListenerAckNotification& oOther);

        /**
        * DTOR
        */
        virtual ~cRegPropListenerAckNotification();

    public: // implements IRegPropListenerAckNotification
        virtual const char * GetPropertyPath() const;
        virtual IProperty * TakeProperty();

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
        fep::Result CreateStringRepresentation(cProperty * poProperty);
        /// parses a string representation of this notification
        fep::Result ParseFromStringRepresentation(const char * strRepr);
    };
}; // namespace fep

#endif // !defined(EA_0B467365_F89C_437A_8FCA_F12D37353DD3__INCLUDED_)
