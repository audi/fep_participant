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

#ifndef FEP_RPC_JSON_RPC_CLIENT_H_INCLUDED
#define FEP_RPC_JSON_RPC_CLIENT_H_INCLUDED

#include <string>
#include <jsonrpccpp/client/iclientconnector.h>
#include <jsonrpccpp/common/exception.h>
#include <jsonrpccpp/server/abstractserverconnector.h>
#include <rpc_pkg/rpc_server.h>

#include "fep_participant_export.h"

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4275 )
#endif

 ///@cond nodoc
namespace fep
{
class IRPC;

namespace detail
{
    struct tClientConnectorInitializerType
    {
        tClientConnectorInitializerType(const char* server_name,
            const char* object_server_name,
            fep::IRPC& rpc) : _server_name(server_name),
            _object_name(object_server_name),
            _rpc(&rpc)
        {
        }
        std::string   _server_name;
        std::string   _object_name;
        fep::IRPC*    _rpc;
    };

    /**
     * Connector that sends RPC messages 
     */
    class FEP_PARTICIPANT_EXPORT cJSONFEPClientConnector: public jsonrpc::IClientConnector
    {
        public:
            /**
             * Constructor
             */
            cJSONFEPClientConnector(const tClientConnectorInitializerType& strElementRPCObjectURL);
            ~cJSONFEPClientConnector();

        public:
            void SendRPCMessage(const std::string& message, std::string& result)
                throw(jsonrpc::JsonRpcException) override;

        private:
            class cImpl;

            cImpl* m_pImplementation;
    };


    class  cJSONFEPPServerConnector : public jsonrpc::AbstractServerConnector
    {
    public:
        bool StartListening() override
        {
            return true;
        }

        bool StopListening() override
        {
            return false;
        }

        bool SendResponse(const std::string& response, void* addInfo)
        {
            ::rpc::IResponse* pResponse = static_cast<::rpc::IResponse*>(addInfo);
            pResponse->Set(response.data(), response.size());
            return true;
        }

        bool OnRequest(const std::string& request, ::rpc::IResponse* response)
        {
            std::string response_value;
            ProcessRequest(request, response_value);
            response->Set(response_value.c_str(), response_value.size());
            return true;
        }
    };

}   //namespace detail


}//ns fep
 ///@endcond nodoc

#ifdef WIN32
#pragma warning( pop )
#endif

#endif //FEP_RPC_JSON_RPC_CLIENT_H_INCLUDED
