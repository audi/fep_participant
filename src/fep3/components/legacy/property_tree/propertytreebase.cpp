/**
 * Implementation of the Class cPropertyTreeBase.
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

#include "fep3/components/legacy/property_tree/propertytreebase.h"
#include <cstddef>
#include <list>
#include <string>
#include <a_util/result/result_type.h>

#include "fep3/components/legacy/property_tree/property.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep_errors.h"

namespace fep
{
class IPropertyListener;

//_MAKE_RESULT(-4, ERR_POINTER);
//_MAKE_RESULT(-24, ERR_PATH_NOT_FOUND);
//_MAKE_RESULT(-38, ERR_FAILED);

cPropertyTreeBase::cPropertyTreeBase() : m_poPropertyRoot(NULL)
{
    m_poPropertyRoot = new cProperty("", "");
}

cPropertyTreeBase::~cPropertyTreeBase ()
{
    // delete local properties
    delete m_poPropertyRoot;
    m_poPropertyRoot = NULL;
}

IProperty * cPropertyTreeBase::MergeProperty(std::string strPath,
    const IProperty * poProperty)
{
    if (!poProperty)
    {
        return NULL;
    }

    if (!strPath.empty())
    {
        strPath.push_back('.');
    }
    strPath.append(poProperty->GetName());

    IProperty * poLocal = NULL;
    if (poProperty->IsString())
    {
        const char * strVal;
        poProperty->GetValue(strVal);
        poLocal = m_poPropertyRoot->GetOrMakeSubproperty(strPath.c_str(), true, strVal);
    }
    else if (poProperty->IsFloat())
    {
        double fVal;
        poProperty->GetValue(fVal);
        poLocal = m_poPropertyRoot->GetOrMakeSubproperty(strPath.c_str(), true, fVal);
    }
    else if (poProperty->IsInteger())
    {
        int32_t nVal;
        poProperty->GetValue(nVal);
        poLocal = m_poPropertyRoot->GetOrMakeSubproperty(strPath.c_str(), true, nVal);
    }
    else if (poProperty->IsBoolean())
    {
        bool bVal;
        poProperty->GetValue(bVal);
        poLocal = m_poPropertyRoot->GetOrMakeSubproperty(strPath.c_str(), true, bVal);
    }
    // if the property was just created this setvalue won't change anything
    cProperty::CopyPropertyValue(poProperty, poLocal);

    const IProperty::tPropertyList & lstSubs = poProperty->GetSubProperties();
    for (IProperty::tPropertyList::const_iterator itSubs = lstSubs.begin();
        itSubs != lstSubs.end(); ++itSubs)
    {
        if (!MergeProperty(strPath, *itSubs))
        {
            return NULL;
        }
    }

    return poLocal;
}

IProperty * cPropertyTreeBase::GetProperty(char const * strPropPath)
{
    return m_poPropertyRoot->GetSubproperty(strPropPath);
}

IProperty const * cPropertyTreeBase::GetProperty(char const * strPropPath) const
{
    return m_poPropertyRoot->GetSubproperty(strPropPath);
}

a_util::result::Result cPropertyTreeBase::SetPropertyValue(char const * strPropPath,
    char const * strValue)
{
    // prevent property creation (SetValue would fail anyway)
    if (!strValue)
    {
        return ERR_POINTER;
    }

    IProperty * pProperty =
        m_poPropertyRoot->GetOrMakeSubproperty(strPropPath, true, strValue);
    // in case the property already exists, we need to set the value here instead of in
    // the getsubproperty call. if it was created, this call is a no-op anyway.
    return pProperty->SetValue(strValue);
}

a_util::result::Result cPropertyTreeBase::SetPropertyValue(char const * strPropPath, double const f64Value)
{
    IProperty * pProperty =
        m_poPropertyRoot->GetOrMakeSubproperty(strPropPath, true, f64Value);
    return pProperty->SetValue(f64Value);
}

a_util::result::Result cPropertyTreeBase::SetPropertyValue(char const * strPropPath, int32_t const n32Value)
{
    IProperty * pProperty =
        m_poPropertyRoot->GetOrMakeSubproperty(strPropPath, true, n32Value);
    return pProperty->SetValue(n32Value);
}

a_util::result::Result cPropertyTreeBase::SetPropertyValue(char const * strPropPath, bool const bValue)
{
    IProperty * pProperty =
        m_poPropertyRoot->GetOrMakeSubproperty(strPropPath, true, bValue);
    return pProperty->SetValue(bValue);
}

template <typename T>
a_util::result::Result GetPropertyValueTemplate(char const * strPropPath,
    T &value, cPropertyTreeBase const * const poTree)
{
    if (!strPropPath)
    {
        return ERR_POINTER;
    }
    
    a_util::result::Result nResult;
    IProperty const * poProperty = NULL;
    poProperty = poTree->GetProperty(strPropPath);
    if (!poProperty)
    {
        nResult = ERR_PATH_NOT_FOUND;
    }
    else
    {
        nResult = poProperty->GetValue(value);
    }
    return nResult;
}


a_util::result::Result cPropertyTreeBase::GetPropertyValue(const char * strPropPath,
    const char *& strValue) const
{
    return GetPropertyValueTemplate(strPropPath, strValue, this);
}

a_util::result::Result cPropertyTreeBase::GetPropertyValue(const char * strPropPath,
    double & fValue) const
{
    return GetPropertyValueTemplate(strPropPath, fValue, this);
}

a_util::result::Result cPropertyTreeBase::GetPropertyValue(const char * strPropPath,
    int32_t & nValue) const
{
    return GetPropertyValueTemplate(strPropPath, nValue, this);
}

a_util::result::Result cPropertyTreeBase::GetPropertyValue(const char * strPropPath,
    bool & bValue) const
{
    return GetPropertyValueTemplate(strPropPath, bValue, this);
}

a_util::result::Result cPropertyTreeBase::RegisterListener(char const * strPropertyPath,
    IPropertyListener* const poListener)
{
    a_util::result::Result nResult;
    IProperty * pProperty = GetProperty(strPropertyPath);
    nResult = NULL != pProperty ? nResult : ERR_FAILED;
    if (a_util::result::isOk(nResult))
    {
        nResult = pProperty->RegisterListener(poListener);
    }
    return nResult;
}

a_util::result::Result cPropertyTreeBase::UnregisterListener(char const * strPropertyPath,
    IPropertyListener* const poListener)
{
    a_util::result::Result nResult;
    IProperty * pProperty = GetProperty(strPropertyPath);
    nResult = NULL != pProperty ? nResult : ERR_FAILED;
    if (a_util::result::isOk(nResult))
    {
        nResult = pProperty->UnregisterListener(poListener);
    }
    return nResult;
}

a_util::result::Result cPropertyTreeBase::ClearProperty(char const * strPropertyPath)
{
    a_util::result::Result nResult;
    IProperty * pProperty = GetProperty(strPropertyPath);
    nResult = NULL != pProperty ? nResult : ERR_PATH_NOT_FOUND;
    if (a_util::result::isOk(nResult))
    {
        // array truncation is done implicitly in SetValue
        if (pProperty->IsString())
        {
            nResult = pProperty->SetValue("");
        }
        else if (pProperty->IsBoolean())
        {
            nResult = pProperty->SetValue(false);
        }
        else if (pProperty->IsFloat())
        {
            nResult = pProperty->SetValue(0.0);
        }
        else if (pProperty->IsInteger())
        {
            nResult = pProperty->SetValue((int32_t)0);
        }

        // Clear the sub properties.
        while (!pProperty->GetSubProperties().empty())
        {
            IProperty const * pSubProperty = *(pProperty->GetSubProperties().begin());
            pProperty->DeleteSubproperty(pSubProperty->GetName());
        }
    }
    return nResult;
}

a_util::result::Result cPropertyTreeBase::DeleteProperty(char const * strPropertyPath)
{
    a_util::result::Result nResult;
    IProperty * pProperty = NULL;
    IProperty * pParentProperty = NULL;

    if (a_util::result::isOk(nResult))
    {
        pProperty = GetProperty(strPropertyPath);
        if (pProperty == m_poPropertyRoot)
        {
            nResult = ERR_FAILED;
        }
        else
        {
            nResult = NULL != pProperty ? nResult : ERR_PATH_NOT_FOUND;
        }
    }
    if (a_util::result::isOk(nResult))
    {
        // Get the parent property
        pParentProperty = pProperty->GetParent();
        nResult = NULL != pParentProperty ? nResult : ERR_PATH_NOT_FOUND;
    }
    if (a_util::result::isOk(nResult))
    {
        nResult = pParentProperty->DeleteSubproperty(pProperty->GetName());
    }
    return nResult;
}

} // ns fep
