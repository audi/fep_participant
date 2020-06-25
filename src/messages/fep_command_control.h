/**
 * Declaration of the Class cControlCommand.
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

#if !defined(EA_1AB2AB31_6F26_4705_BBB2_4FFA0BE20603__INCLUDED_)
#define EA_1AB2AB31_6F26_4705_BBB2_4FFA0BE20603__INCLUDED_

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_command_control_intf.h"
#include "messages/fep_control_event.h"
#include "messages/fep_message.h"

namespace fep
{
    /**
     * This class represents a control command.
     */
    class FEP_PARTICIPANT_EXPORT cControlCommand :
            public virtual cMessage,
            public IControlCommand
    {
        /// D-pointer to private implementation
        FEP_UTILS_D(cControlCommand);

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * 
         * @param [in] eEvent  The control event.
         */
        cControlCommand(tControlEvent eEvent,
            const char* strSender, const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         *
         * @param [in] strControlCommand  A JSON string representation of the command.
         */
        cControlCommand(char const * strControlCommand);

        /**
         * @brief cControlCommand Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cControlCommand(const cControlCommand& oOther);

        /**
         * @brief operator = cControlCommand Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cControlCommand& operator= (const cControlCommand& oOther);

        /**
         * DTOR
         */
        virtual ~cControlCommand();

    public: // implements IControlCommand
        virtual tControlEvent GetEvent() const;

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


#endif // !defined(EA_1AB2AB31_6F26_4705_BBB2_4FFA0BE20603__INCLUDED_)
