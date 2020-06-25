/**
 * Declaration of the Class IPropertyTreeBase.
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

#pragma once
#include "fep3/components/legacy/property_tree/property_intf.h"

namespace fep
{
    /**
        * Access interface for property tree. It provides access the following
        * two ways:
        * <ul>
        *     <li>based on a given path (as string)</li>
        *     <li>object-oriented</li>
        * </ul>
        * See \ref sec_properties "Properties" for more informations about properties in FEP.
        */
    class FEP_PARTICIPANT_EXPORT IPropertyTreeBase
    {

    public:
        /**
            * DTOR
            */
        virtual ~IPropertyTreeBase ()
        {
        }

        /**
            * The method \c GetLocalProperty returns the property according to the path.
            * If the property does not exist, NULL will be returned.
            * Remember, the ownership of the property lies at the property tree.
            * 
            * @param [in] strPropPath  The path to the property.
            * @returns  The property if exists, NULL otherwise.
            */
        virtual IProperty const * GetProperty(char const * strPropPath) const = 0;

        /**
            * The method \c GetLocalProperty returns the property according to the path.
            * If the property does not exist, NULL will be returned.
            * Remember, the ownership of the property lies at the property tree.
            * 
            * @param [in] strPropPath  The path to the property.
            * @returns  The property if exists, NULL otherwise.
            */
        virtual IProperty * GetProperty(char const * strPropPath) = 0;

        /**
            * The method \c SetPropertyValue sets the value of a specific property. If the property does
            * not yet exists, it is created.
            * 
            * @param [in] strPropPath  The path to the property.
            * @param [in] strValue  The new string value of the property.
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
        virtual Result SetPropertyValue(char const * strPropPath, char const * strValue) =0;

        /**
            * \overload
            * 
            * @param [in] strPropPath  The path to the property.
            * @param [in] f64Value  The double value.
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
        virtual Result SetPropertyValue(char const * strPropPath, double const f64Value) =0;

        /**
            * \overload
            * 
            * @param [in] strPropPath  The path to the property.
            * @param [in] n32Value  The integer value.
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
        virtual Result SetPropertyValue(char const * strPropPath, int32_t const n32Value) =0;

        /**
            * \overload
            * 
            * @param [in] strPropPath  The path to the property.
            * @param [in] bValue  The boolean value.
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
        virtual Result SetPropertyValue(char const * strPropPath, bool const bValue) =0;

        /**
            * The method \c GetPropertyValue directly gets the value of a specific property.
            *
            * @param [in] strPropPath The path to the property.
            * @param [out] strValue The destination variable for the string value.
            * @returns Standard result code.
            * @retval ERR_NOERROR Everything went fine.
            * @retval ERR_PATH_NOT_FOUND The property does not exist at this path.
            * @retval ERR_INVALID_TYPE The property was found but it's not a string.
            * @retval ERR_POINTER The property path pointer is null.
            */
        virtual Result GetPropertyValue(const char * strPropPath,
            const char *& strValue) const =0;

        /**
            * \overload
            *
            * @param [in] strPropPath The path to the property.
            * @param [out] fValue The destination variable for the float value.
            * @returns Standard result code.
            * @retval ERR_NOERROR Everything went fine.
            * @retval ERR_PATH_NOT_FOUND The property does not exist at this path.
            * @retval ERR_INVALID_TYPE The property was found but it's not a number.
            * @retval ERR_POINTER The property path pointer is null.
            */
        virtual Result GetPropertyValue(const char * strPropPath,
            double & fValue) const =0;

        /**
            * \overload
            *
            * @param [in] strPropPath The path to the property.
            * @param [out] nValue The destination variable for the integer value.
            * @returns Standard result code.
            * @retval ERR_NOERROR Everything went fine.
            * @retval ERR_PATH_NOT_FOUND The property does not exist at this path.
            * @retval ERR_INVALID_TYPE The property was found but it's not a number.
            * @retval ERR_POINTER The property path pointer is null.
            */
        virtual Result GetPropertyValue(const char * strPropPath,
            int32_t & nValue) const =0;

        /**
            * \overload
            *
            * @param [in] strPropPath The path to the property.
            * @param [out] bValue The destination variable for the boolean value.
            * @returns Standard result code.
            * @retval ERR_NOERROR Everything went fine.
            * @retval ERR_PATH_NOT_FOUND The property does not exist at this path.
            * @retval ERR_INVALID_TYPE The property was found but it's not a boolean.
            * @retval ERR_POINTER The property path pointer is null.
            */
        virtual Result GetPropertyValue(const char * strPropPath,
            bool & bValue) const =0;

        /**
            * The method \c RegisterListener registers a property listener to the property at
            * the given path.
            *
            * \note If strPropertyPath is empty, the listener will be registered on the hidden
            * root property meaning you will get updates for _all_ properties in the tree.
            *
            * @param [in] strPropertyPath  The path to the property.
            * @param [in] poListener  The listener.
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            * @retval ERR_PATH_NOT_FOUND  The property could not be found at that path.
            */
        virtual Result RegisterListener(char const * strPropertyPath,
            IPropertyListener* const poListener) =0;

        /**
            * The method \c UnregisterListener unregisters a listener from a property.
            * 
            * @param [in] strPropertyPath  The path to the property.
            * @param [in] poListener  The listener.
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            * @retval ERR_PATH_NOT_FOUND  The property could not be found at that path.
            */
        virtual Result UnregisterListener(char const * strPropertyPath,
            IPropertyListener* const poListener) =0;

        /**
            * The method \c ClearProperty clears a property.
            * This means, that a string value will be set to "", a double value
            * to 0.0, an integer value to 0 and a bool value will be set to false.
            * All sub properties of the property will be deleted and arrays will be
            * truncated to a single element.
            * 
            * @param [in] strPropertyPath  The path to the property.
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            * @retval ERR_PATH_NOT_FOUND  The property could not be found at that path.
            */
        virtual Result ClearProperty(char const * strPropertyPath) = 0;

        /**
            * The method \c DeleteProperty deletes a property and its sub properties.
            * 
            * @param [in] strPropertyPath  The path to the property.
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            * @retval ERR_PATH_NOT_FOUND  The property could not be found at that path.
            */
        virtual Result DeleteProperty(char const * strPropertyPath) = 0;
    };
}
