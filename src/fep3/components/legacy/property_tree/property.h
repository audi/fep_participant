/**
 * Declaration of the Class cProperty.
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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <list>
#include <mutex>
#include <string>
#include <vector>
#include <a_util/variant/variant.h>

#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep3/components/legacy/property_tree/property_listener_intf.h"
#include "fep_participant_export.h"
#include "fep_result_decl.h"

class JSONNode;

namespace fep
{
    /**
    * This class implements a property.
    * See \ref sec_properties "Properties" for for more information about properties.
    */
    class FEP_PARTICIPANT_EXPORT cProperty : public IProperty
    {
    private: // types
        /// type of the property listener container
        typedef std::vector<IPropertyListener *> tPropListenerContainer;
        /// Value container typedef
        typedef std::vector<a_util::variant::Variant> tValueType;
        /// Typedef for a function pointer to a member function of IPropertyListener
        typedef a_util::result::Result (IPropertyListener::*tPropertyCallback)
            (IProperty const *, IProperty const *, char const *);

    public:
        /**
            * CTOR
            * 
            * @param [in] strName  The name of the property.
            * @param [in] strValue  The initial string value of the property.
            * @param [in] poParent  The parent property. This class will not own this object.
            */
        cProperty (std::string const & strName,
            const char * strValue,
            cProperty * const poParent = NULL);
        /**
            * CTOR
            * 
            * @param [in] strName  The name of the property.
            * @param [in] fValue  The initial float value of the property.
            * @param [in] poParent  The parent property. This class will not own this object.
            */
        cProperty (std::string const & strName,
            double fValue,
            cProperty * const poParent = NULL);
        /**
            * CTOR
            * 
            * @param [in] strName  The name of the property.
            * @param [in] nValue  The initial integer value of the property.
            * @param [in] poParent  The parent property. This class will not own this object.
            */
        cProperty (std::string const & strName,
            int32_t nValue,
            cProperty * const poParent = NULL);
        /**
            * CTOR
            * 
            * @param [in] strName  The name of the property.
            * @param [in] bValue  The initial boolean value of the property.
            * @param [in] poParent  The parent property. This class will not own this object.
            */
        cProperty (std::string const & strName,
            bool bValue,
            cProperty * const poParent = NULL);

        /**
            * DTOR
            */
        virtual ~cProperty ();

        /// copy ctor
        cProperty(const cProperty & oOther);

        /// assignment operator
        cProperty & operator=(const cProperty & oOther);

        /**
        * PropertyToJSON serializes this property into a JSON node.
        *
        * @param [out] oNode Destination JSON node
        * @return Returns standard result.
        * @retval ERR_NOERROR Everything went fine.
        */
        a_util::result::Result PropertyToJSON(JSONNode & oNode) const;

        /**
        * The method \c JSONToProperty reinitializes the property from
        * the JSON node based on the format used by \c PropertyToJSON.
        *
        * @param [in] oNode The JSON node from which to load.
        * @return Returns standard result.
        * @retval ERR_NOERROR Everything went fine.
        * @retval ERR_INVALID_ARG JSON is not a valid property
        */
        a_util::result::Result JSONToProperty(const JSONNode & oNode);

        /**
        * The method \c ValuesToJSON serializes the actual values of the property
        * into a JSON node.
        * 
        * @param [inout] oNode  The JSON node.
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        a_util::result::Result ValuesToJSON(JSONNode & oNode) const;
        
        /**
        * The method \c JSONToValues deserializes values from the JSON node
        * into this property.
        * 
        * @param [in] oNode  The JSON node.
        * @returns  Standard result code.
        * @retval ERR_INVALID_ARG  Invalid format
        * @retval ERR_NOERROR  Everything went fine
        */
        a_util::result::Result JSONToValues(const JSONNode & oNode);

        /**
        * \c ClearListeners removes all listeners from this property.
        * Needed to prevent invalid references when remote properties are deleted.
        * @return Returns standard result.
        * @retval ERR_NOERROR Everything went fine.
        */
        a_util::result::Result ClearListeners();

        /**
        * The method \c ClearSubproperties will delete all sub properties of the property.
        * 
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        */
        a_util::result::Result ClearSubproperties();

        /**
        * The method \c AddSubproperty adds a sub property to the property.
        * This property will take ownership of the sub property.
        * 
        * @param [in] poSubproperty  The property to be attached to this property.
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_POINTER  poSubproperty was NULL
        * @retval ERR_EMPTY    the name of property is empty ("")
        * @retval ERR_FAILED   the parent could not be set
        */
        a_util::result::Result AddSubproperty (cProperty * poSubproperty);

        /**
        * The method \c DeleteSubproperty will delete a sub property of the property.
        * 
        * @param [in] poSubproperty  The sub property to be deleted.
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_NOT_FOUND  The passed property is not a sub property.
        */
        a_util::result::Result DeleteSubproperty (IProperty const * poSubproperty);

        /**
        * The method \c SetParent replaces the current parent with a new parent.
        * The new parent will also take over the ownership of this property.
        * If \a poParent is NULL, the caller takes ownership of this property.
        * 
        * @param [in] poParent  The new parent.
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_POINTER  poSubproperty was NULL
        * @retval ERR_EMPTY    the name of property is empty ("")
        * @retval ERR_FAILED   the parent could not be set
        */
        a_util::result::Result SetParent(cProperty * const poParent);

        /**
        * This templated \c GetOrMakeSubproperty looks for the subproperty at the
        * relative path, creating path elements on the way if needed
        * (empty string values, last one with the provided value)
        * @param [in] strRelativePath The relative path to the subproperty.
        * @param [in] bMake Flag to signal whether properties along the path should be created.
        * @param [in] value The templated value to use for the last property.
        * @return Returns the property.
        */
        template <typename T>
        IProperty * GetOrMakeSubproperty(const char * strRelativePath, bool bMake, T value);

        /**
        * The static helper function \c CopyPropertyValue copies the value of poProperty
        * into poDestProperty using only their public interface.
        * \note Only the value is copied, no name, children nor parent information!
        *
        * @param [in] poProperty Input property.
        * @param [in] poDestProperty Output property.
        * @return Returns standard result.
        * @retval ERR_POINTER One of the properties is NULL.
        * @retval ERR_NOERROR Everything went fine.
        */
        static a_util::result::Result CopyPropertyValue(const IProperty * poProperty,
            IProperty * poDestProperty);


    public: // implements IProperty
        IProperty const * GetSubproperty(const char * strRelativePath) const;
        IProperty * GetSubproperty(const char * strRelativePath);
        a_util::result::Result SetSubproperty(const char * strName, char const * strValue);
        a_util::result::Result SetSubproperty(const char * strName, double f64Value);
        a_util::result::Result SetSubproperty(const char * strName, int32_t n32Value);
        a_util::result::Result SetSubproperty(const char * strName, bool bValue);
        a_util::result::Result DeleteSubproperty(const char * strName);
        char const * GetName() const;
        char const * GetPath() const;
        IProperty const * GetParent() const;
        IProperty * GetParent();
        IProperty::tPropertyList const & GetSubProperties() const;
        tPropertyList::iterator GetBeginIterator();
        tPropertyList::iterator GetEndIterator();
        tPropertyList::const_iterator GetBeginIterator() const;
        tPropertyList::const_iterator GetEndIterator() const;
        a_util::result::Result RegisterListener(IPropertyListener * const poListener);
        a_util::result::Result UnregisterListener(IPropertyListener * const poListener);
        a_util::result::Result SetName(char const * strName);
        char const * ToString();
        a_util::result::Result SetValue(char const * strValue);
        a_util::result::Result SetValue(const double f64Value);
        a_util::result::Result SetValue(const int32_t n32Value);
        a_util::result::Result SetValue(const bool bValue);
        a_util::result::Result GetValue(const char*& strValue, size_t szIndex = 0) const;
        a_util::result::Result GetValue(bool & bValue, size_t szIndex = 0) const;
        a_util::result::Result GetValue(double & f64Value, size_t szIndex = 0) const;
        a_util::result::Result GetValue(int32_t & n32Value, size_t szIndex = 0) const;
        bool IsBoolean() const;
        bool IsFloat() const;
        bool IsInteger() const;
        bool IsNumeric() const;
        bool IsString() const;
        bool IsArray() const;
        size_t GetArraySize() const;
        a_util::result::Result AppendValue(char const * strValue);
        a_util::result::Result AppendValue(double fValue);
        a_util::result::Result AppendValue(int32_t nValue);
        a_util::result::Result AppendValue(bool bValue);

    private: // methods

        /**
        *  The helper method \c FindSubproperty tries to find a subproperty based on its name.
        * @param [in] strName The name of the subproperty.
        * @return Returns an iterator pointing into the subpropertylist.
        */
        IProperty::tPropertyList::iterator FindSubproperty(const char * strName);

        /**
            * The method \c NotifyListeners will call all listeners callback method \a pCallback.
            * 
            * @param [in] poProperty  The affected property.
            * @param [in] pCallback  The callback to be called.
            * @returns  Standard result code.
            * @retval ERR_NOERROR  Everything went fine
            */
        a_util::result::Result NotifyListeners(cProperty * poProperty, tPropertyCallback pCallback) const;

    private: // members
        /// The parent of the property.
        cProperty * m_poParent;
        /// The name of the property.
        std::string m_strName;
        /// The path to the property.
        std::string m_strPath;
        /// The values of the property.
        tValueType m_vecValues;
        /// The sub properties of the property.
        IProperty::tPropertyList m_lstSubproperties;
        /// The registered property listeners.
        tPropListenerContainer m_vecListeners;
        /// The critical section to protect the listeners
        std::recursive_mutex m_oCritSecListeners;
        /// Holds the debug string representation created by ToString
        std::string m_strDebugString;
    };

    template <typename T>
    IProperty * cProperty::GetOrMakeSubproperty(const char * strRelativePath, bool bMake, T value)
    {
        if (!strRelativePath)
        {
            return NULL;
        }

        std::string strPath(strRelativePath);
        if (strPath.empty())
        {
            return this;
        }

        std::string strCurrent;
        size_t nIndex = static_cast<int>(strPath.find('.'));
        if (nIndex != std::string::npos)
        {
            strCurrent = strPath.substr(0, nIndex);
            strPath.erase(0, nIndex + 1);
        }
        else
        {
            strCurrent = strPath;
            strPath.clear();
        }

        IProperty::tPropertyList::iterator itProp = FindSubproperty(strCurrent.c_str());
        cProperty * poNext = NULL;
        if (itProp == m_lstSubproperties.end())
        {
            if (bMake)
            {
                if (strPath.empty()) // i.e this is the last property in the path
                {
                    poNext = new cProperty(strCurrent.c_str(), value, this);
                }
                else
                {
                    // its a "folder" property on the way to the specified path, use empty string
                    poNext = new cProperty(strCurrent, "", this);
                }
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            poNext = dynamic_cast<cProperty *>(*itProp);
            assert(poNext != NULL);
        }
            
        return poNext->GetOrMakeSubproperty(strPath.c_str(), bMake, value);
    }
}
