/**
*
* FEP RPC Legacy 
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
#ifndef FEP_RPC_STUBS_LEGACY_HEADER_
#define FEP_RPC_STUBS_LEGACY_HEADER_

#include <a_util/result/result_type.h>
#include <jsonrpccpp/server/abstractserverconnector.h>
#include <rpc_pkg/rpc_server.h>
#include <rpc_pkg/json_rpc.h>
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/rpc/fep_json_rpc.h"
#include "module/fep_module_intf.h"

namespace fep
{

namespace legacy
{
    /**
     * \brief Parent class of all rpc clients.
     * \tparam Stub Generated client stub.
     * \tparam Interface Server id interface.
     */
    template <typename Stub, typename Interface>
    class rpc_object_client:
        public ::rpc::jsonrpc_remote_object<Stub, fep::detail::cJSONFEPClientConnector, fep::detail::tClientConnectorInitializerType>
    {
        protected:
            rpc_object_client() = delete;
        public:
            /// The json rpc base class
            typedef ::rpc::jsonrpc_remote_object<Stub, fep::detail::cJSONFEPClientConnector, fep::detail::tClientConnectorInitializerType> base_class;

            /**
            * CTOR
            *
            * @param [in] strElementName The name of this client
            * @param [in] strRPCServerObjectName The name of the RPC server we connect to 
            * @param [in] oModuleToBind The FEP participant this client is bound to 
            */
            explicit rpc_object_client(const char* strElementName,
                const char* strRPCServerObjectName,
                const IModule& oModuleToBind) :
                base_class(fep::detail::tClientConnectorInitializerType(
                    strElementName, strRPCServerObjectName, *oModuleToBind.GetRPC()))
            {
                // Setting default timeout parameter of client to property tree
                oModuleToBind.GetPropertyTree()->SetPropertyValue(FEP_RPC_CLIENT_REMOTE_TIMEOUT_PATH, 5000);
            }

            /**
            * @retval The ID of the bound rpc server
            */
            const char* GetRPCObjectIID() const
            {
                return Interface::FEP_RPC_IID;
            }
    };

} //ns legacy

} //ns fep


#endif //FEP_RPC_STUBS_LEGACY_HEADER_


