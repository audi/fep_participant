/**
 * Declaration of the Class cSignalInfoNotification.
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
#ifndef __FEP_NOTIFICATION_SIGNAL_INFO_H
#define __FEP_NOTIFICATION_SIGNAL_INFO_H

#include <cstdint>
#include <memory>
#include <string>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "messages/fep_message.h"
#include "messages/fep_notification_signal_info_intf.h"

namespace fep
{
    class IStringList;
    class cStringList;

    /// Implementation of a signal info notification
    class FEP_PARTICIPANT_EXPORT cSignalInfoNotification : public cMessage, public ISignalInfoNotification
    {
    public:
        /// CTOR (by lists of signal names)
        cSignalInfoNotification(
            fep::IStringList const * poRxSignals, fep::IStringList const * poTxSignals,
            char const * strSender,
            char const * strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime);
        /// CTOR (by JSON representation)
        cSignalInfoNotification(char const * strPropertyNotification);
        /// Copy-CTOR
        cSignalInfoNotification(const cSignalInfoNotification& oOther);
        /// operator = / cSignalInfoNotification assignment operator
        cSignalInfoNotification &operator=(const cSignalInfoNotification &oOther);
        /// DTOR
        virtual ~cSignalInfoNotification();

    public: /* implements ISignalInfoNotification */
        virtual fep::Result TakeSignalLists(
            fep::IStringList *& poRxSignals, fep::IStringList *& poTxSignals);

    private:
        /// creates the JSON string representation of the notification
        fep::Result CreateStringRepresentation(
            fep::IStringList const * poRxSignals, fep::IStringList const * poTxSignals);
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
        /// pointer to the list holding all RX signal names (even index) and types (odd index)
        std::unique_ptr<fep::cStringList> m_poRxList;
        /// pointer to the list holding all TX signal names (even index) and types (odd index)
        std::unique_ptr<fep::cStringList> m_poTxList;
    }; /* class FEP_PARTICIPANT_EXPORT cSignalInfoNotification */
} /* namespace fep */

#endif /* __FEP_NOTIFICATION_SIGNAL_INFO_H */
