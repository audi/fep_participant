/**
 * Declaration of the Class cSignalDescriptionNotification.
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
#ifndef __FEP_NOTIFICATION_SIGNAL_DESCRIPTION_H
#define __FEP_NOTIFICATION_SIGNAL_DESCRIPTION_H

#include <cstdint>
#include <string>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "messages/fep_message.h"
#include "messages/fep_notification_signal_description_intf.h"

namespace fep
{
    /// Implementation of a signal description notification
    class FEP_PARTICIPANT_EXPORT cSignalDescriptionNotification : 
        public cMessage, public ISignalDescriptionNotification
    {
    public:
        /// CTOR (by lists of signal names)
        cSignalDescriptionNotification(
            char const * strSignalDescription, char const * strSender,
            char const * strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime);
        /// CTOR (by JSON representation)
        cSignalDescriptionNotification(char const * strPropertyNotification);
        /// Copy-CTOR
        cSignalDescriptionNotification(const cSignalDescriptionNotification& oOther);
        /// operator = / cSignalInfoNotification assignment operator
        cSignalDescriptionNotification &operator=(const cSignalDescriptionNotification &oOther);
        /// DTOR
        virtual ~cSignalDescriptionNotification();

    public: /* implements ISignalDescriptionNotification */
        virtual const char * GetSignalDescription() const;

    private:
        /// creates the JSON string representation of the notification
        fep::Result CreateStringRepresentation();
        /// parses a received JSON string representation
        fep::Result ParseFromStringRepresentation(const char * strRepr);

    public: /* implements IMessage */
        virtual uint8_t GetMajorVersion() const;
        virtual uint8_t GetMinorVersion() const;
        virtual char const * GetReceiver() const;
        virtual char const * GetSender() const;
        virtual timestamp_t GetTimeStamp() const;
        virtual timestamp_t GetSimulationTime() const;
        virtual char const * ToString() const;

    private:
        /// JSON representation of this notification
        std::string m_strRepresentation;
        /// Stored signal description
        std::string m_strSignalDescription;

    }; /* class FEP_PARTICIPANT_EXPORT cSignalDescriptionNotification */
} /* namespace fep */

#endif /* __FEP_NOTIFICATION_SIGNAL_DESCRIPTION_H */
