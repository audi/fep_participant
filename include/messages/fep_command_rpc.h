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

#if !defined(RPC_COMMAND_INCLUDED)
#define RPC_COMMAND_INCLUDED

#include <cstdint>
#include <string>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_command_rpc_intf.h"
#include "messages/fep_message.h"

namespace fep
{
    /**
     * This class represents a custom command.
     */
    class FEP_PARTICIPANT_EXPORT cRPCCommand :
            public cMessage,
            public IRPCCommand
    {
        /// D-pointer to private implementation
        FEP_UTILS_D(cRPCCommand);
 
    protected:
        /**
         * @brief cCustomCommand Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cRPCCommand(const cRPCCommand& oOther) = delete;

        /**
         * @brief operator = cCustomCommand Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cRPCCommand& operator= (const cRPCCommand& oOther) = delete;

    public:

         /**
         * CTOR
         *
         * @param[in] eType The type of the command request / response.
         * @param[in] strSender  The sender of the command
         * @param[in] strReceiver  The intended receiver of the command.
         * @param[in] strRPCObjectServerName The RPC server name.
         * @param[in] nRequestId The request ID.
         * @param[in] strContent The command content.
         * @param[in] tmTimeStamp  The timestamp, when the command was created.
         * @param[in] tmSimTime  The simulation time, when the command was created.
         */
        cRPCCommand(IRPCCommand::eRPCCommandType eType,
                    const std::string& strSender,
                    const std::string& strReceiver,
                    const std::string& strRPCObjectServerName,
                    uint32_t nRequestId,
                    const std::string& strContent,
                    timestamp_t tmTimeStamp,
                    timestamp_t tmSimTime);

        /**
         * CTOR
         *
         * @param [in] strJSONCommand  A JSON string representation of the command.
         */
        cRPCCommand(char const * strJSONCommand);

        /**
         * DTOR
         */
        virtual ~cRPCCommand();

    public: // implements IRPCCommand
        virtual const char* GetRPCContent() const;
        virtual const char* GetRPCServerObject() const;
        virtual uint32_t GetRequestid() const;
        virtual eRPCCommandType GetType() const;

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

#endif // !defined(RPC_COMMAND_INCLUDED)
