/**
 * Declaration of the Class cGetSignalInfoCommand.
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

#ifndef __FEP_COMMAND_GET_SIGNAL_INFO_H
#define __FEP_COMMAND_GET_SIGNAL_INFO_H

#include <cstdint>
#include <string>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "messages/fep_message.h"
#include "messages/fep_command_get_signal_info_intf.h"

namespace fep
{
    /// This class represents a Get Signal Information Command.
    class FEP_PARTICIPANT_EXPORT cGetSignalInfoCommand : public cMessage, public IGetSignalInfoCommand
    {
    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         */
        cGetSignalInfoCommand(
            char const * strSender, char const * strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         * @param [in] strCommand  A string representation of this command
         */
        cGetSignalInfoCommand(char const * strCommand);

        /**
         * @brief Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cGetSignalInfoCommand(const cGetSignalInfoCommand& oOther);

        /**
         * @brief operator = / Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cGetSignalInfoCommand& operator= (const cGetSignalInfoCommand& oOther);

        /// Default DTOR
        virtual ~cGetSignalInfoCommand();

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

    private:
        /// JSON string representation of this command
        std::string m_strRepresentation;
    };
}; // namespace fep
#endif /* __FEP_COMMAND_GET_SIGNAL_INFO_H */
