/**
 * Declaration of the Interface ISignalMapping.
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

#if !defined(FEP_SIGNAL_MAPPING_INTERFACE__INCLUDED_)
#define FEP_SIGNAL_MAPPING_INTERFACE__INCLUDED_

#include "fep_types.h"

namespace fep
{
    /**
     * @brief The ISignalMapping interface
     * Interface for the FEP Signal Mapping implementation. For details refer to
     * page @ref fep_signal_mapping.
     */
    class FEP_PARTICIPANT_EXPORT ISignalMapping
    {
    public:
        /// Virtual Destructor
        virtual ~ISignalMapping() {}

    public:
        /// Mapping flags (see \ref RegisterMappingConfiguration)
        enum tMappingFlags
        {
            /// Replace the entire known mapping information. Mutually exclusive with MF_MERGE.
            MF_REPLACE      = 0x1 << 0,

            /// Merge new mapping configuration elements from the configuration, return ERR_FAILED on conflict.
            /// Mutually exclusive with MF_REPLACE.
            MF_MERGE        = 0x1 << 1,

            /// Interpret the passed mapping as file path and load it from there
            MF_MAPPING_FILE = 0x1 << 2
        };

        /**
         * Registers mapping configuration in the system. This has no effect on currently registered
         * signals, only during signal registration is the mapping configuration considered. If the
         * new signal is matched to a target signal, the mapping is instantiated.
         *
         * @param [in] strConfig The configuration (or a file path, depending on the flags used)
         * @param [in] ui32MappingFlags The flags, as specified in \ref tMappingFlags, combined
         *                              using bitwise OR
         *
         * @retval ERR_NOT_SUPPORTED Invalid flag combination
         * @retval ERR_INVALID_ARG Config or file path is empty or NULL
         * @retval ERR_FAILED Configuration is invalid
         * @retval ERR_INVALID_FILE Could not read configuration file
         * @retval ERR_NOERROR Everything went as expected
         */
        virtual fep::Result RegisterMappingConfiguration(const char* strConfig,
            uint32_t ui32MappingFlags = MF_REPLACE) = 0;

        /**
         * Clears all mapping configuration in the system. This has no effect on currently registered
         * signals or instantiated mappings.
         *
         * @retval ERR_NOERROR Everything went as expected
         */
        virtual fep::Result ClearMappingConfiguration() = 0;
    };
}
#endif // !defined(FEP_SIGNAL_MAPPING_INTERFACE__INCLUDED_)
