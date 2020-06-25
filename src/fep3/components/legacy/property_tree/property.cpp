/**
 * Implementation of the Class cProperty.
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

#include "fep3/components/legacy/property_tree/property.h"
#include <algorithm>
#include <memory>
#include <vector>
#include <a_util/result/result_info_decl.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_functions.h>
#include <libjson.h>

#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep_errors.h"

//_MAKE_RESULT(-4, ERR_POINTER);
//_MAKE_RESULT(-5, ERR_INVALID_ARG);
//_MAKE_RESULT(-10, ERR_INVALID_INDEX);
//_MAKE_RESULT(-20, ERR_NOT_FOUND);
//_MAKE_RESULT(-38, ERR_FAILED);
//_MAKE_RESULT(-42, ERR_INVALID_TYPE);
//_MAKE_RESULT(-43, ERR_EMPTY);

using namespace fep;

// m_poParent must be initialized to NULL, so that the SetParent method will attach the property
// to the parent
cProperty::cProperty (std::string const & strName, const char * strValue, cProperty * const poParent) :
    m_poParent(NULL), m_strName(strName), m_strPath(), m_vecValues(),
    m_lstSubproperties(), m_vecListeners(), m_oCritSecListeners(), m_strDebugString()
{
    m_vecValues.push_back(a_util::variant::Variant(strValue));

    // call this method to resolve the path
    SetParent(poParent);
}

// m_poParent must be initialized to NULL, so that the SetParent method will attach the property
// to the parent
cProperty::cProperty (std::string const & strName, double fValue, cProperty * const poParent) :
    m_poParent(NULL), m_strName(strName), m_strPath(), m_vecValues(),
    m_lstSubproperties(), m_vecListeners(), m_oCritSecListeners(), m_strDebugString()
{
    m_vecValues.push_back(a_util::variant::Variant(fValue));

    // call this method to resolve the path
    SetParent(poParent);
}

// m_poParent must be initialized to NULL, so that the SetParent method will attach the property
// to the parent
cProperty::cProperty (std::string const & strName, int32_t nValue, cProperty * const poParent) :
    m_poParent(NULL), m_strName(strName), m_strPath(), m_vecValues(),
    m_lstSubproperties(), m_vecListeners(), m_oCritSecListeners(), m_strDebugString()
{
    m_vecValues.push_back(a_util::variant::Variant(nValue));

    // call this method to resolve the path
    SetParent(poParent);
}

// m_poParent must be initialized to NULL, so that the SetParent method will attach the property
// to the parent
cProperty::cProperty (std::string const & strName, bool bValue, cProperty * const poParent) :
    m_poParent(NULL), m_strName(strName), m_strPath(), m_vecValues(),
    m_lstSubproperties(), m_vecListeners(), m_oCritSecListeners(), m_strDebugString()
{
    m_vecValues.push_back(a_util::variant::Variant(bValue));

    // call this method to resolve the path
    SetParent(poParent);
}

cProperty::~cProperty ()
{
    std::unique_lock<std::recursive_mutex> guard(m_oCritSecListeners);

    NotifyListeners(this, &IPropertyListener::ProcessPropertyDelete);

    // Since the property has ownership of all children, it will delete them all
    for (IProperty::tPropertyList::iterator pIter = m_lstSubproperties.begin();
        m_lstSubproperties.end() != pIter; pIter++)
    {
        delete *pIter;
    }
}

cProperty::cProperty(const cProperty & oOther) :
    m_poParent(NULL), m_strName(oOther.m_strName), m_strPath(), m_vecValues(oOther.m_vecValues),
    m_lstSubproperties(), m_vecListeners(), m_oCritSecListeners(), m_strDebugString()
{
    // only the name and the value(s) are copied! no subproperties

    // call this method to resolve the path
    SetParent(m_poParent);
}

cProperty & cProperty::operator=(const cProperty & oOther)
{
    if (this != &oOther)
    {
        ClearSubproperties();
        ClearListeners();
        m_strDebugString.clear();
        m_strName = oOther.m_strName;
        m_vecValues = oOther.m_vecValues;
        SetParent(NULL);
    }

    return *this;
}

a_util::result::Result cProperty::ValuesToJSON(JSONNode & oNode) const
{
    const char * strVal;
    double fVal;
    int32_t nVal;
    bool bVal;

    if (!IsArray())
    {
        if (IsString())
        {
            GetValue(strVal);
            oNode = JSONNode("Value", strVal);
        }
        else if (IsFloat())
        {
            GetValue(fVal);
            oNode = JSONNode("Value", fVal);
        }
        else if (IsInteger())
        {
            GetValue(nVal);
            oNode = JSONNode("Value", nVal);
        }
        else
        {
            GetValue(bVal);
            oNode = JSONNode("Value", bVal);
        }
    }
    else
    {
        oNode = JSONNode(JSON_ARRAY);
        oNode.set_name("Value");
        for (size_t szIndex = 0; szIndex < GetArraySize(); ++szIndex)
        {
            if (IsString())
            {
                GetValue(strVal, szIndex);
                oNode.push_back(JSONNode("", strVal));
            }
            else if (IsFloat())
            {
                GetValue(fVal, szIndex);
                oNode.push_back(JSONNode("", fVal));
            }
            else if (IsInteger())
            {
                GetValue(nVal, szIndex);
                oNode.push_back(JSONNode("", nVal));
            }
            else
            {
                GetValue(bVal, szIndex);
                oNode.push_back(JSONNode("", bVal));
            }
        }
    }

    return a_util::result::SUCCESS;
}

a_util::result::Result cProperty::PropertyToJSON(JSONNode & oNode) const
{
    // set (sub)property name
    oNode.set_name(GetName());

    // add "Value" node
    JSONNode oValueNode;
    a_util::result::Result res = ValuesToJSON(oValueNode);
    if (a_util::result::isFailed(res))
    {
        return res;
    }

    oNode.push_back(oValueNode);

    // add subproperty nodes
    const IProperty::tPropertyList & oList = GetSubProperties();
    for (IProperty::tPropertyList::const_iterator itSubProp = oList.begin();
        itSubProp != oList.end(); ++itSubProp)
    {
        JSONNode oSubNode;
        cProperty * poProp = dynamic_cast<cProperty *>((*itSubProp));
        assert(poProp != NULL);
        res = poProp->PropertyToJSON(oSubNode);
        if (a_util::result::isFailed(res))
        {
            return res;
        }
        oNode.push_back(oSubNode);
    }

    return a_util::result::SUCCESS;
}

static a_util::result::Result DeserializeFirstValue(IProperty & poProp, const JSONNode & oNode)
{
    a_util::result::Result nRes;
    char nType = oNode.type();
    if (nType == JSON_STRING)
    {
        poProp.SetValue(oNode.as_string().c_str());
    }
    else if (nType == JSON_NUMBER)
    {
        std::string strNode = oNode.as_string().c_str();
        
        if (std::string::npos != strNode.find_first_of(".", 0))
        {
            double fVal = oNode.as_float();
            poProp.SetValue(fVal);
        }
        else
        {
            int32_t nVal = oNode.as_int();
            poProp.SetValue((int32_t)nVal);
        }
    }
    else if (nType == JSON_BOOL)
    {
        poProp.SetValue(oNode.as_bool());
    }
    else
    {
        nRes = ERR_INVALID_TYPE;
    }

    return nRes;
}

a_util::result::Result cProperty::JSONToValues(const JSONNode & oNode)
{
    if (oNode.name() != "Value")
    {
        return ERR_INVALID_ARG;
    }

    if (oNode.type() == JSON_ARRAY)
    {
        // one-element json arrays are not allowed because valid
        // one-element property arrays would be encoded as single value.
        if (oNode.size() <= 1)
        {
            return ERR_INVALID_ARG;
        }

        // determine array element type
        char nElementType = oNode.at(0).type();
        switch (nElementType)
        {
        case JSON_STRING: // fallthrough
        case JSON_NUMBER: // fallthrough
        case JSON_BOOL:
            break;
        default:
            return ERR_INVALID_ARG;
        }

        // set first value
        a_util::result::Result res = DeserializeFirstValue(*this, oNode.at(0));
        if (a_util::result::isFailed(res))
        {
            return res;
        }

        // add the remaining elements to the property array
        for (json_index_t nIndex = 1; nIndex < oNode.size(); ++nIndex)
        {
            const JSONNode & oSubNode = oNode.at(nIndex);
            char nType = oSubNode.type();
            if (nType != nElementType)
            {
                return ERR_INVALID_ARG;
            }

            if (nType == JSON_STRING)
            {
                AppendValue(oSubNode.as_string().c_str());
            }
            else if (nType == JSON_NUMBER)
            {
                std::string strNode = oSubNode.as_string().c_str();
                if (std::string::npos != strNode.find_first_of(".", 0))
                {
                    double fVal = oSubNode.as_float();
                    AppendValue(fVal);
                }
                else
                {
                    int32_t nVal = oSubNode.as_int();
                    AppendValue(nVal);
                }
            }
            else if (nType == JSON_BOOL)
            {
                AppendValue(oSubNode.as_bool());
            }
        }
    }
    else
    {
        a_util::result::Result res = DeserializeFirstValue(*this, oNode);
        if (a_util::result::isFailed(res))
        {
            return res;
        }
    }

    return a_util::result::SUCCESS;
}

a_util::result::Result cProperty::CopyPropertyValue(const IProperty * poProperty,
    IProperty * poDestProperty)
{
    if (!poProperty || !poDestProperty)
    {
        return ERR_POINTER;
    }

    const char * strVal = NULL;
    int32_t nVal;
    double fVal;
    bool bVal;
    if (poProperty->IsString())
    {
        poProperty->GetValue(strVal);
        poDestProperty->SetValue(strVal);
    }
    else if (poProperty->IsFloat())
    {
        poProperty->GetValue(fVal);
        poDestProperty->SetValue(fVal);
    }
    else if (poProperty->IsInteger())
    {
        poProperty->GetValue(nVal);
        poDestProperty->SetValue(nVal);
    }
    else if (poProperty->IsBoolean())
    {
        poProperty->GetValue(bVal);
        poDestProperty->SetValue(bVal);
    }

    for (size_t szIndex = 1; szIndex < poProperty->GetArraySize(); ++szIndex)
    {
        if (poProperty->IsString())
        {
            poProperty->GetValue(strVal, szIndex);
            poDestProperty->AppendValue(strVal);
        }
        else if (poProperty->IsFloat())
        {
            poProperty->GetValue(fVal, szIndex);
            poDestProperty->AppendValue(fVal);
        }
        else if (poProperty->IsInteger())
        {
            poProperty->GetValue(nVal, szIndex);
            poDestProperty->AppendValue(nVal);
        }
        else if (poProperty->IsBoolean())
        {
            poProperty->GetValue(bVal, szIndex);
            poDestProperty->AppendValue(bVal);
        }
    }

    return a_util::result::SUCCESS;
}

a_util::result::Result cProperty::JSONToProperty(const JSONNode & oNode)
{
    // FIXME: only destroy this property once we know the JSON is valid
    ClearSubproperties();

    // set name
    if(a_util::result::isFailed(SetName(oNode.name().c_str())))
    {
        return ERR_INVALID_ARG;
    }

    // find value node
    JSONNode::const_iterator itNode = oNode.find("Value");
    if (oNode.end() == itNode)
    {
        return ERR_INVALID_ARG;
    }

    // set property value
    a_util::result::Result res = JSONToValues(*itNode);
    if (a_util::result::isFailed(res))
    {
        return res;
    }

    // iterate subproperties
    for (itNode = oNode.begin(); itNode != oNode.end(); ++itNode)
    {
        if (0 != itNode->size() && itNode->type() != JSON_ARRAY)
        {
            cProperty * poSubProp = new cProperty("", "");
            a_util::result::Result nRes = poSubProp->JSONToProperty(*itNode);
            if (a_util::result::isFailed(nRes))
            {
                delete poSubProp;
                return nRes;
            }

            AddSubproperty(poSubProp);
        }
    }

    return a_util::result::SUCCESS;
}

a_util::result::Result cProperty::ClearListeners()
{
    std::unique_lock<std::recursive_mutex> guard(m_oCritSecListeners);

    m_vecListeners.clear();
    return a_util::result::SUCCESS;
}

a_util::result::Result cProperty::AddSubproperty(cProperty * poSubproperty)
{
    a_util::result::Result nResult;
    if (NULL == poSubproperty)
    {
        nResult = ERR_POINTER;
    }

    if (a_util::strings::isEqual(poSubproperty->GetName(), ""))
    {
        return ERR_EMPTY;
    }

    // Only if the property is not already a sub property, it will be added and the listeners
    // will be informed. This is important, since SetParent() will also call
    // AddChildSubproperty().
    if (a_util::result::isOk(nResult) &&
        std::find(m_lstSubproperties.begin(), m_lstSubproperties.end(), poSubproperty) ==
        m_lstSubproperties.end())
    {
        m_lstSubproperties.push_back(poSubproperty);
        cProperty * poProp = dynamic_cast<cProperty *>(poSubproperty);
        assert(poProp != NULL);
        nResult = poProp->SetParent(this);
        if (a_util::result::isOk(nResult))
        {
            nResult = NotifyListeners(poSubproperty, &IPropertyListener::ProcessPropertyAdd);
        }
        else
        {
            nResult = ERR_FAILED;
        }
    }
    else
    {
        // It is ok to add an already added sub property, so we wont return any error here.
    }
    return nResult;
}

a_util::result::Result cProperty::DeleteSubproperty(IProperty const * poSubproperty)
{
    a_util::result::Result nResult;
    IProperty::tPropertyList::iterator pPropertyIter = std::find(m_lstSubproperties.begin(),
        m_lstSubproperties.end(), poSubproperty);
    if (pPropertyIter != m_lstSubproperties.end())
    {
        // setting the parent NULL doesnt mean anything but a segfault
        //(*pPropertyIter)->SetParent(NULL);
        delete *pPropertyIter;
        m_lstSubproperties.erase(pPropertyIter);
    }
    else
    {
        nResult = ERR_NOT_FOUND;
    }
    return nResult;
}

a_util::result::Result cProperty::ClearSubproperties()
{
    // Since the property has ownership of all children, delete them all
    for (IProperty::tPropertyList::iterator pIter = m_lstSubproperties.begin();
        m_lstSubproperties.end() != pIter; pIter++)
    {
        delete *pIter;
    }

    m_lstSubproperties.clear();
    return a_util::result::SUCCESS;
}

IProperty const * cProperty::GetSubproperty(const char * strRelativePath) const
{
    return const_cast<cProperty *>(this)->
        GetOrMakeSubproperty(strRelativePath, false, "");
}

IProperty * cProperty::GetSubproperty(const char * strName)
{
    return const_cast<IProperty *>(
        const_cast<const cProperty*>(this)->GetSubproperty(strName));
}

a_util::result::Result cProperty::SetSubproperty(const char * strName, char const * strValue)
{
    if (!strName || !strValue)
    {
        return ERR_POINTER;
    }

    tPropertyList::iterator itSubProp = FindSubproperty(strName);
    if (itSubProp != m_lstSubproperties.end())
    {
        return (*itSubProp)->SetValue(strValue);
    }
    else
    {
        cProperty * poProp = new cProperty(strName, strValue, this);
        (void*)poProp;
    }

    return a_util::result::SUCCESS;
}

a_util::result::Result cProperty::SetSubproperty(const char * strName, double f64Value)
{
    if (!strName)
    {
        return ERR_POINTER;
    }
    tPropertyList::iterator itSubProp = FindSubproperty(strName);
    if (itSubProp != m_lstSubproperties.end())
    {
        return (*itSubProp)->SetValue(f64Value);
    }
    else
    {
        cProperty * poProp = new cProperty(strName, f64Value, this);
        (void*)poProp;
    }

    return a_util::result::SUCCESS;
}

a_util::result::Result cProperty::SetSubproperty(const char * strName, int32_t n32Value)
{
    if (!strName)
    {
        return ERR_POINTER;
    }
    tPropertyList::iterator itSubProp = FindSubproperty(strName);
    if (itSubProp != m_lstSubproperties.end())
    {
        return (*itSubProp)->SetValue(n32Value);
    }
    else
    {
        cProperty * poProp = new cProperty(strName, n32Value, this);
        (void*)poProp;
    }

    return a_util::result::SUCCESS;
}

a_util::result::Result cProperty::SetSubproperty(const char * strName, bool bValue)
{
    if (!strName)
    {
        return ERR_POINTER;
    }
    tPropertyList::iterator itSubProp = FindSubproperty(strName);
    if (itSubProp != m_lstSubproperties.end())
    {
        return (*itSubProp)->SetValue(bValue);
    }
    else
    {
        cProperty * poProp = new cProperty(strName, bValue, this);
        (void*)poProp;
    }

    return a_util::result::SUCCESS;
}

a_util::result::Result cProperty::DeleteSubproperty(const char * strName)
{
    if (!strName)
    {
        return ERR_POINTER;
    }
    a_util::result::Result nResult;
    IProperty::tPropertyList::iterator pPropertyIter = FindSubproperty(strName);

    if (pPropertyIter != m_lstSubproperties.end())
    {
        delete *pPropertyIter;
        m_lstSubproperties.erase(pPropertyIter);
    }
    else
    {
        nResult = ERR_NOT_FOUND;
    }
    return nResult;
}

const IProperty * cProperty::GetParent () const
{
    return m_poParent;
}

IProperty * cProperty::GetParent ()
{
    return m_poParent;
}

const IProperty::tPropertyList& cProperty::GetSubProperties () const
{
    return m_lstSubproperties;
}

IProperty::tPropertyList::iterator cProperty::GetBeginIterator()
{
    return m_lstSubproperties.begin();
}

IProperty::tPropertyList::iterator cProperty::GetEndIterator()
{
    return m_lstSubproperties.end();
}

IProperty::tPropertyList::const_iterator cProperty::GetBeginIterator() const
{
    return m_lstSubproperties.begin();
}

IProperty::tPropertyList::const_iterator cProperty::GetEndIterator() const
{
    return m_lstSubproperties.end();
}

a_util::result::Result cProperty::RegisterListener (IPropertyListener * const poListener)
{
    std::unique_lock<std::recursive_mutex> guard(m_oCritSecListeners);

    a_util::result::Result nResult;
    if (NULL == poListener)
    {
        nResult = ERR_POINTER;
    }
    else
    {
        m_vecListeners.push_back(poListener);
    }
    return nResult;
}

a_util::result::Result cProperty::SetParent (cProperty * const poParent)
{
    a_util::result::Result nResult;
    // NULL is no error but means the property has no parent anymore.
    // That sounds very sad but I think, little property will be fine without parents.
    // Maybe it will even become BATMAN.

    if (m_poParent != poParent)
    {
        // First lets check we are not already a (grand) parent of the new parent.
        if (NULL != m_poParent)
        {
            IProperty const * pCurrentParent = poParent;
            while (NULL != pCurrentParent || this != pCurrentParent)
            {
                pCurrentParent = pCurrentParent->GetParent();
            }

            if (this != pCurrentParent)
            {
                nResult = a_util::result::SUCCESS;
            }
            else
            {
                nResult = ERR_FAILED;
            }
        }
        // This will make sure the parent knows about us. If the parent already knows us, it will do
        // nothing.
        // It is very important to FIRST set the new parent and then call AddSuproperty, since that
        // call might lead to a SetParent call.
        m_poParent = poParent;
        if (NULL != m_poParent)
        {
            nResult = poParent->AddSubproperty(this);
        }
    }
    if (a_util::result::isOk(nResult))
    {
        // Let's calculate the new path that would result from the new parent.
        std::string strNewPath;
        if (NULL != poParent)
        {
            strNewPath = poParent->m_strPath;
            if (!strNewPath.empty())
            {
                strNewPath.push_back('.');
            }
        }
        strNewPath.append(m_strName);
        // If the resulting path changed, we need to take action.
        if (strNewPath != m_strPath)
        {
            m_strPath = strNewPath;
            // This properties path changed, we need to make sure our sub properties will be aware of
            // that.
            for (tPropertyList::iterator pIter = m_lstSubproperties.begin(); m_lstSubproperties.end() !=
                pIter && a_util::result::isOk(nResult); pIter++)
            {
                cProperty * poProp = dynamic_cast<cProperty *>(*pIter);
                assert(poProp != NULL);
                nResult = poProp->SetParent(this);
            }
        }
    }
    return nResult;
}

a_util::result::Result cProperty::UnregisterListener (
    IPropertyListener * const poListener)
{
    std::unique_lock<std::recursive_mutex> guard(m_oCritSecListeners);

    a_util::result::Result nResult;
    if (NULL == poListener)
    {
        nResult = ERR_POINTER;
    }
    else
    {
        tPropListenerContainer::iterator itListener = std::remove(m_vecListeners.begin(),
            m_vecListeners.end(), poListener);
        if (m_vecListeners.end() == itListener)
        {
            nResult = ERR_NOT_FOUND;
        }
        else
        {
            m_vecListeners.erase(itListener);
        }
    }
    return nResult;
}

char const * cProperty::GetName () const
{
    return m_strName.c_str();
}

char const * cProperty::GetPath () const
{
    return m_strPath.c_str();
}

a_util::result::Result cProperty::SetName (char const *strName)
{
    a_util::result::Result nResult;
    if (NULL == strName)
    {
        nResult = ERR_POINTER;
    }

    if (a_util::result::isOk(nResult) && a_util::strings::isEqual(strName, "") && !GetSubProperties().empty())
    {
        nResult = ERR_EMPTY;
    }

    if (a_util::result::isOk(nResult))
    {
        m_strName = std::string(strName);
        // Update the path
        std::vector<std::string> vecPath = a_util::strings::split(m_strPath, ".", true);
        if (!vecPath.empty())
        {
            vecPath.erase(vecPath.end() - 1);
        }
        vecPath.push_back(m_strName);

        m_strPath.clear();
        for (size_t idx = 0; idx < vecPath.size(); ++idx)
        {
            if (idx > 0)
            {
                m_strPath.push_back('.');
            }
            m_strPath.append(vecPath[idx]);
        }
    }

    if (a_util::result::isOk(nResult) && !a_util::strings::isEqual(m_strName.c_str(), strName))
    {
        nResult = NotifyListeners(this, &IPropertyListener::ProcessPropertyChange);
    }

    return nResult;
}

a_util::result::Result cProperty::GetValue(const char*& strValue, size_t szIndex) const
{
    if (szIndex < 0 || szIndex >= m_vecValues.size())
    {
        return ERR_INVALID_INDEX;
    }

    if (m_vecValues.front().getType() != a_util::variant::VT_String)
    {
        return ERR_INVALID_TYPE;
    }

    strValue = m_vecValues[szIndex].getString();
    return a_util::result::SUCCESS;
}

a_util::result::Result cProperty::GetValue(bool & bValue, size_t szIndex) const 
{
    if (szIndex < 0 || szIndex >= m_vecValues.size())
    {
        return ERR_INVALID_INDEX;
    }

    if (m_vecValues.front().getType() != a_util::variant::VT_Bool)
    {
        return ERR_INVALID_TYPE;
    }

    bValue = m_vecValues[szIndex];
    return a_util::result::SUCCESS;
}

a_util::result::Result cProperty::GetValue(int32_t & n32Value, size_t szIndex) const 
{
    if (szIndex < 0 || szIndex >= m_vecValues.size())
    {
        return ERR_INVALID_INDEX;
    }

    if (m_vecValues.front().getType() == a_util::variant::VT_Int32)
    {
        n32Value = m_vecValues[szIndex].getInt32();
    }
    else if (m_vecValues.front().getType() == a_util::variant::VT_Double)
    {
        n32Value = static_cast<int32_t>(m_vecValues[szIndex].asFloat());
    }
    else
    {
        return ERR_INVALID_TYPE;
    }

    return a_util::result::SUCCESS;
}

a_util::result::Result cProperty::GetValue(double & f64Value, size_t szIndex) const 
{
    if (szIndex < 0 || szIndex >= m_vecValues.size())
    {
        return ERR_INVALID_INDEX;
    }

    if (m_vecValues.front().getType() == a_util::variant::VT_Double)
    {
        f64Value = m_vecValues[szIndex];
    }
    else if (m_vecValues.front().getType() == a_util::variant::VT_Int32)
    {
        f64Value = static_cast<double>(m_vecValues[szIndex].getInt32());
    }
    else
    {
        return ERR_INVALID_TYPE;
    }

    return a_util::result::SUCCESS;
}

bool cProperty::IsBoolean() const 
{
    return m_vecValues.front().getType() == a_util::variant::VT_Bool;
}

bool cProperty::IsFloat() const 
{
    return m_vecValues.front().getType() == a_util::variant::VT_Double;
}

bool cProperty::IsInteger() const 
{
    return m_vecValues.front().getType() == a_util::variant::VT_Int32;
}

bool cProperty::IsNumeric() const 
{
    return IsInteger() || IsFloat();
}

bool cProperty::IsString() const 
{
    return m_vecValues.front().getType() == a_util::variant::VT_String;
}

bool cProperty::IsArray() const
{
    return m_vecValues.size() > 1;
}

size_t cProperty::GetArraySize() const
{
    return static_cast<size_t>(m_vecValues.size());
}

a_util::result::Result cProperty::AppendValue(char const * strValue)
{
    if (!strValue)
    {
        return ERR_POINTER;
    }
    if (m_vecValues.front().getType() != a_util::variant::VT_String)
    {
        return ERR_INVALID_TYPE;
    }
    m_vecValues.push_back(a_util::variant::Variant(strValue));

    a_util::result::Result nRes;

    // always notify because appending always changes the property
    nRes = NotifyListeners(this, &IPropertyListener::ProcessPropertyChange);

    return nRes;
}

a_util::result::Result cProperty::AppendValue(double fValue)
{
    if (m_vecValues.front().getType() == a_util::variant::VT_Double)
    {
        m_vecValues.push_back(a_util::variant::Variant(fValue));
    }
    else if (m_vecValues.front().getType() == a_util::variant::VT_Int32)
    {
        m_vecValues.push_back(a_util::variant::Variant(static_cast<int32_t>(fValue)));
    }
    else
    {
        return ERR_INVALID_TYPE;
    }

    a_util::result::Result nRes;
    nRes = NotifyListeners(this, &IPropertyListener::ProcessPropertyChange);

    return nRes;
}

a_util::result::Result cProperty::AppendValue(int32_t nValue)
{
    if (m_vecValues.front().getType() == a_util::variant::VT_Int32)
    {
        m_vecValues.push_back(a_util::variant::Variant(nValue));
    }
    else if (m_vecValues.front().getType() == a_util::variant::VT_Double)
    {
        m_vecValues.push_back(a_util::variant::Variant(static_cast<double>(nValue)));
    }
    else
    {
        return ERR_INVALID_TYPE;
    }

    a_util::result::Result nRes;
    nRes = NotifyListeners(this, &IPropertyListener::ProcessPropertyChange);

    return nRes;
}

a_util::result::Result cProperty::AppendValue(bool bValue)
{
    if (m_vecValues.front().getType() != a_util::variant::VT_Bool)
    {
        return ERR_INVALID_TYPE;
    }
    m_vecValues.push_back(a_util::variant::Variant(bValue));

    a_util::result::Result nRes;
    nRes = NotifyListeners(this, &IPropertyListener::ProcessPropertyChange);

    return nRes;
}

char const * cProperty::ToString()
{
    JSONNode oProp;
    JSONNode oNode;
    PropertyToJSON(oNode);
    oProp.push_back(oNode);
    std::string strTmp = libjson::to_std_string(oProp.write_formatted());
    m_strDebugString = strTmp.c_str();
    return m_strDebugString.c_str();
}

a_util::result::Result cProperty::SetValue (char const * strValue)
{
    a_util::result::Result nResult;
    if (NULL == strValue)
    {
        nResult = ERR_POINTER;
    }
    /* Value will be (re-)set and listener(s) will be notified if:
     * - value has changed
     * - datatype has changed
     * - property is an array
     */
    if (a_util::result::isOk(nResult))
    {
        bool bNotifiy = true;
        if (IsString())
        {
            const char * strValCurrent = NULL;
            GetValue(strValCurrent);
            bNotifiy = !a_util::strings::isEqual(strValue, strValCurrent) || IsArray();
        }

        m_vecValues.front().reset(strValue);
        // set truncates the array
        m_vecValues.erase(m_vecValues.begin() + 1, m_vecValues.end());

        if (bNotifiy)
        {
            nResult = NotifyListeners(this, &IPropertyListener::ProcessPropertyChange);
        }
    }

    return nResult;
}

