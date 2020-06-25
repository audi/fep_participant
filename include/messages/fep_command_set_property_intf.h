/**
 * Declaration of the Class ISetPropertyCommand.
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

#if !defined(EA_DA0A0353_D482_4033_924D_574B8A8FB33A__INCLUDED_)
#define EA_DA0A0353_D482_4033_924D_574B8A8FB33A__INCLUDED_

#include "messages/fep_command_intf.h"

namespace fep
{
    /**
     * This is the interface for a set property command.
     */
    class FEP_PARTICIPANT_EXPORT ISetPropertyCommand : public ICommand
    {

    public:
        /**
         * DTOR
         */
        virtual ~ISetPropertyCommand() = default;

        /**
         * The method \c GetPropertyPath returns the path to the property.
         * 
         * @returns The path.
         * @retval ERR_NOERROR  Everything went fine
         * @retval [...]
         */
        virtual char const * GetPropertyPath() const =0;

                    /**
         * The method \c SetValue changes the stored value.
         * \note If the value is an array it is truncated by this call!
         * 
         * @param [in] strValue  The new string value
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_POINTER  The given string pointer is NULL
         * @retval ERR_<ANY>    Any error code indicates failure
         */
        virtual fep::Result SetValue(char const * strValue) = 0;

        /**
         * \overload
         * 
         * @param [in] f64Value  The new double value.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_<ANY>    Any error code indicates failure
         */
        virtual fep::Result SetValue(const double f64Value) = 0;

        /**
         * \overload
         * 
         * @param [in] n32Value  The new integer value.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_<ANY>    Any error code indicates failure
         */
        virtual fep::Result SetValue(const int32_t n32Value) = 0;

        /**
         * \overload
         * 
         * @param [in] bValue  The new bool value.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_<ANY>    Any error code indicates failure
         */
        virtual fep::Result SetValue(const bool bValue) = 0;

        /**
         * The method \c GetValue returns the stored value.
         * 
         * @param [out] strValue      A destination pointer for the returned string.
         * @param [in] szIndex        The optional index to access array elements.
         * @retval ERR_NOERROR        Everything went fine.
         * @retval ERR_INVALID_INDEX  Index is invalid.
         * @retval ERR_INVALID_TYPE   Datatype of stored value is not a boolean.
         */
        virtual fep::Result GetValue(const char*& strValue, size_t szIndex = 0) const = 0;

        /**
         * \overload
         * 
         * @param [out] bValue        A boolean the value will be written to.
         * @param [in] szIndex        The optional index to access array elements.
         * @retval ERR_NOERROR        Everything went fine
         * @retval ERR_INVALID_INDEX  Index is invalid.
         * @retval ERR_INVALID_TYPE   Datatype of stored value is not a boolean
         */
        virtual fep::Result GetValue(bool & bValue, size_t szIndex = 0) const = 0;

        /**
         * \overload
         * \note This overload will succeed even if the value is an integer (cast)
         * 
         * @param [out] f64Value      A double the value will be written to.
         * @param [in] szIndex        The optional index to access array elements.
         * @retval ERR_NOERROR        Everything went fine
         * @retval ERR_INVALID_INDEX  Index is invalid.
         * @retval ERR_INVALID_TYPE   Datatype of stored value is not a float type
         */
        virtual fep::Result GetValue(double & f64Value, size_t szIndex = 0) const = 0;

        /**
        * \overload
        * \note This overload will succeed even if the value is a float (cast)
         * 
         * @param [out] n32Value      An integer the value will be written to.
         * @param [in] szIndex        The optional index to access array elements.
         * @retval ERR_NOERROR        Everything went fine
         * @retval ERR_INVALID_INDEX  Index is invalid.
         * @retval ERR_INVALID_TYPE   Datatype of stored value is not an integer type
         */
        virtual fep::Result GetValue(int32_t & n32Value, size_t szIndex = 0) const = 0;

        /**
         * The method \c IsBoolean checks whether the stored value is a boolean.
         * 
         * @returns  True if the stored value is a boolean, false otherwise.
         */
        virtual bool IsBoolean() const = 0;

        /**
         * The method \c IsFloat checks whether the stored value is a float.
         * 
         * @returns  True if the stored value is a float, false otherwise.
         */
        virtual bool IsFloat() const = 0;

        /**
         * The method \c IsInteger checks whether the stored value is an integer.
         * 
         * @returns  True if the stored value is an integer, false otherwise.
         */
        virtual bool IsInteger() const = 0;

        /**
         * The method \c IsNumeric checks whether the stored value is a float or an integer.
         * 
         * @returns  True if the stored value is a float or an integer, false otherwise.
         */
        virtual bool IsNumeric() const = 0;

        /**
         * The method \c IsString checks whether the stored value is a string.
         * 
         * @returns  True if the stored value is a string, false otherwise.
         */
        virtual bool IsString() const = 0;

        /**
         * The method \c IsArray returns whether the stored value holds an array
         * of the stored type.
         * \note See \ref subsec_arrays for more information about arrays.
         *
         * @returns True if the stored value holds an array of the stored type.
         */
        virtual bool IsArray() const = 0;

        /**
         * \c GetArraySize returns the array size of the stored value or 1 if there's
         * only one element stored inside.
         * \note See \ref subsec_arrays for more information about arrays.
         *
         * @returns The size of the array (1 on normal, single-element properties)
         */
        virtual size_t GetArraySize() const = 0;

        /**
         * The method \c AppendValue appends the value, making the stored value
         * an array in the process.
         * \note See \ref subsec_arrays for more information about arrays.
         * 
         * @param [in] strValue  The new string value
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_POINTER  The given string pointer is NULL
         * @retval ERR_INVALID_TYPE The stored value does not hold strings.
         * @retval ERR_<ANY>    Any error code indicates failure
         */
        virtual fep::Result AppendValue(char const * strValue) = 0;

        /**
         * \overload
         * \note This overload will succeed even if the stored value contains integers (cast).
         * 
         * @param [in] fValue   The float value
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_INVALID_TYPE The stored value does not hold floats or integers.
         * @retval ERR_<ANY>    Any error code indicates failure
         */
        virtual fep::Result AppendValue(double fValue) = 0;

        /**
        * \overload
        * \note This overload will succeed even if the stored value contains floats (cast).
         * 
         * @param [in] nValue   The integer value
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_INVALID_TYPE The stored value does not hold integers or floats.
         * @retval ERR_<ANY>    Any error code indicates failure
         */
        virtual fep::Result AppendValue(int32_t nValue) = 0;

        /**
         * \overload
         * 
         * @param [in] bValue   The bool value
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_INVALID_TYPE The stored value does not hold booleans.
         * @retval ERR_<ANY>    Any error code indicates failure
         */
        virtual fep::Result AppendValue(bool bValue) = 0;
    };
} // namespace fep
#endif // !defined(EA_DA0A0353_D482_4033_924D_574B8A8FB33A__INCLUDED_)
