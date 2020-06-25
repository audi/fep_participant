/**
 * Declaration of the Class cLogNotification.
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

#if !defined(EA_6B6B410A_D191_4e28_942D_3F6B69742D4E__INCLUDED_)
#define EA_6B6B410A_D191_4e28_942D_3F6B69742D4E__INCLUDED_

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "incident_handler/fep_severity_level.h"
#include "messages/fep_message.h"
#include "messages/fep_notification_incident_intf.h"

namespace fep
{
    /**
     * Base implementation of IIncidentNotification
     */
    class FEP_PARTICIPANT_EXPORT cIncidentNotification : public cMessage, public IIncidentNotification
    {
        /// D-Pointer
        FEP_UTILS_D(cIncidentNotification);

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         *
         * @param [in] strLogMessage  The log message.
         * @param [in] severity  The level of the log message.
         */
        cIncidentNotification(char const * strLogMessage, tSeverityLevel severity,
            const char* strSender,
            const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         *
         * @param [in] strLogMessage  The log message.
         * @param [in] severity  The level of the log message.
         * @param [in] nIncidentCode A specific incident code to accompany the log message.
         * 
         */
        cIncidentNotification(int16_t nIncidentCode, char const * strLogMessage,
            tSeverityLevel severity,
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         * 
         * @param [in] strLogNotification  A JSON string representation of the notification.
         */
        cIncidentNotification(char const * strLogNotification);

        /**
         * @brief cLogNotifications Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cIncidentNotification(const cIncidentNotification& oOther);

        /**
         * @brief operator = cLogNotification Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cIncidentNotification &operator= (const cIncidentNotification& oOther);

        /// Default virtual destructor
        virtual ~cIncidentNotification();

    public: // implements IIncidentNotification
        virtual tSeverityLevel GetSeverity() const;
        virtual char const * GetDescription() const;
        virtual int16_t GetIncidentCode() const;

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

#endif // !defined(EA_6B6B410A_D191_4e28_942D_3F6B69742D4E__INCLUDED_)
