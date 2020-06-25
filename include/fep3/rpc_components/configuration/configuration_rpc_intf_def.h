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

#ifndef __FEP_RPC_CONFIGURATION_DEFINITON_INTF_DEF_H
#define __FEP_RPC_CONFIGURATION_DEFINITON_INTF_DEF_H

#include <vector>
#include <string>
//very important to have this relative! system library!
#include "../rpc/fep_rpc_definition.h"

namespace fep
{
namespace rpc
{
    /**
     * @brief Defition of the RPC Interface definion of the external service interface of the configuration
     * @see configuration.json file
     */
    class IRPCConfigurationDef
    {
        protected:
            /**
             * @brief Destroy the IRPCConfigurationDef object
             * 
             */
            virtual ~IRPCConfigurationDef() = default;

        public:
            ///definiton of the FEP rpc service iid for the configuration interface
            FEP_RPC_IID("configuration.iid", "configuration");
    };
}
}

#endif // __FEP_RPC_CONFIGURATION_DEFINITON_INTF_H
