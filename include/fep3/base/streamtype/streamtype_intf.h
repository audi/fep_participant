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
#ifndef __FEP_COMP_STREAMTYPE_INTF_H
#define __FEP_COMP_STREAMTYPE_INTF_H

#include <string>
#include "../properties/properties_intf.h"

namespace fep
{
    /**
     * Definition of a streamtype interface.
     * The streamtype is a composition of properties
     * (name value pairs) to describe a stream or data sample content.
     * 
     * @see page_stream_type
     */
    class IStreamType : public IProperties
    {
        protected:
            /** 
             * protected DTOR.
             */
            virtual ~IStreamType() = default;

        public:
            /** 
             * returns the streammetatypes name.
             * @see fep::StreamMetaType
             * @return pointer to the streammetatype name
             */
            virtual const char* getMetaTypeName() const = 0;
    };
}
/**
 * bool operator to compare streamtypes.
 * streamtypes are equal if the name of the stream metatypes and the set properties are equal.
 */
inline bool operator==(const fep::IStreamType& left, const fep::IStreamType& right)
{
    if (std::string(left.getMetaTypeName()) != right.getMetaTypeName())
    {
        return false;
    }
    else
    {
        return left.isEqual(right);
    }
}

#endif //__FEP_COMP_STREAMTYPE_INTF_H
