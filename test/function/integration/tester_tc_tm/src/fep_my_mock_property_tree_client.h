/**
 * Implementation of adapted property tree mockup used by this test
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

#ifndef _FEP_TEST_MY_MOCK_PROPERTY_TREE_CLIENT_H_INC_
#define _FEP_TEST_MY_MOCK_PROPERTY_TREE_CLIENT_H_INC_

#include <a_util/strings/strings_functions.h>

#include "function/_common/fep_mock_property_tree.h"
#include "fep3/components/legacy/property_tree/property.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/fep_module_header_config.h"

using namespace fep;

class cMyMockPropertyTreeClient : public cMockPropertyTree
{
public:
    cMyMockPropertyTreeClient()
        : m_oStrModuleNameProperty(fep::g_strElementHeaderPath_strElementName, "UnnamedModule")
        , m_oStrMasterNameProperty(FEP_TIMING_MASTER_PARTICIPANT, "")
    {
    }

public:
    void SetModuleName(const char* strValue)
    {
        m_oStrModuleNameProperty.SetValue(strValue);
    }

    const char * GetModuleName() const
    {
        const char* strValue = NULL;

        m_oStrModuleNameProperty.GetValue(strValue);

        return strValue;
    }

public:
    void SetMasterName(const char* strValue)
    {
        m_oStrMasterNameProperty.SetValue(strValue);
    }

    const char * GetMasterName() const
    {
        const char* strValue = NULL;

        m_oStrMasterNameProperty.GetValue(strValue);

        return strValue;
    }

public:
    virtual fep::IProperty const * GetProperty(char const * strPropPath) const
    {
        if (a_util::strings::isEqual(fep::g_strElementHeaderPath_strElementName, strPropPath))
        {
            return &m_oStrModuleNameProperty;
        }
        if (a_util::strings::isEqual(FEP_TIMING_MASTER_PARTICIPANT, strPropPath))
        {
            return &m_oStrMasterNameProperty;
        }

        return NULL;
    }

    virtual fep::IProperty * GetProperty(char const * strPropPath)
    {
        if (a_util::strings::isEqual(fep::g_strElementHeaderPath_strElementName, strPropPath))
        {
            return &m_oStrModuleNameProperty;
        } 
        if (a_util::strings::isEqual(FEP_TIMING_MASTER_PARTICIPANT, strPropPath))
        {
            return &m_oStrMasterNameProperty;
        }

        return NULL;
    }

    virtual fep::Result GetPropertyValue(const char * strPropPath, const char *& strValue) const
    {
        const fep::IProperty* pProperty = GetProperty(strPropPath);

        if (pProperty && pProperty->IsString())
        {
            return pProperty->GetValue(strValue);
        }

        return ERR_PATH_NOT_FOUND;
    }

    virtual fep::Result GetPropertyValue(const char * strPropPath, double & fValue) const
    {
        const fep::IProperty* pProperty = GetProperty(strPropPath);

        if (pProperty && pProperty->IsFloat())
        {
            return pProperty->GetValue(fValue);
        }

        return ERR_PATH_NOT_FOUND;
    }

    virtual fep::IProperty const * GetLocalProperty(char const * strPropPath) const
    {
        return GetProperty(strPropPath);
    }

    virtual fep::IProperty * GetLocalProperty(char const * strPropPath)
    {
        return GetProperty(strPropPath);
    }

public:
    fep::cProperty m_oStrModuleNameProperty;
    fep::cProperty m_oStrMasterNameProperty;
};

#endif // _FEP_TEST_MY_MOCK_PROPERTY_TREE_CLIENT_H_INC_
