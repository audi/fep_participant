/**
 * Declaration of the Class ICustomCommand.
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

#if !defined(EA_D2A38F94_DC71_4be1_905C_778705E31CE1__INCLUDED_)
#define EA_D2A38F94_DC71_4be1_905C_778705E31CE1__INCLUDED_

#include "messages/fep_command_intf.h"

namespace fep
{
    /**
     * This is the interface for a custom command.
     */
    class FEP_PARTICIPANT_EXPORT ICustomCommand : public ICommand
    {

    public:
        /**
         * DTOR
         */
        virtual ~ICustomCommand() = default;

        /**
         * The method \c GetName returns the name of the Custom Command.
         * 
         * @returns  The name.
         */
        virtual char const * GetName() const =0;

        /**
         * The method \c GetParameters returns a string representing the parameters of the
         * command. The string will be a JSON string.
         *
         * @note Depending on the complexity of the JSON formatted parameter list, simple
         * string operations may not be sufficient. It is recommended to parse the JSON
         * statement through either of the two following methods: <br>
         * a) Standard Regular Expression evaluators. <br>
         * Attention: carefully take newlines and tabulator characters in the parameter
         * list into account when creating regular expressions! <br>
         * b) A full-spec JSON parser: LibJSON from http://sourceforge.net/projects/libjson/
         * has proven to be efficient and reliable.
         * 
         * @returns The parameter list in full JSON format, including \\n and \\t.
         */
        virtual char const * GetParameters() const =0;

    };
} // namespace fep
#endif // !defined(EA_D2A38F94_DC71_4be1_905C_778705E31CE1__INCLUDED_)
