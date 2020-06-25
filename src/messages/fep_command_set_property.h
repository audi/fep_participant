/**
 * Declaration of the Class cSetPropertyCommand.
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

#if !defined(EA_9CB2D2F3_A6D2_4507_A513_B1F9BE9F1C72__INCLUDED_)
#define EA_9CB2D2F3_A6D2_4507_A513_B1F9BE9F1C72__INCLUDED_

#include <cstddef>
#include <cstdint>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "messages/fep_command_set_property_intf.h"
#include "messages/fep_message.h"

namespace fep
{
    /**
     * This class represents a Set Property Command. Use it to send such a command via
     * \c ICommandAccess.
     */
    class FEP_PARTICIPANT_EXPORT cSetPropertyCommand : public cMessage,
                                                   public ISetPropertyCommand
    {
        /// D-pointer to private implementation
        FEP_UTILS_D(cSetPropertyCommand);

    public:
        /**
         * \copydoc cMessage::cMessage(const char* , const char* , timestamp_t, timestamp_t)
         * 
         * @param [in] bValue  The value the property will be set to.
         * @param [in] strPropertyPath  The path to the property.
         */
        cSetPropertyCommand(bool bValue, char const * strPropertyPath, 
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * \overload
         */
        cSetPropertyCommand(double f64Value, char const * strPropertyPath, 
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * \overload
         */
        cSetPropertyCommand(int32_t n32Value, char const * strPropertyPath, 
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * \overload
         */
        cSetPropertyCommand(char const * strValue, char const * strPropertyPath, 
            const char* strSender, const char* strReceiver,
            timestamp_t tmTimeStamp, timestamp_t tmSimTime);

        /**
         * CTOR
         * 
         * @param [in] strSetPropertyCommand  A string representation of a Set Property Command
         */
        cSetPropertyCommand(char const * strSetPropertyCommand);

        /**
         * @brief cSetPropertyCommand Copy Constructor
         * @param oOther Other instance to copy from.
         */
        cSetPropertyCommand(const cSetPropertyCommand& oOther);

        /**
         * @brief operator = cSetPropertyCommand Assignment Operator
         * @param oOther Other instance to copy from.
         * @retval *this
         */
        cSetPropertyCommand& operator= (const cSetPropertyCommand& oOther);

        /**
         * DTOR
         */
        virtual ~cSetPropertyCommand();

    public: // implements IValueAccess
        char const * GetPropertyPath() const;
        fep::Result SetValue(char const * strValue);
        fep::Result SetValue(const double f64Value);
        fep::Result SetValue(const int32_t n32Value);
        fep::Result SetValue(const bool bValue);
        fep::Result GetValue(const char*& strValue, size_t szIndex = 0) const;
        fep::Result GetValue(bool & bValue, size_t szIndex = 0) const;
        fep::Result GetValue(double & f64Value, size_t szIndex = 0) const;
        fep::Result GetValue(int32_t & n32Value, size_t szIndex = 0) const;
        bool IsBoolean() const;
        bool IsFloat() const;
        bool IsInteger() const;
        bool IsNumeric() const;
        bool IsString() const;
        bool IsArray() const;
        size_t GetArraySize() const;
        fep::Result AppendValue(char const * strValue);
        fep::Result AppendValue(double fValue);
        fep::Result AppendValue(int32_t nValue);
        fep::Result AppendValue(bool bValue);

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
#endif // !defined(EA_9CB2D2F3_A6D2_4507_A513_B1F9BE9F1C72__INCLUDED_)
