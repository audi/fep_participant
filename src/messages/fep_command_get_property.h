/**
 * Declaration of the Class cGetPropertyCommand.
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

#if !defined(EA_7F29A0E8_0213_49BC_B477_0CF35C9457C3__INCLUDED_)
#define EA_7F29A0E8_0213_49BC_B477_0CF35C9457C3__INCLUDED_

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_message.h"
#include "messages/fep_command_get_property_intf.h"

namespace fep
{
    /**
     * This class represents a Get Property Command.
     */
    class FEP_PARTICIPANT_EXPORT cGetPropertyCommand : public cMessage, public IGetPropertyCommand
    {
        /// D-pointer to private implementation
        FEP_UTILS_D(cGetPropertyCommand);

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * 
         * @param [in] strPropertyPath  The path to the property.
         */
        cGetPropertyCommand(char const * strPropertyPath,
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         * 
         * @param [in] strGetPropertyCommand  A string representation of a Get Property Command
         */
        cGetPropertyCommand(char const * strGetPropertyCommand);

        /**
         * @brief cGetPropertyCommand Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cGetPropertyCommand(const cGetPropertyCommand& oOther);

        /**
         * @brief operator = cGetPropertyCommand Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cGetPropertyCommand& operator= (const cGetPropertyCommand& oOther);

        /**
         * DTOR
         */
        virtual ~cGetPropertyCommand();

    public: // implements IGetPropertyCommand
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
#endif // !defined(EA_7F29A0E8_0213_49BC_B477_0CF35C9457C3__INCLUDED_)
