/**
 * Declaration of the Class cMappingConfigurationCommand.
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

#ifndef __FEP_COMMAND_MAPPING_CONFIGURATION_H
#define __FEP_COMMAND_MAPPING_CONFIGURATION_H

#include <cstdint>
#include <string>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "messages/fep_command_mapping_configuration_intf.h"
#include "messages/fep_message.h"

namespace fep
{
    /**
     * This class represents a Mapping Configuration Command.
     */
    class FEP_PARTICIPANT_EXPORT cMappingConfigurationCommand :
        public cMessage, public IMappingConfigurationCommand
    {
        /// command type
        IMappingConfigurationCommand::tCommandType m_Type;
        /// Mapping string
        std::string m_strConfiguration;
        /// Mapping flags
        uint32_t m_ui32Flags;
        /// command cookie
        int64_t m_nCookie;
        /// string representation of the message
        std::string m_strRepresentation;

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * \note This constructor sets the command type to CT_REGISTER_MAPPING
         *
         * @param [in] strConfiguration  The mapping configuration
         * @param [in] ui32Flags  The mapping flags
         */
        cMappingConfigurationCommand(const char* strConfiguration, uint32_t ui32Flags,
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * \note This constructor sets the command type to CT_CLEAR_MAPPING
         */
        cMappingConfigurationCommand(const char* strSender,
            const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         * 
         * @param [in] strRepr  A string representation of the command
         */
        cMappingConfigurationCommand(char const * strRepr);

        /**
         * @brief Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cMappingConfigurationCommand(const cMappingConfigurationCommand& oOther);

        /**
         * @brief operator = Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cMappingConfigurationCommand& operator= (const cMappingConfigurationCommand& oOther);

    public: // implements IMappingConfigurationCommand
        virtual int64_t GetCommandCookie() const;
        virtual tCommandType GetCommandType() const;
        virtual const char* GetConfigurationString() const;
        virtual uint32_t GetMappingFlags() const;

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
#endif // __FEP_COMMAND_MAPPING_CONFIGURATION_H
