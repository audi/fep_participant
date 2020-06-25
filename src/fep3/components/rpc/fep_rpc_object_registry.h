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

#ifndef FEP_OBJECT_REGISTRY_H_INCLUDED
#define FEP_OBJECT_REGISTRY_H_INCLUDED

#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <rpc_pkg/rpc_object_registry.h>
#include <rpc_pkg/rpc_server.h>

#include "fep_result_decl.h"
#include "fep3/components/rpc/fep_element_object_client.h"
#include "fep3/components/rpc/fep_rpc_intf.h"

namespace fep
{
namespace detail
{

class cFEPObjectServerWrapper : public ::rpc::IRPCObject
{
    public:
        cFEPObjectServerWrapper(fep::IRPCObjectServer& pObjectServer);
        virtual ~cFEPObjectServerWrapper();

        ::fep::Result HandleCall(const char* strRequest,
            size_t nRequestSize,
            ::rpc::IResponse& oResponse);

        fep::IRPCObjectServer& GetObjectServer() const;

    private:
        fep::IRPCObjectServer& m_oObjectServer;
};
} //detail

/**
 * An RPC Server that receives calls via FEP.
 */
class cRPCObjectRegistry : public ::rpc::cRPCObjectsRegistry,
                           public fep::IRPCObjectServerRegistry,
                           public rpc::IRPCElementInfo
{
    
    public:
        /**
         * Constructor.
         * @param[in] strContentType The content type that should be set for the responses.
         */
        cRPCObjectRegistry();

        /**
         * Destructor.
         */
        ~cRPCObjectRegistry();

        fep::Result ProcessRequest(const std::string& strObjectName,
                                   const std::string& strRequest,
                                   std::string& strResponse);

        fep::Result RegisterObjectServer(const char* strName,
                                         IRPCObjectServer& oRPCObjectServerInstance) override;
        fep::Result UnregisterObjectServer(const char* strName) override;
     public:
         std::vector<std::string> GetObjects() const override;
         std::vector<std::string> GetRPCIIDsForObject(const std::string& strObject) const override;
         std::string GetRPCInterfaceDefinition(const std::string& strObject, const std::string& strIID) const override;
     private:
         std::map<std::string, std::unique_ptr<detail::cFEPObjectServerWrapper>> m_mapObjects;
         mutable std::mutex m_csMapLock; //only for registration/deregistration
};

}//ns rpc

#endif //FEP_OBJECT_REGISTRY_H_INCLUDED
