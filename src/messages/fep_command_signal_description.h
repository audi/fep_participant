/**
 * Declaration of the Class cSignalDescriptionCommand.
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

#ifndef __FEP_COMMAND_SIGNAL_DESCRIPTION_H
#define __FEP_COMMAND_SIGNAL_DESCRIPTION_H

#include <cstdint>
#include <string>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "messages/fep_command_signal_description_intf.h"
#include "messages/fep_message.h"

namespace fep
{
    /**
     * This class represents a Signal Description Command.
     */
    class FEP_PARTICIPANT_EXPORT cSignalDescriptionCommand : public cMessage, public ISignalDescriptionCommand
    {
        /// command type
        ISignalDescriptionCommand::tCommandType m_Type;
        /// description string
        std::string m_strDescription;
        /// description flags
        uint32_t m_ui32Flags;
        /// command cookie
        int64_t m_nCookie;
        /// string representation of the message
        std::string m_strRepresentation;

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * \note This constructor sets the command type to CT_REGISTER_DESCRIPTION
         *
         * @param [in] strDescription  The signal description
         * @param [in] ui32Flags  The signal description flags
         */
        cSignalDescriptionCommand(const char* strDescription, uint32_t ui32Flags,
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * \note This constructor sets the command type to CT_CLEAR_DESCRIPTIONS
         */
        cSignalDescriptionCommand(const char* strSender,
            const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         * 
         * @param [in] strRepr  A string representation of the command
         */
        cSignalDescriptionCommand(char const * strRepr);

        /**
         * @brief Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cSignalDescriptionCommand(const cSignalDescriptionCommand& oOther);

        /**
         * @brief operator = Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cSignalDescriptionCommand& operator= (const cSignalDescriptionCommand& oOther);

    public: // implements ISignalDescriptionCommand
        virtual int64_t GetCommandCookie() const;
        virtual tCommandType GetCommandType() const;
        virtual const char* GetDescriptionString() const;
        virtual uint32_t GetDescriptionFlags() const;

    public: // implements IMessage
        virtual uint8_t GetMajorVersion() const;
        virtual uint8_t GetMinorVersion() const;
        virtual char const * GetReceiver() const;
        virtual char const * GetSender() const;
        virtual timestamp_t GetTimeStamp() const;
        virtual timestamp_t GetSimulationTime() const;
        virtual char const * ToString() const;

    private:
        /// @copydoc cMessage::CreateStringRepresentation
        fep::Result CreateStringRepresentation();

        /// parses a received JSON string representation
        fep::Result ParseFromStringRepresentation(const char * strRepr);
    };
}; // namespace fep
#endif // __FEP_COMMAND_SIGNAL_DESCRIPTION_H