a_util::result::Result cProperty::SetValue(const double f64Value)
{
    a_util::result::Result nResult;
    /* Value will be (re-)set and listener(s) will be notified if:
     * - value has changed
     * - datatype has changed
     * - property is an array
     */

    bool bNotifiy = true;
    if (IsFloat())
    {
        double fValCurrent;
        GetValue(fValCurrent);
        bNotifiy = f64Value != fValCurrent
            || IsArray();
    }
    else if (IsInteger())
    {
        int32_t nValCurrent;
        GetValue(nValCurrent);
        bNotifiy = static_cast<int32_t>(f64Value) != nValCurrent
            || IsArray();
    }

    m_vecValues.front().reset(f64Value);
    // set truncates the array
    m_vecValues.erase(m_vecValues.begin() + 1, m_vecValues.end());

    if (bNotifiy)
    {
        nResult = NotifyListeners(this, &IPropertyListener::ProcessPropertyChange);
    }

    return nResult;
}

a_util::result::Result cProperty::SetValue(const int32_t n32Value)
{
    a_util::result::Result nResult;
    /* Value will be (re-)set and listener(s) will be notified if:
     * - value has changed
     * - datatype has changed
     * - property is an array
     */

    bool bNotifiy = true;
    if (IsInteger())
    {
        int32_t nValCurrent;
        GetValue(nValCurrent);
        bNotifiy = n32Value != nValCurrent || IsArray();
    }
    else if (IsFloat())
    {
        double fValCurrent;
        GetValue(fValCurrent);
        bNotifiy = static_cast<double>(n32Value) != fValCurrent
            || IsArray();
    }

    m_vecValues.front().reset(n32Value);
    // set truncates the array
    m_vecValues.erase(m_vecValues.begin() + 1, m_vecValues.end());

    if (bNotifiy)
    {
        nResult = NotifyListeners(this, &IPropertyListener::ProcessPropertyChange);
    }

    return nResult;
}

