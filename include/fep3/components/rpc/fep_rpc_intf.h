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
#include "./../../../fep_types.h"
#include "./../../components/base/fep_component.h"
#include "./../../rpc_components/rpc/fep_rpc_definition.h"
#include "./../../rpc_components/rpc/fep_rpc_object_client_intf.h"

#ifndef FEP_RPC_OBJECT_REGISTRY_H_INCLUDED
#define FEP_RPC_OBJECT_REGISTRY_H_INCLUDED

namespace fep
{


/// Interface of a RPC response
class IRPCResponse
{
protected:
    /**
    * DTOR
    */
    virtual ~IRPCResponse() {};
public:

    /**
    * \c Sets the response message.
    *
    * @param[in] strResponse The response data.
    *
    * @retval ERR_NOERROR Everything went as expected.
    */
    virtual fep::Result Set(const char* strResponse) = 0;
};

/// Interface of a RPC server
class IRPCObjectServer
{
    protected:
        /**
        * DTOR
        */
        virtual ~IRPCObjectServer() {};

    public:
        /**
        * \c Sets the server name.
        *
        * @param[in] strName The server name.
        */
        virtual void SetRPCServerObjectName(const char* strName) = 0;

        /**
        * @retval The server ID.
        */
        virtual const char* GetRPCServerIID() const = 0;

        /**
        * @retval The server name.
        */
        virtual std::string GetRPCServerObjectName() const = 0;

        /**
        * @retval The json interface definition of this server.
        */
        virtual std::string GetRPCInterfaceDefinition() const = 0;

        /**
        * \c Handles a request send to this server.
        *
        * @param[in]  strContentType The content type of the request.
        * @param[in]  strRequestMessage The request message.
        * @param[out] oResponseMessage The response message.
        *
        * @retval ERR_NOERROR Everything went as expected.
        * @retval ERR_INVALID_ARG Invalid arguments for the message.
        * @retval ERR_UNEXPECTED Something else went wrong.
        */
        virtual fep::Result HandleRequest(const char* strContentType,
                                          const char* strRequestMessage,
                                          IRPCResponse& oResponseMessage) = 0;
};

/// Interface of a RPC server registry used to register RPC servers
class IRPCObjectServerRegistry
{
    protected:
        /**
        * DTOR
        */
        virtual ~IRPCObjectServerRegistry() {};

    public:
        /**
        * \c Registers a RPC server with a unique \c strName
        *
        * @param[in] strName The name of the server. \c oRPCObjectServerInstance name is updated.
        * @param[in,out] oRPCObjectServerInstance The server that is registered.
        * \note The server instance must be available as long as the server is registered.
        *
        * @retval ERR_INVALID_ARG If the name already exists
        * @retval ERR_NOERROR Everything went as expected.
        */
        virtual fep::Result RegisterObjectServer(const char* strName,
                                                 IRPCObjectServer& oRPCObjectServerInstance) = 0;

        /**
        * \c Unregisters a server by its \c strName.
        *
        * @param[in] strName The name of the server.
        * \note Only after the server is uregistered it may be deleted.
        *
        * @retval ERR_NOT_FOUND If the server is not registered.
        * @retval ERR_NOERROR Everything went as expected.
        */
        virtual fep::Result UnregisterObjectServer(const char* strName) = 0;

};

/// The RPC access interface used to register a object rpc server or to send rpc messages as a client.
class IRPC
{
    public:
        FEP_COMPONENT_IID("IRPC");

    protected:
        /**
        * DTOR
        */
        ~IRPC() {};
    public:
        /**
        * \c Connects the \c strElement to a RPC server
        * 
        * @param[in] strElement The name of the element that connects to the server.
        * @param[in] strServerObjectName The name of the server to connect to.
        * \note The server must first be registered.
        *
        * @retval Standard results.
        */
        virtual fep::Result Connect(const char* strElement, const char* strServerObjectName) = 0;

        /**
        * \c Sends a request from \c strElement to the RPC server \c strServerObjectName.
        *
        * @param[in] strElement The name of the element that sends the request.
        * \note The element must already be connected.
        * @param[in] strServerObjectName The name of the server to send the request to.
        * \note The server must already be registered.
        * @param[in] strMessage The request message.
        * @param[in,out] pResponse The request response.
        * \note The response must already be allocated.
        * 
        * @retval ERR_NOT_CONNECTED If not connected.
        * @retval ERR_NOT_INITIALISED Not correct initialized.
        * @retval ERR_TIMEOUT A timeout
        * @retval ERR_CANCELLED Request was cancelled
        * @retval ERR_NOERROR Everything went as expected.
        */
        virtual fep::Result SendRequest(const char* strElement, 
                                        const char* strServerObjectName,
                                        const char* strMessage,
                                        IRPCResponse* pResponse) const = 0;

        /**
        * \c Retrieve the RPC server registry
        *
        * @retval The RPC server registry
        */
        virtual IRPCObjectServerRegistry* GetRegistry() const = 0;

        virtual std::string GetLocalName() const = 0;
};

}//ns rpc

#endif //FEP_RPC_H_INCLUDED
