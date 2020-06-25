/**
 * Declaration of the Class IMappingConfigurationCommand.
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

#ifndef __FEP_COMMAND_MAPPING_CONFIGURATION_INTERFACE_H
#define __FEP_COMMAND_MAPPING_CONFIGURATION_INTERFACE_H

#include "messages/fep_command_intf.h"

namespace fep
{
    /**
     * This is the interface for a mapping configuration command.
     */
    class FEP_PARTICIPANT_EXPORT IMappingConfigurationCommand : public ICommand
    {
    public:
        /// enum that describes the type of the command
        typedef enum
        {
            CT_REGISTER_MAPPING,
            CT_CLEAR_MAPPING
        } tCommandType;

    public:
        /**
         * DTOR
         */
        virtual ~IMappingConfigurationCommand() = default;

        /**
         * The method \c GetCommandCookie returns the cookie associated with this cookie
         * @retval The cookie
         */
        virtual int64_t GetCommandCookie() const = 0;

        /**
         * The method \c GetCommandType returns the type of the command.
         * @retval The command type
         */
        virtual tCommandType GetCommandType() const = 0;

        /**
         * The method \c GetConfigurationString returns the configuration string or file path.
         * 
         * \note Returns NULL if command type is not CT_REGISTER_MAPPING.
         * @retval Returns the contained config string or NULL
         */
        virtual const char* GetConfigurationString() const = 0;

        /**
         * The method \c GetMappingFlags returns the mapping flags.
         *
         * \note Returns 0 if command type is not CT_REGISTER_MAPPING.
         * \note See \ref fep::ISignalMapping::tMappingFlags
         * @retval Returns the contained mapping flags
         */
        virtual uint32_t GetMappingFlags() const = 0;
    };
} // namespace fep
#endif // __FEP_COMMAND_MAPPING_CONFIGURATION_INTERFACE_H
