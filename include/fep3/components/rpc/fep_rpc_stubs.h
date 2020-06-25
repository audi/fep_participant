/**
*
* ADTF System Client.
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
#ifndef FEP_RPC_STUBS_HEADER_
#define FEP_RPC_STUBS_HEADER_

#include <a_util/result/result_type.h>
#include <jsonrpccpp/server/abstractserverconnector.h>

#include "module/fep_module_intf.h"
#include "fep3/components/rpc/fep_legacy_rpc_stubs.h"

namespace fep
{

/**
 * \brief Parent class of all rpc clients.
 * \tparam Stub Generated client stub.
 * \tparam Interface Server id interface.
 */
template <typename Stub, typename Interface>
class rpc_object_client :
    public ::rpc::jsonrpc_remote_object<Stub, detail::cJSONFEPClientConnector, detail::tClientConnectorInitializerType>,
    public IRPCObjectClient
{
    
protected:
    rpc_object_client() = delete;
public:
    /// The json rpc base class
    typedef ::rpc::jsonrpc_remote_object<Stub, detail::cJSONFEPClientConnector, detail::tClientConnectorInitializerType> base_class;

    /**
    * CTOR
    *
    * @param [in] server_name The name of the RPC server we connect to
    * @param [in] server_object_name The name of the RPC object / RPC component we connect to
    * @param [in] rpc The RPC implementation
    */
    explicit rpc_object_client(const char* server_name,
        const char* server_object_name,
        IRPC& rpc) :
        base_class(detail::tClientConnectorInitializerType(
                   server_name, server_object_name, rpc))
    {
        // Setting default timeout parameter of client to property tree
        //oModuleToBind.GetPropertyTree()->SetPropertyValue(FEP_RPC_CLIENT_REMOTE_TIMEOUT_PATH, 5000);
    }

    /**
     * @retval The ID of the bound rpc server
     */
    std::string getRPCObjectIID() const override
    {
        return fep::getRPCIID<Interface>();
    }

    /**
     * @retval The ID of the bound rpc server
     */
    std::string getRPCObjectDefaultName() const override
    {
        return fep::getRPCDefaultName<Interface>();
    }
};

/**
 * \brief Parent class of all rpc clients.
 * \tparam Stub Generated client stub.
 * \tparam Interface Server id interface.
 */
template <typename Stub, typename Interface>
class rpc_object_proxy :
    public ::rpc::jsonrpc_remote_interface<Stub, Interface, detail::cJSONFEPClientConnector, detail::tClientConnectorInitializerType>,
    public IRPCObjectClient
{

protected:
    rpc_object_proxy() = delete;
public:
    /// The json rpc base class
    typedef ::rpc::jsonrpc_remote_interface<Stub, Interface, detail::cJSONFEPClientConnector, detail::tClientConnectorInitializerType> base_class;

    /**
    * CTOR
    *
    * @param [in] server_name The name of the RPC server we connect to
    * @param [in] server_object_name The name of the RPC object / RPC component we connect to
    * @param [in] rpc The RPC implementation
    */
    explicit rpc_object_proxy(const char* server_name,
        const char* server_object_name,
        IRPC& rpc) :
        base_class(detail::tClientConnectorInitializerType(
            server_name, server_object_name, rpc))
    {
        // Setting default timeout parameter of client to property tree
        //oModuleToBind.GetPropertyTree()->SetPropertyValue(FEP_RPC_CLIENT_REMOTE_TIMEOUT_PATH, 5000);
    }

    /**
     * @retval The ID of the bound rpc server
     */
    std::string getRPCObjectIID() const override
    {
        return fep::getRPCIID<Interface>();
    }

    /**
     * @retval The ID of the bound rpc server
     */
    std::string getRPCObjectDefaultName() const override
    {
        return fep::getRPCDefaultName<Interface>();
    }
};

class cFepResponseToRPCResponse : public ::rpc::IResponse
{
protected:
    fep::IRPCResponse & m_oResponse;
public:
    cFepResponseToRPCResponse(fep::IRPCResponse & oResponce) :
        m_oResponse(oResponce)
    {

    }

    void Set(const char* strResponse, size_t nResponseSize)
    {
        if (strResponse[nResponseSize - 1] != '\0')
        {
            std::string strNullTerminatedString(strResponse, nResponseSize);
            m_oResponse.Set(strNullTerminatedString.c_str());
        }   
        else
        {
            m_oResponse.Set(strResponse);
        }
    }
};

/**
* \brief Parent class of all rpc servers.
* \tparam ServerStub Generated server stub.
* \tparam Interface Server id interface.
*/
template <typename ServerStub, typename Interface>
class rpc_object_server:
    protected ::rpc::jsonrpc_object_server<ServerStub, detail::cJSONFEPPServerConnector>,
    public fep::IRPCObjectServer
{

    public: // implements IRPCObjectServer
        //! @copydoc  fep::IRPCObjectServer::HandleRequest
        fep::Result HandleRequest(const char* strContentType,
                                  const char* strRequestMessage,
                                  fep::IRPCResponse& oResponseMessage) override
        {
            try
            {
                cFepResponseToRPCResponse response(oResponseMessage);
                std::string strRequest(strRequestMessage);
                if (!::rpc::jsonrpc_object_server<ServerStub, detail::cJSONFEPPServerConnector>::OnRequest(strRequest, &response))
                {
                    return fep::ERR_INVALID_ARG;
                }
            }
            catch(...)
            {
               return fep::ERR_UNEXPECTED;
            }

           return fep::Result();
        }

        //! @copydoc  fep::IRPCObjectServer::GetRPCServerIID
        const char* GetRPCServerIID() const override
        {
            return Interface::RPC_IID;
        }

        //! @copydoc  fep::IRPCObjectServer::GetRPCServerObjectName
        std::string GetRPCServerObjectName() const override
        {
            return m_strRPCObjectName;
        }

        //! @copydoc  fep::IRPCObjectServer::SetRPCServerObjectName
        void SetRPCServerObjectName(const char* strName) override
        {
            m_strRPCObjectName = strName;
        }

        //! @copydoc  fep::IRPCObjectServer::GetRPCInterfaceDefinition
        std::string GetRPCInterfaceDefinition() const override
        {
            return ServerStub::interface_definition;
        }

    private:
        std::string m_strRPCObjectName;
};

} //ns fep


#endif //FEP_RPC_STUBS_HEADER_


