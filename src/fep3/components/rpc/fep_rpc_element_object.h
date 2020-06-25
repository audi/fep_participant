/**
 *
 * RPC Protocol declaration.
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

#ifndef FEP_RPC_ELEMENT_OBJECT_H_IMPL_INCLUDED
#define FEP_RPC_ELEMENT_OBJECT_H_IMPL_INCLUDED

#include <string>
#include <rpc_pkg/rpc_server.h>

#include "fep_result_decl.h"
#include "fep3/components/rpc/fep_element_object_client.h"
#include "fep3/components/rpc/rpc_stubs_element_object_server.h"
#include "fep3/components/rpc/fep_rpc_stubs.h"

namespace fep
{

class cRPCElementObjectServer : public rpc_object_server<rpc_stubs::cRPCElementObjectServer, rpc::IRPCElementInfo>
{
    rpc::IRPCElementInfo& m_oElementInfo;
    
    public:
        cRPCElementObjectServer(rpc::IRPCElementInfo& oElemInfo);
        std::string GetObjects() override;
        std::string GetRPCIIDsForObject(const std::string& strObject) override;
        std::string GetRPCInterfaceDefinition(const std::string& strIID, const std::string& strObject) override;

};

}//ns fep

#endif //FEP_RPC_ELEMENT_OBJECT_H_IMPL_INCLUDED
