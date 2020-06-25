/**
* Declaration of the Class IRPCStateMachine. (can be reached from over rpc)
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

#ifndef __FEP_RPC_PARTICIPANT_STATE_MACHINE_INTF_DEF_H
#define __FEP_RPC_PARTICIPANT_STATE_MACHINE_INTF_DEF_H

#include <vector>
#include <string>
//very important to have this relative! system library!
#include "../../rpc/fep_rpc_definition.h"
#include "../../../base/states/fep2_state.h"

namespace fep
{
namespace rpc
{
    /**
     * @brief definition of the external service interface state machine content
     * @see delivered state_machine.json file
     */
    class IRPCStateMachineDef
    {
        protected:
            /**
             * @brief Destroy the IRPCStateMachine 
             * 
             */
            virtual ~IRPCStateMachineDef() = default;

        public:
            ///definiton of the FEP rpc service iid as state machin
            FEP_RPC_IID("state_machine_fep2.iid", "state_machine_fep2");
            /// wrap up and redeclare the participant states 
            ///@see @ref fep_state_machine
            using State = fep::tState;
    };
}
}

#endif // __FEP_RPC_PARTICIPANT_STATE_MACHINE_INTF_H