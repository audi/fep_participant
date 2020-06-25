/**
 * Declaration of the Class cNameChangeCommand.
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

#ifndef __FEP_COMMAND_NAME_CHANGE_H
#define __FEP_COMMAND_NAME_CHANGE_H

#include <cstdint>
#include <string>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "messages/fep_command_name_change_intf.h"
#include "messages/fep_message.h"

namespace fep
{
    /**
     * This class represents a Name Change Command.
     */
    class FEP_PARTICIPANT_EXPORT cNameChangeCommand :
        public cMessage, public INameChangeCommand
    {
        /// Mapping string
        std::string m_strName;
        /// string representation of the message
        std::string m_strRepresentation;

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * \note This constructor sets the command type to CT_REGISTER_MAPPING
         *
         * @param [in] strName  The mapping configuration
         */
        cNameChangeCommand(const char* strName,
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         * 
         * @param [in] strRepr  A string representation of the command
         */
        cNameChangeCommand(char const * strRepr);

        /**
         * @brief Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cNameChangeCommand(const cNameChangeCommand& oOther);

        /**
         * @brief operator = Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cNameChangeCommand& operator= (const cNameChangeCommand& oOther);

    public: // implements INameChangeCommand
        virtual const char* GetNewName() const;

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
#endif // __FEP_COMMAND_NAME_CHANGE_H
