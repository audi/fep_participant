/**
 * Declaration of the Class cRegPropListenerCommand.
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

#if !defined(EA_2FE46CD0_C769_4279_917C_705FF2E2F6E6__INCLUDED_)
#define EA_2FE46CD0_C769_4279_917C_705FF2E2F6E6__INCLUDED_

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_command_reg_prop_listener_intf.h"
#include "messages/fep_message.h"

namespace fep
{
    /**
     * This class represents a Get Property Command.
     */
    class FEP_PARTICIPANT_EXPORT cRegPropListenerCommand : public cMessage, public IRegPropListenerCommand
    {
        /// D-pointer to private implementation
        FEP_UTILS_D(cRegPropListenerCommand);

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * 
         * @param [in] strPropertyPath  The path to the property.
         */
        cRegPropListenerCommand(char const * strPropertyPath,
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         * 
         * @param [in] strRegPropListenerCommand  A string representation of the command
         */
        cRegPropListenerCommand(char const * strRegPropListenerCommand);

        /**
         * @brief cRegPropListenerCommand Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cRegPropListenerCommand(const cRegPropListenerCommand& oOther);

        /**
         * @brief operator = cRegPropListenerCommand Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cRegPropListenerCommand& operator= (const cRegPropListenerCommand& oOther);

        /**
         * DTOR
         */
        virtual ~cRegPropListenerCommand();

    public: // implements IRegPropListenerCommand
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
#endif // !defined(EA_2FE46CD0_C769_4279_917C_705FF2E2F6E6__INCLUDED_)
