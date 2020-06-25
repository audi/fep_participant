/**
 *
 * RPC Protocol implementation.
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

#include <vector>
#include <a_util/strings/strings_functions.h>
#include "fep3/components/rpc/fep_element_object_client.h"
#include "fep3/components/rpc/rpc_stubs_element_object_server.h"
#include "fep_rpc_element_object.h"

namespace fep
{

cRPCElementObjectServer::cRPCElementObjectServer(rpc::IRPCElementInfo& oElemInfo) : m_oElementInfo(oElemInfo)
{
}


std::string cRPCElementObjectServer::GetObjects()
{
    std::vector<std::string> lstRes = m_oElementInfo.GetObjects();
    return a_util::strings::join(lstRes, ", ");
}

std::string cRPCElementObjectServer::GetRPCIIDsForObject(const std::string& strObject)
{
    std::vector<std::string> lstRes = m_oElementInfo.GetRPCIIDsForObject(strObject);
    return a_util::strings::join(lstRes, ", ");
}

std::string cRPCElementObjectServer::GetRPCInterfaceDefinition(const std::string& strIID, const std::string& strObject)
{
    return m_oElementInfo.GetRPCInterfaceDefinition(strObject, strIID);
}


}//ns fep