a_util::result::Result cProperty::SetValue(const bool bValue)
{
    a_util::result::Result nResult;
    /* Value will be (re-)set and listener(s) will be notified if:
     * - value has changed
     * - datatype has changed
     * - property is an array
     */

    bool bNotifiy = true;
    if (IsBoolean())
    {
        bool bValCurrent;
        GetValue(bValCurrent);
        bNotifiy = bValue != bValCurrent || IsArray();
    }

    m_vecValues.front().reset(bValue);
    // set truncates the array
    m_vecValues.erase(m_vecValues.begin() + 1, m_vecValues.end());

    if (bNotifiy)
    {
        nResult = NotifyListeners(this, &IPropertyListener::ProcessPropertyChange);
    }

    return nResult;
}

a_util::result::Result cProperty::NotifyListeners(cProperty * poProperty, tPropertyCallback pCallback) const
{
    std::unique_lock<std::recursive_mutex> guard(
                const_cast<cProperty*>(this)->m_oCritSecListeners);

    a_util::result::Result nResult;
    std::list<a_util::result::Result> lstResults;

    // notify all listeners and collect results in result list
    for (tPropListenerContainer::const_iterator itListener = m_vecListeners.begin();
        itListener != m_vecListeners.end(); ++itListener)
    {
        a_util::result::Result nRes;
        std::string strFullPath = poProperty->GetPath();
        std::string strRelPath = strFullPath.erase(0, m_strPath.size());
        if (strRelPath.empty())
        {
            nRes = ((*itListener)->*pCallback)(this, poProperty, m_strName.c_str());
        }
        else
        {
            // need to check if relative path starts with a dot and delete it
            if (0 == strRelPath.find_first_of(".", 0))
            {
                strRelPath.erase(0, 1);
            }
            nRes = ((*itListener)->*pCallback)(this, poProperty, strRelPath.c_str());
        }
        lstResults.push_back(nRes);
    }

    // find erroneous results inside list
    for (std::list<a_util::result::Result>::const_iterator itFailed = lstResults.begin();
        itFailed != lstResults.end(); ++itFailed)
    {
        if (a_util::result::isFailed((*itFailed)))
        {
            // return first failed result
            nResult = *itFailed;
            break;
        }
    }


    // WARNING: The following dynamic cast is NOT safe as soon as cProperty instances
    // are being shared among multiple, different compilations (binary compatibility is not ensured).
    // As of this stage, Properties are only shared through the BUS and are being re-created
    // from JSON statements within each individual instance.

    cProperty* pParent = dynamic_cast<cProperty*>(m_poParent);
    if (pParent)
    {
        // also notify all parent and grandparent properties (e.g. this)
        a_util::result::Result nParentResult =
                pParent->NotifyListeners(poProperty, pCallback);

        if (a_util::result::isOk(nResult))
        {
            nResult = nParentResult;
        }
    }

    return nResult;
}

IProperty::tPropertyList::iterator cProperty::FindSubproperty(const char * strName)
{
    IProperty::tPropertyList::iterator pPropertyIter;
    for (pPropertyIter = m_lstSubproperties.begin();
        pPropertyIter != m_lstSubproperties.end(); ++pPropertyIter)
    {
        if (a_util::strings::isEqual((*pPropertyIter)->GetName(), strName))
        {
            break;
        }
    }

    return pPropertyIter;
}
