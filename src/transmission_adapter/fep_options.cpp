/**

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
 */
/// Someone should add a header here some time

#include <utility>
#include "transmission_adapter/fep_options_verifier_intf.h"
#include "transmission_adapter/fep_options.h"

using namespace fep;

cOptions::cOptions() :
m_pOptionsVerifier(NULL)
{
}

cOptions::~cOptions()
{
}

void cOptions::SetOptionsVerifier(IOptionsVerifier * poVerifier)
{
    if(NULL != m_pOptionsVerifier)
    {
        m_mapBoolOptions.clear();
        m_mapIntOptions.clear();
        m_mapFloatOptions.clear();
        m_mapStringOptions.clear();
    }

    m_pOptionsVerifier = poVerifier;
}

bool cOptions::GetOption(const std::string& strOptionName, bool &bValue) const
{
    if (NULL != m_pOptionsVerifier && m_pOptionsVerifier->CheckOption(strOptionName, bValue))
    {
        tBoolOptions::const_iterator it = m_mapBoolOptions.find(strOptionName);
        if(it != m_mapBoolOptions.end())
        {
            bValue = (it->second);
            return true;
        }
    }
    return false;
}


bool cOptions::GetOption(const std::string& strOptionName, int &dValue) const
{
    if (NULL != m_pOptionsVerifier && m_pOptionsVerifier->CheckOption(strOptionName, dValue))
    {
        tIntOptions::const_iterator it = m_mapIntOptions.find(strOptionName);
        if(it != m_mapIntOptions.end())
        {
            dValue = (it->second);
            return true;
        }
    }

    return false;
}

bool cOptions::GetOption(const std::string& strOptionName, size_t &szValue) const
{
    if (NULL != m_pOptionsVerifier && m_pOptionsVerifier->CheckOption(strOptionName, szValue))
    {
        tSizeOptions::const_iterator it = m_mapSizeOptions.find(strOptionName);
        if(it != m_mapSizeOptions.end())
        {
            szValue = (it->second);
            return true;
        }
    }
    return false;
}

bool cOptions::GetOption(const std::string& strOptionName, float &fValue) const
{
    if (NULL != m_pOptionsVerifier && m_pOptionsVerifier->CheckOption(strOptionName, fValue))
    {
       tFloatOptions::const_iterator it = m_mapFloatOptions.find(strOptionName);
        if( it != m_mapFloatOptions.end())
        {
            fValue = (it->second);
            return true;
        }
    }

    return false;
}

bool cOptions::GetOption(const std::string& strOptionName, std::string &strValue) const
{
    if (NULL != m_pOptionsVerifier && m_pOptionsVerifier->CheckOption(strOptionName, strValue))
    {
        tStringOptions::const_iterator it = m_mapStringOptions.find(strOptionName);
        if(it != m_mapStringOptions.end())
        {
            strValue = (it->second);
            return true;
        }
    }

    return false;
}

bool cOptions::SetOption(const std::string& strOptionName, const bool&bValue)
{
    if (NULL != m_pOptionsVerifier && m_pOptionsVerifier->CheckOption(strOptionName, bValue))
    {
        tBoolOptions::iterator it = m_mapBoolOptions.find(strOptionName);
        if(it == m_mapBoolOptions.end())
        {
            m_mapBoolOptions.insert(std::make_pair(strOptionName, bValue));
        }
        else
        {
            it->second = bValue;
        }

        return true;
    }

    return false;
}

bool cOptions::SetOption(const std::string& strOptionName, const int &dValue)
{
    if (NULL != m_pOptionsVerifier && m_pOptionsVerifier->CheckOption(strOptionName, dValue))
    {
        tIntOptions::iterator it = m_mapIntOptions.find(strOptionName);
        if( it == m_mapIntOptions.end())
        {
            m_mapIntOptions.insert(std::make_pair(strOptionName, dValue));
        }
        else
        {
            it->second = dValue;
        }
        return true;
    }

    return false;
}

bool cOptions::SetOption(const std::string& strOptionName, const size_t &szValue)
{
    if (NULL != m_pOptionsVerifier && m_pOptionsVerifier->CheckOption(strOptionName, szValue))
    {
        tSizeOptions::iterator it = m_mapSizeOptions.find(strOptionName);
        if( it == m_mapSizeOptions.end())
        {
            m_mapSizeOptions.insert(std::make_pair(strOptionName, szValue));
        }
        else
        {
            it->second = szValue;
        }
        return true;
    }

    return false;
}

bool cOptions::SetOption(const std::string& strOptionName, const float &fValue)
{
    if (NULL != m_pOptionsVerifier && m_pOptionsVerifier->CheckOption(strOptionName, fValue))
    {
        tFloatOptions::iterator it  = m_mapFloatOptions.find(strOptionName);
        if( it == m_mapFloatOptions.end())
        {
            m_mapFloatOptions.insert(std::make_pair(strOptionName, fValue));
        }
        else
        {
            it->second = fValue;
        }
        return true;
    }

    return false;
}

bool cOptions::SetOption(const std::string& strOptionName, const std::string &strValue)
{
    if (NULL != m_pOptionsVerifier && m_pOptionsVerifier->CheckOption(strOptionName, strValue))
    {
        tStringOptions::iterator it = m_mapStringOptions.find(strOptionName);
        if(it == m_mapStringOptions.end())
        {
            m_mapStringOptions.insert(std::make_pair(strOptionName, strValue));
        }
        else
        {
            it->second = strValue;
        }
        return true;
    }

    return false;
}

bool cOptions::SetOption(const std::string& strOptionName, const char* strValue)
{
    std::string strTmpStrValue = strValue;
    return SetOption(strOptionName, strTmpStrValue);
}
