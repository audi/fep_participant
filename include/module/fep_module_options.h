/**
 * Declaration of the Class cModuleOptions.
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

#ifndef _FEP_MODULE_OPTIONS_H_
#define _FEP_MODULE_OPTIONS_H_

#include <cstdint>
#include <string>   //std::string

#include "fep_dptr.h"
#include "transmission_adapter/fep_transmission_type.h"

namespace fep
{
    class IStringList;

    /**
     * option to set default timing within the module/participant
     */
    enum FEP_PARTICIPANT_EXPORT eTimingSupportDefault
    {
        /// default timing will be set to @ref fep_timing_2 
        timing_FEP_20 = 0,
        ///default timing will be set to @ref fep_timing_3
        ///@remark if set to fep 3 the legacy layer is switched off completely
        timing_FEP_30 = 1
    };

    /**
     * FEP Module options class
     */ 
    class FEP_PARTICIPANT_EXPORT cModuleOptions
    {
        /// D-pointer to private implementation
        FEP_UTILS_D(cModuleOptions);

    public:
        /**
         * CTOR
         */
         cModuleOptions();

        /**
         * CTOR
         * 
         * @param [in] strElementName  The name of the element.
         */
         cModuleOptions(const char* strElementName);

         /**
          * CTOR
          *
          * @param [in] strElementName  The name of the element.
          * @param [in] eTimingSupportType  The default timing support.
          */
         cModuleOptions(const char* strElementName,
                        const eTimingSupportDefault eTimingSupportType);

        /**
         * CTOR
         * 
         * @param [in] eTransmissionType The FEP Element's transmission type.
         * @param [in] strElementName  The name of the element.
         */
         cModuleOptions(const eFEPTransmissionType eTransmissionType,
                        const char* strElementName);

         /**
          * CTOR
          *
          * @param [in] eTransmissionType The FEP Element's transmission type.
          * @param [in] strElementName  The name of the element.
          * @param [in] eTimingSupportType  The default timing support.
          */
         cModuleOptions(const eFEPTransmissionType eTransmissionType,
                        const char* strElementName,
                        const eTimingSupportDefault eTimingSupportType);

        /**
         * CTOR
         * 
         * @param [in]  rhs other instance to copy from
         */
        cModuleOptions(const cModuleOptions& rhs);

        /**
         * CTOR
         *
         * @param [in]  rhs other instance to move from
         */
        cModuleOptions(cModuleOptions&& rhs);

        /**
         * DTOR
         */
        ~cModuleOptions();

    public:
        /**
         * Copy operator
         * 
         * @param [in] rhs other instance to copy from
         * @return current instance
         */
        cModuleOptions& operator=(const cModuleOptions& rhs);

        /**
         * Move operator
         *
         * @param [in] rhs other instance to move from
         * @return current instance
         */
        cModuleOptions& operator=(cModuleOptions&& rhs);

    public:
        /**
         * Reset all values to the default values
         * 
         * This functions clears values and sets defaults
         * Part of this process is reading the environment variables
         * for doamin id ("FEP_MODULE_DOMAIN") 
         * as well as for the transmission driver ("FEP_TRANSMISSION_DRIVER")
         * 
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_INVALID_ARG Environment variable are out of range 
         */
        Result Reset();

    public:
        /**
        * Parse command line options
        *
        * @param [in,out] nArgc Program argument count
        * @param [in,out] pArgv Program arguments
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_INVALID_ARG Error in case of no option prefix / unknown argument / missing option value
        */
        Result ParseCommandLine(int& nArgc, const char** pArgv);

        /**
        * The method \c SetAdditionalOption allows to set additional options for the command line parser.
        * These options must be set before the arguments are parsed. After setting a valid option the
        * command line parser can parse these additional arguments and stores them in the first value.
        *
        * @param [in,out] strMonitoredValue variable to store value of evaluated argument
        * @param [in] strShortcutCall monitored call argument, short version. Care about existing arguments!
        * @param [in] strFullnameCall monitored call argument, long version. Care about existing arguments!
        * @param [in] strHelpText describing help text of these argument
        * @param [in] strHint (optional) a hint about the type that is needed (eg. file, int, string, ..)
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_INVALID_ARG Error in case of no option prefix / unknown argument / missing option value
        */
        a_util::result::Result SetAdditionalOption(std::string& strMonitoredValue,
            const std::string& strShortcutCall,
            const std::string& strFullnameCall,
            const std::string& strHelpText,
            const std::string& strHint = "");
        /**
        * The method \c SetAdditionalOption allows to set additional flag option for the command line parser.
        * These flags must be set before the arguments are parsed. After setting a valid flag the
        * command line parser can parse these additional flags and stores the result in the first value.
        *
        * @param [in,out] bMonitoredValue variable to store result if flag got called
        * @param [in] strShortcutCall monitored call argument, short version. Care about existing arguments!
        * @param [in] strFullnameCall monitored call argument, long version. Care about existing arguments!
        * @param [in] strHelpText describing help text of these argument
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_INVALID_ARG Error in case of no option prefix / unknown argument / missing option value
        */
        a_util::result::Result SetAdditionalOption(bool& bMonitoredValue,
            const std::string& strShortcutCall,
            const std::string& strFullnameCall,
            const std::string& strHelpText);

    public:
        /**
        * The method \c PrintHelp prints the current help with all available command line information
        * to the std::cout
        *
        */
        void PrintHelp() const;
        /**
         * The method \c GetParticipantName returns the participant name
         *
         * @return The FEP Participant's name.
         */
        const char* GetParticipantName() const;

        /**
         * The method \c SetParticipantName sets the participant name. A Valid Participant name
         * follows the following RegEx: "^([a-zA-Z0-9_\.\-]+)$"
         *
         * @param [in] strParticipantName The FEP Participant's name.
         */
        void SetParticipantName(const char* strParticipantName);

    public:
        /**
         * The method \c GetDomainId returns the domain id
         *
         * @return The FEP Element's domain id.
         */
        uint16_t GetDomainId() const;

        /**
         * The method \c SetDomainId sets the domain id
         *
         * @param [in] nDomainId The FEP Element's domain id.
         */
        void SetDomainId(const uint16_t nDomainId);

    public:
            /**
             * The method \c GetDefaultTimingSupport returns timing support
             *
             * @return The FEP module's default timing support.
             */
            eTimingSupportDefault GetDefaultTimingSupport() const;

            /**
             * The method \c SetDefaultTimingSupport sets default timing support type.
             *
             * @param [in] eTimingSupportType The FEP module's default timing support type.
             */
            void SetDefaultTimingSupport(const eTimingSupportDefault eTimingSupportType);

    public:
        /**
         * The method \c GetTransmissionType returns the transmission type
         *
         * @return The FEP module's transmission type.
         */
        eFEPTransmissionType GetTransmissionType() const;

        /**
         * The method \c SetTransmissionType sets the domain id
         *
         * @param [in] eTransmissionType The FEP module's transmission type.
         */
        void SetTransmissionType(const eFEPTransmissionType eTransmissionType);

    public:
        /**
         * The method \c GetNetworkInterfaceWhitelist returns a list
         * of all whitelisted network interfaces.
         *
         * @param [out] pStringList list of network names
         * @returns Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        Result GetNetworkInterfaceList(IStringList*& pStringList) const;

        /**
         * The method \c AppendToNetworkInterfaceWhitelist appends the
         * given interface to the whitelisted interfaces
         *
         * @param [in] strInterfaceName A interface definitions
         * @returns Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_INVALID_ARG The given interface name is invalid or not available
         * @retval ERR_POINTER The argument is a null pointer
         */
        Result AppendToNetworkInterfaceList(const char* strInterfaceName);
        
        /**
         * The method \c ClearNetworkInterfaceList clears
         * current interface list
         *
         * @returns Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        Result ClearNetworkInterfaceList();
        
    public:
        /**
         * Check is module options are valid
         * 
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything is fine. Options are valid
         * @retval ERR_EMPTY    Current module name is empty
         * @retval ERR_INVALID_ARG Current module name or other options are invalid
         */
        Result CheckValidity() const;
        /**
        * The method \c GetExecutableName returns the name of the application
        *
        * @return The Application/Executable name.
        */
        std::string GetExecutableName() const;
    };
}

#endif // !defined(_FEP_MODULE_OPTIONS_H_)
