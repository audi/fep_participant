/**
* Declaration of the Class IRPCDataRegistry. (can be reached from over rpc)
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

#ifndef __FEP_RPC_DATA_REGISTRY_INTF_DEF_H
#define __FEP_RPC_DATA_REGISTRY_INTF_DEF_H

#include <vector>
#include <string>
//very important to have this relative! system library!
#include "../rpc/fep_rpc_definition.h"

namespace fep
{
namespace rpc
{
    /**
     * @brief definition of the external service interface of the data registry
     * @see data_registry.json file
     */
    class IRPCDataRegistryDef
    {
        protected:
            /**
             * @brief Destroy the IRPCDataRegistry object
             * 
             */
            virtual ~IRPCDataRegistryDef() = default;

        public:
            ///definiton of the FEP rpc service iid as clock service
            FEP_RPC_IID("data_registry.iid", "data_registry");

    };
}
}

#endif // __FEP_RPC_PARTICIPANT_INFO_INTF_H
