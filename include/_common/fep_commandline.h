/**
* Declaration of the Class cCommandLine.
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

#ifndef _FEP_COMMAND_LINE_H_
#define _FEP_COMMAND_LINE_H_

#include <string>
#include <vector>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"

namespace fep
{
    /// Commandline parser
    class FEP_PARTICIPANT_EXPORT cCommandLine
    {
        /// D-pointer to private implementation
        FEP_UTILS_D(cCommandLine);

    public:
        /**
        * CTOR
        */
        cCommandLine();
        /**
        * CTOR
        *
        * @param [in]  rhs other instance to copy from
        */
        cCommandLine(const cCommandLine& rhs);

        /**
         * CTOR
         *
         * @param [in]  rhs other instance to move from
         */
        cCommandLine(cCommandLine&& rhs);
        /**
        * DTOR
        */
        virtual ~cCommandLine();

        /**
        * Copy operator
        *
        * @param [in] rhs other instance to copy from
        * @return current instance
        */
        cCommandLine& operator=(const cCommandLine& rhs);

        /**
        * move operator
        *
        * @param [in] rhs other instance to move from
        * @return current instance
        */
        cCommandLine& operator=(cCommandLine&& rhs);
        /**
        * Parse command line options
        *
        * @param [in,out] argc Program argument count
        * @param [in,out] argv Program arguments
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_INVALID_ARG Error in case of no option prefix / unknown argument / missing option value
        */
        fep::Result ParseArgs(int argc, const char** argv);
        /**
        * Returns true if any kind of help flag is parsed by ParseArgs functions
        *
        * @return true if help flag is received, else false
        */
        bool IsHelpRequested();
        /**
        * Returns true if any kind of version flag is parsed by ParseArgs functions
        *
        * @return true if version flag is received, else false
        */
        bool IsVersionRequested();
        /**
        * prints to std::cout all information about the known flags/options that are set to cCommandLine
        * printing style depends on clara lib
        *
        */
        void PrintHelp();
        /**
        * The method \c GetTransmission returns the transmission type
        *
        * @returns  transmission type as string
        */
        std::string GetTransmission();
        /**
        * The method \c GetDomain returns the domain id
        *
        * @returns  The FEP Participants domain id.
        */
        std::string GetDomain();
        /**
        * The method \c GetInterface returns a list of interfaces
        *
        * @returns  The FEP Participants interfaces that got parsed
        */
        std::vector<std::string> GetInterface();
        /**
        * The method \c GetParticipantName returns the participant name
        *
        * @return The FEP Participant's name.
        */
        std::string GetParticipantName();
        /**
        * The method \c GetExeName returns the name of the parsed executable
        *
        * @return Name of the executable
        */
        std::string GetExeName();
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
        fep::Result SetAdditionalOption(std::string& strMonitoredValue,
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
        fep::Result SetAdditionalOption(bool& bMonitoredValue,
            const std::string& strShortcutCall,
            const std::string& strFullnameCall,
            const std::string& strHelpText);
    };
}
#endif // _FEP_COMMAND_LINE_H_

