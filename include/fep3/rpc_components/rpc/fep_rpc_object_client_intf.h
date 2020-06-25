/**
 *
 * RPC Protocol Object Registry declaration.
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
#include <string>
#include "fep_rpc_definition.h"

#ifndef FEP_RPC_OBJECT_CLIENT_INTF_INCLUDED
#define FEP_RPC_OBJECT_CLIENT_INTF_INCLUDED

namespace fep
{

/**
 * @brief retrieve rpc interface identifier information from type \p T
 * 
 * @tparam T the type that must define an interface identifier via @ref FEP_RPC_IID
 * @return std::string the rpc identifier of the type
 */
template<class T>
std::string getRPCIID()
{
    return T::getRPCIID();
}

/**
 * @brief retrieves the default name of this RPC object 
 * this will be the default name it is available within the RPC Object Registry (see @ref IRPCObjectServerRegistry)
 * 
 * @tparam T  the type that must define an interface identifier via @ref FEP_RPC_IID 
 * @return std::string 
 */
template<class T>
std::string getRPCDefaultName()
{
    return T::getRPCDefaultName();
}

/// Interface of a RPC Client
class IRPCObjectClient
{
    public:
        /**
        * DTOR
        */
        virtual ~IRPCObjectClient() {};

    public:
        /**
         * @retval The ID of the bound rpc server
         */
        virtual std::string getRPCObjectIID() const = 0;

        /**
         * @retval The ID of the bound rpc server
         */
        virtual std::string getRPCObjectDefaultName() const = 0;
};


}//ns fep

#endif //FEP_RPC_OBJECT_CLIENT_INTF_INCLUDED
