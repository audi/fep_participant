/**
 * Declaration of the Class cUnregPropListenerAckNotification.
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

#if !defined(EA_A820132D_3345_41E3_82A0_4BD1099C9D08__INCLUDED_)
#define EA_A820132D_3345_41E3_82A0_4BD1099C9D08__INCLUDED_

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_message.h"
#include "messages/fep_notification_unreg_prop_listener_ack_intf.h"

namespace fep
{
    /**
     * Implementation of cUnregPropListenerAckNotification
     */
    class FEP_PARTICIPANT_EXPORT cUnregPropListenerAckNotification : public cMessage, public IUnregPropListenerAckNotification
    {
        /// d-ptr
        FEP_UTILS_D(cUnregPropListenerAckNotification);

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * 
         * @param [in] strPropPath  The property path.
         */
        cUnregPropListenerAckNotification(char const * strPropPath,
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);
        
        /**
         * CTOR
         * 
         * @param [in] strNotification  A JSON string representation of the notification.
         */
        cUnregPropListenerAckNotification(char const * strNotification);

        /**
         * @brief cUnregPropListenerAckNotification Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cUnregPropListenerAckNotification(const cUnregPropListenerAckNotification& oOther);

        /**
         * @brief operator = cUnregPropListenerAckNotification Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cUnregPropListenerAckNotification &operator= (const cUnregPropListenerAckNotification& oOther);

        /**
        * DTOR
        */
        virtual ~cUnregPropListenerAckNotification();

    public: // implements IUnregPropListenerAckNotification
        virtual const char * GetPropertyPath() const;

    public: // implements IMessage
        virtual uint8_t GetMajorVersion() const;
        virtual uint8_t GetMinorVersion() const;
        virtual char const * GetReceiver() const;
        virtual char const * GetSender() const;
        virtual timestamp_t GetTimeStamp() const;
        virtual timestamp_t GetSimulationTime() const;
        virtual char const * ToString() const;

    private:
        /// creates and stores the json string representation of the notification
        fep::Result CreateStringRepresentation();
    };
}; // namespace fep

#endif // !defined(EA_A820132D_3345_41E3_82A0_4BD1099C9D08__INCLUDED_)
