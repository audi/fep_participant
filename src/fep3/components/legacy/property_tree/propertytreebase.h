/**
 * Declaration of the Class cPropertyTreeBase.
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

#include <string>
#include <cstdint>

#include "fep3/components/legacy/property_tree/propertytreebase_intf.h"
#include "fep_participant_export.h"
#include "fep_result_decl.h"

namespace fep
{
    class IProperty;
    class IPropertyListener;
    class cProperty;

    /**
     * This class is the implementation of the property tree.
     */
    class FEP_PARTICIPANT_EXPORT cPropertyTreeBase : public virtual IPropertyTreeBase
    {
    public:
        /**
         * CTOR
         */
        cPropertyTreeBase();

        /**
         * DTOR
         */
        virtual ~cPropertyTreeBase();

        /**
        * \c MergeProperty merges an existing property into the local tree.
        * Already existing (sub)properties will be updated, nonexisting ones
        * will be created. Existing local subproperties that are not present in
        * the merged property are not affected and remain untouched.
        * @param [in] strPath Path of the current local property.
        * @param [in] poProperty The property to be merged.
        * @return Returns the inserted property.
        * @retval NULL on any error.
        */
        IProperty * MergeProperty(std::string strPath, const IProperty * poProperty);

    public: // implements IPropertyTreeBase
        IProperty const * GetProperty(char const * strPropPath) const;
        IProperty * GetProperty(char const * strPropPath);
        a_util::result::Result SetPropertyValue(char const * strPropPath, char const * strValue);
        a_util::result::Result SetPropertyValue(char const * strPropPath, double const f64Value);
        a_util::result::Result SetPropertyValue(char const * strPropPath, int32_t const n32Value);
        a_util::result::Result SetPropertyValue(char const * strPropPath, bool const bValue);
        a_util::result::Result GetPropertyValue(const char * strPropPath, const char *& strValue) const;
        a_util::result::Result GetPropertyValue(const char * strPropPath, double & fValue) const;
        a_util::result::Result GetPropertyValue(const char * strPropPath, int32_t & nValue) const;
        a_util::result::Result GetPropertyValue(const char * strPropPath, bool & bValue) const;
        a_util::result::Result RegisterListener(char const * strPropertyPath,
            IPropertyListener* const poListener);
        a_util::result::Result UnregisterListener(char const * strPropertyPath,
            IPropertyListener* const poListener);
        a_util::result::Result ClearProperty(char const * strPropertyPath);
        a_util::result::Result DeleteProperty(char const * strPropertyPath);

    private:
        /// The root element of the property tree.
        cProperty * m_poPropertyRoot;
    };
}
