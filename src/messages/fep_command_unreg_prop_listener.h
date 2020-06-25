/**
 * Declaration of the Class cUnregPropListenerCommand.
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

#if !defined(EA_FA61F8CD_25DD_4B83_9E70_5B63CDADA273__INCLUDED_)
#define EA_FA61F8CD_25DD_4B83_9E70_5B63CDADA273__INCLUDED_

#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_command_unreg_prop_listener_intf.h"
#include "messages/fep_message.h"

namespace fep
{
    /**
     * This class represents a unreg Property Command.
     */
    class FEP_PARTICIPANT_EXPORT cUnregPropListenerCommand : public cMessage, public IUnregPropListenerCommand
    {
        /// D-pointer to private implementation
        FEP_UTILS_D(cUnregPropListenerCommand);

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * 
         * @param [in] strPropertyPath  The path to the property.
         */
        cUnregPropListenerCommand(char const * strPropertyPath,
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         * 
         * @param [in] strUnregPropListenerCommand  A string representation of the command
         */
        cUnregPropListenerCommand(char const * strUnregPropListenerCommand);

        /**
         * @brief cUnregPropListenerCommand Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cUnregPropListenerCommand(const cUnregPropListenerCommand& oOther);

        /**
         * @brief operator = cUnregPropListenerCommand Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cUnregPropListenerCommand& operator= (const cUnregPropListenerCommand& oOther);

        /**
         * DTOR
         */
        virtual ~cUnregPropListenerCommand();

    public: // implements IUnregPropListenerCommand
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
#endif // !defined(EA_FA61F8CD_25DD_4B83_9E70_5B63CDADA273__INCLUDED_)
