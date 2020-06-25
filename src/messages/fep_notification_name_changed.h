/**
 * Declaration of the Class cNameChangedNotification.
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

#if !defined(FEP_NOTIFICATION_NAME_CHANGED_INCLUDED)
#define FEP_NOTIFICATION_NAME_CHANGED_INCLUDED

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_message.h"
#include "messages/fep_notification_name_changed_intf.h"

namespace fep
{
    /**
     * Base implementation of INameChangedNotification
     */
    class FEP_PARTICIPANT_EXPORT cNameChangedNotification : public cMessage, public INameChangedNotification
    {
        /// d-ptr
        FEP_UTILS_D(cNameChangedNotification);

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * 
         * @param [in] strOldElementName  The old Name of the sender.
         */
        cNameChangedNotification(const char* strOldElementName,
            const char* strSender, const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         * 
         * @param [in] strNameChangedNotification  A JSON string representation of the notification.
         */
        cNameChangedNotification(char const * strNameChangedNotification);

        /**
         * @brief cNameChangedNotification Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cNameChangedNotification(const cNameChangedNotification& oOther);

        /**
         * @brief operator = cNameChangedNotification Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cNameChangedNotification &operator= (const cNameChangedNotification& oOther);

        /**
        * DTOR
        */
        virtual ~cNameChangedNotification();

    public: // implements INameChangedNotification
        virtual const char* GetOldParticipantName() const;

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
} // namespace fep
#endif // !defined(FEP_NOTIFICATION_NAME_CHANGED_INCLUDED)
