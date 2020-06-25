/**
 * Implementation of propertytree mockup used by FEP functional test cases!
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

#ifndef _FEP_TEST_MOCK_PROPERTYTREE_H_INC_
#define _FEP_TEST_MOCK_PROPERTYTREE_H_INC_
#include <fep3/components/legacy/property_tree/fep_propertytree.h>

class cMockPropertyTree : public fep::IPropertyTree, public fep::IPropertyTreePrivate
{
public:
    cMockPropertyTree() { }
    virtual ~cMockPropertyTree () { }
    virtual fep::IProperty const * GetProperty(char const * strPropPath) const
    {
        return NULL;
    }

    virtual fep::IProperty * GetProperty(char const * strPropPath)
    {
        return NULL;
    }

    virtual fep::Result SetPropertyValue(char const * strPropPath, char const * strValue)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetPropertyValue(char const * strPropPath, double const f64Value)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetPropertyValue(char const * strPropPath, int32_t const n32Value)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetPropertyValue(char const * strPropPath,bool const bValue)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result GetPropertyValue(const char * strPropPath,
        const char *& strValue) const
    {
        return ERR_NOERROR;
    }

    virtual fep::Result GetPropertyValue(const char * strPropPath,
        double & fValue) const
    {
        return ERR_NOERROR;
    }

    virtual fep::Result GetPropertyValue(const char * strPropPath,
        int32_t & nValue) const
    {
        return ERR_NOERROR;
    }

    virtual fep::Result GetPropertyValue(const char * strPropPath,
        bool & bValue) const
    {
        return ERR_NOERROR;
    }

    virtual fep::Result RegisterListener(char const * strPropertyPath,
        fep::IPropertyListener* const poListener)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result UnregisterListener(char const * strPropertyPath,
        fep::IPropertyListener* const poListener)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result ClearProperty(char const * strPropertyPath)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result DeleteProperty(char const * strPropertyPath)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetPropertyValues(char const * strPropPath,
        char const *const * strFirstValue, size_t szArraySize)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetPropertyValues(char const * strPropPath,
        double const * f64FirstValue, size_t szArraySize)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetPropertyValues(char const * strPropPath,
        int32_t const * n32FirstValue, size_t szArraySize)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetPropertyValues(char const * strPropPath,
        bool const * bFirstValue, size_t szArraySize)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetRemotePropertyValue(
        char const * strElementName, char const * strPropPath, 
        char const * strValue, const timestamp_t tmTimeout = 0)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetRemotePropertyValue(
        char const * strElementName, char const * strPropPath, 
        double const f64Value, const timestamp_t tmTimeout = 0)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetRemotePropertyValue(
        char const * strElementName, char const * strPropPath, 
        int32_t const n32Value, const timestamp_t tmTimeout = 0)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetRemotePropertyValue(
        char const * strElementName, char const * strPropPath, 
        bool const bValue, const timestamp_t tmTimeout = 0)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetRemotePropertyValues(
        char const * strElementName, char const * strPropPath,
        char const *const * strFirstValue, size_t szArraySize, const timestamp_t tmTimeout = 0)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetRemotePropertyValues(
        char const * strElementName, char const * strPropPath,
        double const * f64FirstValue, size_t szArraySize, const timestamp_t tmTimeout = 0)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetRemotePropertyValues(
        char const * strElementName, char const * strPropPath,
        int32_t const * n32FirstValue, size_t szArraySize, const timestamp_t tmTimeout = 0)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetRemotePropertyValues(
        char const * strElementName, char const * strPropPath,
        bool const * bFirstValue, size_t szArraySize, const timestamp_t tmTimeout = 0)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result DeleteRemoteProperty(
        char const * strElementName, char const * strPropPath)
    {
        return ERR_NOERROR;
    }

    virtual fep::IProperty const * GetLocalProperty(char const * strPropPath) const
    {
        return NULL;
    }

    virtual fep::IProperty * GetLocalProperty(char const * strPropPath)
    {
        return NULL;
    }

    virtual fep::Result GetRemoteProperty(char const * strElementName, char const * strPropPath,
        fep::IProperty ** pProperty, timestamp_t const tmTimeout)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result MirrorRemoteProperty(char const * strElementName, char const * strRemotePath,
        char const * strLocalPath, timestamp_t const tmTimeout)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result UnmirrorRemoteProperty(char const * strElementName, char const * strRemotePath,
        char const * strLocalPath, timestamp_t const tmTimeout)
    {
        return ERR_NOERROR;
    }

};

#endif // _FEP_TEST_MOCK_PROPERTYTREE_H_INC_
