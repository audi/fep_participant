/**
 * Declaration of the Class IMessage.
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

#if !defined(EA_F00BD3D2_10DC_4fe8_8356_8FFAFB300DF0__INCLUDED_)
#define EA_F00BD3D2_10DC_4fe8_8356_8FFAFB300DF0__INCLUDED_

#include "fep_types.h"

namespace fep
{
    /**
     * This is the base interface for all messages in FEP.
     */
    class FEP_PARTICIPANT_EXPORT IMessage
    {

    public:

        /**
         * DTOR
         */
        virtual ~IMessage() = default;

        /**
         * The method \c GetMajorVersion returns the major version this message is using.
         * 
         * @returns  The major version used in this message.
         */
        virtual uint8_t GetMajorVersion() const =0;

        /**
         * The method \c GetMinorVersion returns the minor version this message is using.
         *
         * @returns  The minor version used in this message.
         */
        virtual uint8_t GetMinorVersion() const =0;


        /**
         * The method \c GetReceiver returns the intended receiver of this message.
         * 
         * @returns  The receiver.
         */
        virtual char const * GetReceiver() const =0;

        /**
         * The method \c GetSender returns the sender of this message.
         * 
         * @returns  The sender.
         */
        virtual char const * GetSender() const =0;

        /**
         * The method \c GetTimeStamp returns the timestamp this message was created and sent in microseconds.
         * 
         * @returns  The time stamp.
         */
        virtual timestamp_t GetTimeStamp() const =0;

        /**
         * The method \c GetSimulationTime returns the timestamp this message was created
         * and sent, in simulation time. This may be 0 if the element is not in FS_RUNNING.
         * 
         * @returns  The simulation time stamp.
         */
        virtual timestamp_t GetSimulationTime() const =0;

        /**
         * The method \c ToString returns a JSON string representation of the message.
         * 
         * @returns  The string representation.
         */
        virtual char const * ToString() const =0;

    };
}// namespace fep
#endif // !defined(EA_F00BD3D2_10DC_4fe8_8356_8FFAFB300DF0__INCLUDED_)
