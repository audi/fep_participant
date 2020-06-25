/**

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
 */
#ifndef __LEGACY_TIMING_TIMING_CONFIG_H__
#define __LEGACY_TIMING_TIMING_CONFIG_H__

#include <string>
#include "fep_participant_export.h"
#include "fep_result_decl.h"

namespace fep
{
namespace timing
{
    struct TimingConfiguration;

        /**
         * The timing configuration class 
         * The timing clients are reading the configuration from files. 
         * The timing configuration class provides support for this task. 
         */
    class FEP_PARTICIPANT_EXPORT TimingConfig
    {
        public:
            /**
            * Read configuration from file
            *
            * @param [in] filename name possibly including path to the configuration file
            * @param [out] timing_configuration timing configuration output
            * @returns  Standard result code.
            * @retval ERR_NOERROR Everything went fine
            * @retval ERR_UNEXPECTED Something unexpected happend. Details inside the result.
            */
            static fep::Result readTimingConfigFromFile(const std::string& filename, TimingConfiguration& timing_configuration);
            
            /**
            * Read configuration from string
            *
            * @param [in] xml_string string with configuration in xml format
            * @param [out] timing_configuration timing configuration output
            * @returns  Standard result code.
            * @retval ERR_NOERROR Everything went fine
            * @retval ERR_UNEXPECTED Something unexpected happend. Details inside the result.
            */
            static fep::Result readTimingConfigFromString(const std::string& xml_string, TimingConfiguration& timing_configuration);

            /**
            * Write configuration from file
            *
            * @param [out] filename name possibly including path to the configuration file
            * @param [in] timing_configuration timing configuration input
            * @returns Standard result code.
            * @retval ERR_NOERROR Everything went fine
            * @retval ERR_UNEXPECTED Something unexpected happend. Details inside the result.
            */
            static fep::Result writeTimingConfigToFile(const std::string& filename, const TimingConfiguration& timing_configuration);

            /**
            * Write configuration from string
            *
            * @param [out] xml_string string with configuration in xml format
            * @param [in] timing_configuration timing configuration input
            * @returns  Standard result code.
            * @retval ERR_NOERROR Everything went fine
            * @retval ERR_UNEXPECTED Something unexpected happend. Details inside the result.
            */
            static fep::Result writeTimingConfigToString(std::string& xml_string, const TimingConfiguration& timing_configuration);
    };
}
}



#endif // __LEGACY_TIMING_TIMING_CONFIG_H__
