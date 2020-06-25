/**
 * Declaration of the Class cCustomCommand.
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

#if !defined(EA_9C47B9CE_67ED_463e_A405_2A5D854B013A__INCLUDED_)
#define EA_9C47B9CE_67ED_463e_A405_2A5D854B013A__INCLUDED_

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_command_custom_intf.h"
#include "messages/fep_message.h"

namespace fep
{
    /**
     * This class represents a custom command.
     */
    class FEP_PARTICIPANT_EXPORT cCustomCommand :
            public cMessage,
            public ICustomCommand
    {
        /// D-pointer to private implementation
        FEP_UTILS_D(cCustomCommand);

    public:

        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * 
         * @param [in] strCommand  The name of the command.
         * @param [in] strParameterTree  The parameters of the command formatted as JSON string.
         *
         * @note Depending on the complexity of the JSON formatted parameter list, simple
         * string operations may not be sufficient. It is recommended to parse the JSON
         * statement through either of the two following methods: <br>
         * a) Standard Regular Expression evaluators. <br>
         * Attention: carefully take newlines and tabulator characters in the parameter
         * list into account when creating regular expressions! <br>
         * b) A full-spec JSON parser: LibJSON from http://sourceforge.net/projects/libjson/
         * has proven to be efficient and reliable.
         */
        cCustomCommand(const char* strCommand, const char* strParameterTree,
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         *
         * @param [in] strCustomCommand  A JSON string representation of the command.
         */
        cCustomCommand(char const * strCustomCommand);

        /**
         * @brief cCustomCommand Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cCustomCommand(const cCustomCommand& oOther);

        /**
         * @brief operator = cCustomCommand Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cCustomCommand& operator= (const cCustomCommand& oOther);

        /**
         * DTOR
         */
        virtual ~cCustomCommand();

    public: // implements ICustomCommand
        virtual char const * GetName() const;
        virtual char const * GetParameters() const;

    public: // implements IMessage
        virtual uint8_t GetMajorVersion() const;
        virtual uint8_t GetMinorVersion() const;
        virtual char const * GetReceiver() const;
        virtual char const * GetSender() const;
        virtual timestamp_t GetTimeStamp() const;
        virtual timestamp_t GetSimulationTime() const;
        virtual char const * ToString() const;

    private:
        /**
         * The method \c CreateStringRepresentation creates an internally stored string representation
         * of the message (JSON syntax).
         * 
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        fep::Result CreateStringRepresentation();
    };
}; // namespace fep

#endif // !defined(EA_9C47B9CE_67ED_463e_A405_2A5D854B013A__INCLUDED_)
