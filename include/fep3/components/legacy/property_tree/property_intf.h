/**
 * Declaration of the Class IProperty.
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
#include <list>
#include "fep3/components/legacy/property_tree/property_listener_intf.h"
#include "fep_types.h"

namespace fep
{
    class IPropertyListener;

    /**
    * This is the interface representing a property.
    */
    class FEP_PARTICIPANT_EXPORT IProperty
    {

    public:
        // types
        /// Typedef for a list of properties.
        typedef std::list<IProperty *> tPropertyList;

    public:
        /**
        * DTOR
        */
        virtual ~IProperty () = default;

        /**
        * The method \c GetSubproperty will return a const subproperty.
        * 
        * \note If strRelativePath is empty this property will be returned.
        *
        * @param [in] strRelativePath The relative path to the subproperty.
        * @returns  The const subproperty or NULL.
        */
        virtual IProperty const * GetSubproperty(const char * strRelativePath) const =0;

        /**
        * The method \c GetSubproperty will return a subproperty.
        * 
        * \note If strRelativePath is empty this property will be returned.
        *
        * @param [in] strRelativePath The relative path to the subproperty.
        * @returns  The subproperty or NULL.
        */
        virtual IProperty * GetSubproperty(const char * strRelativePath) =0;

        /**
        * The method \c SetSubproperty sets the value of a sub property to the property.
        * The subproperty will be created if the subproperty does not exist yet.
        * 
        * @param [in] strName  The name of the subproperty.
        * @param [in] strValue  The new string value of the subproperty.
        * @returns  Standard result code.
        * @retval ERR_POINTER  Name or value string pointers are NULL.
        * @retval ERR_NOERROR  Everything went fine.
        */
        virtual Result SetSubproperty(const char * strName, char const * strValue) =0;

        /**
        * The method \c SetSubproperty sets the value of a sub property to the property.
        * The subproperty will be created if the subproperty does not exist yet.
        * 
        * @param [in] strName  The name of the subproperty.
        * @param [in] f64Value  The new float value of the subproperty.
        * @returns  Standard result code.
        * @retval ERR_POINTER  Name string pointer is NULL.
        * @retval ERR_NOERROR  Everything went fine.
        */
        virtual Result SetSubproperty(const char * strName, double f64Value) =0;

        /**
        * The method \c SetSubproperty sets the value of a sub property to the property.
        * The subproperty will be created if the subproperty does not exist yet.
        *
        * @param [in] strName  The name of the subproperty.
        * @param [in] n32Value  The new integer value of the subproperty.
        * @returns  Standard result code.
        * @retval ERR_POINTER  Name string pointer is NULL.
        * @retval ERR_NOERROR  Everything went fine.
        */
        virtual Result SetSubproperty(const char * strName, int32_t n32Value) =0;

        /**
        * The method \c SetSubproperty sets the value of a sub property to the property.
        * The subproperty will be created if the subproperty does not exist yet.
        * 
        * @param [in] strName  The name of the subproperty.
        * @param [in] bValue  The new boolean value of the subproperty.
        * @returns  Standard result code.
        * @retval ERR_POINTER  Name string pointer is NULL.
        * @retval ERR_NOERROR  Everything went fine.
        */
        virtual Result SetSubproperty(const char * strName, bool bValue) =0;

        /**
        * The method \c DeleteSubproperty will delete a sub property of the property.
        * 
        * @param [in] strName  The name of the sub property to be deleted.
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_POINTER  Name string pointer is NULL.
        * @retval ERR_NOT_FOUND  The passed property is not a sub property.
        */
        virtual Result DeleteSubproperty(const char * strName) =0;
            
        /**
        * The method \c GetName returns the name of the property.
        * 
        * @returns  The name of the property.
        */
        virtual char const * GetName() const =0;

        /**
        * The method \c GetPath returns the full path of this property inside the property
        * tree.
        * 
        * @returns  The path.
        */
        virtual char const * GetPath() const =0;

        /**
        * The method \c GetParent will return its parent property.
        *
        * @returns  The parent property.
        */
        virtual IProperty const * GetParent() const =0;

        /**
        * The method \c GetParent will return its parent property.
        *
        * @returns  The parent property.
        */
        virtual IProperty * GetParent() =0;

        /**
        * The method \c GetSubProperties returns a list of all sub properties.
        * Remember: the property owns the sub properties.
        * 
        * @returns  The list of all sub properties.
        */
        virtual tPropertyList const & GetSubProperties() const =0;

        /**
        * The method \c GetBeginIterator returns an stl-style iterator for the
        * subproperty list pointing to the beginning of the list.
        * See \c GetEndIterator for the respective end iterator.
        *
        * @returns Returns non-const begin iterator
        */
        virtual tPropertyList::iterator GetBeginIterator() =0;

        /**
        * The method \c GetEndIterator returns an stl-style iterator for the
        * subproperty list pointing to the end of the list.
        * See \c GetBeginIterator for the respective begin iterator.
        *
        * @returns Returns non-const end iterator
        */
        virtual tPropertyList::iterator GetEndIterator() =0;

        /**
        * The method \c GetBeginIterator returns an stl-style iterator for the
        * subproperty list pointing to the beginning of the list.
        * See \c GetEndIterator for the respective end iterator.
        *
        * @returns Returns const begin iterator
        */
        virtual tPropertyList::const_iterator GetBeginIterator() const =0;

        /**
        * The method \c GetEndIterator returns an stl-style iterator for the
        * subproperty list pointing to the end of the list.
        * See \c GetBeginIterator for the respective begin iterator.
        *
        * @returns Returns const end iterator
        */
        virtual tPropertyList::const_iterator GetEndIterator() const =0;

        /**
        * The method \c RegisterListener registers a lister to the property.
        * 
        * @param [in] poListener  The listener.
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual Result RegisterListener(IPropertyListener * const poListener) =0;

        /**
        * The method \c SetName changes the name of the property to \a strName.
        *
        * @param [in] strName  The new name of the property.
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual Result SetName(char const * strName) =0;

        /**
        * The method \c UnregisterListener removes a previously registered listener.
        * 
        * @param [in] poListener  The listener.
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        virtual Result UnregisterListener(IPropertyListener * const poListener) =0;

        /**
        * The method \c ToString creates a string representation of the whole
        * property along with any subproperties. The representation uses JSON
        * and is useful only for debugging purposes.
        *
        * \note There is no way to reconstruct a property from this representation!
        * @return Returns reprentation string.
        */
        virtual char const * ToString() =0;

        /**
        * The method \c SetValue changes the stored value.
        * \note If the value is an array it is truncated by this call!
        * 
        * @param [in] strValue  The new string value
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_POINTER  The given string pointer is NULL
        * @retval ERR_<ANY>    Any error code indicates failure
        */
        virtual Result SetValue(char const * strValue) = 0;

        /**
        * \overload
        * 
        * @param [in] f64Value  The new double value.
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_<ANY>    Any error code indicates failure
        */
        virtual Result SetValue(const double f64Value) = 0;

        /**
        * \overload
        * 
        * @param [in] n32Value  The new integer value.
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_<ANY>    Any error code indicates failure
        */
        virtual Result SetValue(const int32_t n32Value) = 0;

        /**
        * \overload
        * 
        * @param [in] bValue  The new bool value.
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_<ANY>    Any error code indicates failure
        */
        virtual Result SetValue(const bool bValue) = 0;

        /**
        * The method \c GetValue returns the stored value.
        * 
        * @param [out] strValue      A destination pointer for the returned string.
        * @param [in] szIndex        The optional index to access array elements.
        * @retval ERR_NOERROR        Everything went fine.
        * @retval ERR_INVALID_INDEX  Index is invalid.
        * @retval ERR_INVALID_TYPE   Datatype of stored value is not a boolean.
        */
        virtual Result GetValue(const char*& strValue, size_t szIndex = 0) const = 0;

        /**
        * \overload
        * 
        * @param [out] bValue        A boolean the value will be written to.
        * @param [in] szIndex        The optional index to access array elements.
        * @retval ERR_NOERROR        Everything went fine
        * @retval ERR_INVALID_INDEX  Index is invalid.
        * @retval ERR_INVALID_TYPE   Datatype of stored value is not a boolean
        */
        virtual Result GetValue(bool & bValue, size_t szIndex = 0) const = 0;

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
        virtual Result GetValue(double & f64Value, size_t szIndex = 0) const = 0;

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
        virtual Result GetValue(int32_t & n32Value, size_t szIndex = 0) const = 0;

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
        * \note See \ref subsec_arrays "Usage of Arrays in Properties" for more 
        * information about arrays.
        *
        * @returns True if the stored value holds an array of the stored type.
        */
        virtual bool IsArray() const = 0;

        /**
        * \c GetArraySize returns the array size of the stored value or 1 if there's
        * only one element stored inside.
        * \note See \ref subsec_arrays "Usage of Arrays in Properties" for more 
        * information about arrays.
        *
        * @returns The size of the array (1 on normal, single-element properties)
        */
        virtual size_t GetArraySize() const = 0;

        /**
        * The method \c AppendValue appends the value, making the stored value
        * an array in the process.
        * \note See \ref subsec_arrays "Usage of Arrays in Properties" for more 
        * information about arrays.
        * 
        * @param [in] strValue  The new string value
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_POINTER  The given string pointer is NULL
        * @retval ERR_INVALID_TYPE The stored value does not hold strings.
        * @retval ERR_<ANY>    Any error code indicates failure
        */
        virtual Result AppendValue(char const * strValue) = 0;

        /**
        * \overload
        * \note This overload will succeed even if the stored value contains integers (cast).
        * 
        * @param [in] fValue   The float value
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_INVALID_TYPE The stored value does not hold floats or integers.
        * @retval ERR_<ANY>    Any error code indicates failure
        */
        virtual Result AppendValue(double fValue) = 0;

        /**
        * \overload
        * \note This overload will succeed even if the stored value contains floats (cast).
        * 
        * @param [in] nValue   The integer value
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_INVALID_TYPE The stored value does not hold integers or floats.
        * @retval ERR_<ANY>    Any error code indicates failure
        */
        virtual Result AppendValue(int32_t nValue) = 0;

        /**
        * \overload
        * 
        * @param [in] bValue   The bool value
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_INVALID_TYPE The stored value does not hold booleans.
        * @retval ERR_<ANY>    Any error code indicates failure
        */
        virtual Result AppendValue(bool bValue) = 0;
    };
}
