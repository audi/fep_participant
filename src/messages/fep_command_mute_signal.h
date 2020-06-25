/**
* Declaration of the Class cMuteSignalCommand.
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

#ifndef __FEP_COMMAND_MUTE_H
#define __FEP_COMMAND_MUTE_H

#include <cstdint>
#include <string>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "messages/fep_command_mute_signal_intf.h"
#include "messages/fep_message.h"
#include "transmission_adapter/fep_signal_direction.h"

namespace fep
{
    /// This class represents a Get Signal Information Command.
    class FEP_PARTICIPANT_EXPORT cMuteSignalCommand : public cMessage, public IMuteSignalCommand
    {
    public:
        /**
        * The method \c GetSignalName returns the name of the affected signal
        * @retval name of the affected signal
        */
        virtual const char* GetSignalName() const;

        /**
        * The method \c GetMutingStatus returns the
        * new muting status of the affected signal.
        *
        * @retval true -> mute signal
        * @retval false -> unmute signal
        */
        virtual bool GetMutingStatus() const;

        /**
         * The method \c GetSignalDirection returns the direction of
         * the signal to be muted (currently only output signals are supported)
         * @retval SD_Output output signal
         * @retval SD_Input input signal
        */
        virtual tSignalDirection GetSignalDirection() const;

        /**
        * The method \c GetCommandCookie returns the cookie associated with this cookie
        * @retval The cookie
        */
        virtual int64_t GetCommandCookie() const;

        /**
        * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
        */
        cMuteSignalCommand(
            const char * strSignalName, const fep::tSignalDirection eSignalDirection, bool bMuteSignal,
            char const * strSender, char const * strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
        * CTOR
        * @param [in] strCommand  A string representation of this command
        */
        cMuteSignalCommand(char const * strCommand);

        /**
        * @brief Copy Constructor
        * @param oOther Other instance to copy from.
        */
        cMuteSignalCommand(const cMuteSignalCommand& oOther);

        /**
        * @brief operator = / Assignment Operator
        * @param oOther Other instance to copy from.
        * @retval *this
        */
        cMuteSignalCommand& operator= (const cMuteSignalCommand& oOther);

        /// Default DTOR
        virtual ~cMuteSignalCommand()
        {
        };

    public: // implements IMessage
        virtual uint8_t GetMajorVersion() const;
        virtual uint8_t GetMinorVersion() const;
        virtual char const * GetReceiver() const;
        virtual char const * GetSender() const;
        virtual timestamp_t GetTimeStamp() const;
        virtual timestamp_t GetSimulationTime() const;
        virtual char const * ToString() const;

    private:
        /// creates a JSON string representation and stores it to class member m_strRepresentation
        fep::Result CreateStringRepresentation();
        /// parses a received JSON string representation
        fep::Result ParseFromStringRepresentation(const char * strRepr);

    private:
        /// JSON string representation of this command
        std::string m_strRepresentation;
        /// Name of the affected Signal
        std::string m_strAffectedSignal;
        /// New mute status of the affected signal
        bool m_bMuteStatus;
        /// command cookie
        int64_t m_nCookie;
        /// signal direction
        fep::tSignalDirection m_eSignalDirection;
    };
}; // namespace fep
#endif /* __FEP_COMMAND_MUTE_H */
