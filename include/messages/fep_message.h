/**
 * Declaration of the Class cMessage.
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

#if !defined(EA_12DA8CD4_C6A9_4ec8_96E1_93CF5FA3F8CE__INCLUDED_)
#define EA_12DA8CD4_C6A9_4ec8_96E1_93CF5FA3F8CE__INCLUDED_

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"

namespace fep
{
    /**
     * This class is the base implementation of any FEP command or notification.
     */
    class FEP_PARTICIPANT_EXPORT cMessage
    {
        /// D-pointer to private implementation
        FEP_UTILS_D(cMessage);

    public:
        /**
         * CTOR
         * 
         * @param [in] strMessage  A JSON string representation of the message.
         */
        cMessage(char const * strMessage);

        /**
         * @brief cMessage Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cMessage(const cMessage& oOther);

        /**
         * @brief operator = cMessage Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cMessage &operator= (const cMessage& oOther);

        /**
         * CTOR
         * 
         * @param [in] strSender  The sender of the command
         * @param [in] strReceiver  The intended receiver of the command.
         * @param [in] tmTimeStamp  The timestamp, when the command was created.
         * @param [in] tmSimTime  The simulation time, when the command was created.
         */
        cMessage(const char* strSender,
            const char* strReceiver, timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * DTOR
         */
        virtual ~cMessage();

        /**
         * \copydoc IMessage::GetMajorVersion
         */
        virtual uint8_t GetMajorVersion() const;

        /**
         * \copydoc IMessage::GetMinorVersion
         */
        virtual uint8_t GetMinorVersion() const;

        /**
         * \copydoc IMessage::GetReceiver
         */
        virtual char const * GetReceiver() const;

        /**
         * \copydoc IMessage::GetSender
         */
        virtual char const * GetSender() const;

        /**
         * \copydoc IMessage::GetTimeStamp
         */
        virtual timestamp_t GetTimeStamp() const;

        /**
         * \copydoc IMessage::GetSimulationTime
         */
        virtual timestamp_t GetSimulationTime() const;

        /**
         * \copydoc IMessage::ToString
         */
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


#endif // !defined(EA_12DA8CD4_C6A9_4ec8_96E1_93CF5FA3F8CE__INCLUDED_)
