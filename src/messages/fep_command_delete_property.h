/**
 * Declaration of the Class cDeletePropertyCommand.
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

#ifndef __FEP_COMMAND_DELETE_PROPERTY_H
#define __FEP_COMMAND_DELETE_PROPERTY_H

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_message.h"
#include "messages/fep_command_delete_property_intf.h"

namespace fep
{
    /**
     * This class represents a Delete Property Command.
     */
    class FEP_PARTICIPANT_EXPORT cDeletePropertyCommand : public cMessage, public IDeletePropertyCommand
    {
        /// D-pointer to private implementation
        FEP_UTILS_D(cDeletePropertyCommand);

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * 
         * @param [in] strPropertyPath  The path to the property.
         */
        cDeletePropertyCommand(char const * strPropertyPath,
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         * 
         * @param [in] strDeletePropertyCommand  A string representation of a Delete Property Command
         */
        cDeletePropertyCommand(char const * strDeletePropertyCommand);

        /**
         * @brief cDeletePropertyCommand Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cDeletePropertyCommand(const cDeletePropertyCommand& oOther);

        /**
         * @brief operator = cDeletePropertyCommand Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cDeletePropertyCommand& operator= (const cDeletePropertyCommand& oOther);

        /**
         * DTOR
         */
        virtual ~cDeletePropertyCommand();

    public: // implements IDeletePropertyCommand
        virtual char const * GetPropertyPath() const;

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
    };
}; // namespace fep
#endif // __FEP_COMMAND_DELETE_PROPERTY_H
