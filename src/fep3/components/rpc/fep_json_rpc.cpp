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

#include <string>
#include <jsonrpccpp/common/errors.h>
#include <jsonrpccpp/common/exception.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>

#include "fep_result_decl.h"
#include "fep3/components/rpc/fep_json_rpc.h"
#include "fep3/components/rpc/fep_rpc_intf.h"
#include "fep_errors.h"

#ifdef _MSC_VER
#pragma warning(disable:4290)
#endif

namespace fep
{

namespace detail
{

struct cStringResponse : public fep::IRPCResponse
{
    std::string& m_strBindedString;

    cStringResponse(std::string& strBindedString) : m_strBindedString(strBindedString)
    {
    }
    fep::Result Set(const char* strResponse)
    {
        m_strBindedString = strResponse;
        return Result();
    }
};

class cJSONFEPClientConnector::cImpl
{
    public:
        tClientConnectorInitializerType m_oInit;
        cImpl(const tClientConnectorInitializerType& oInitStruct):
            m_oInit(oInitStruct)
        {
        }
};

cJSONFEPClientConnector::cJSONFEPClientConnector(const tClientConnectorInitializerType& oInitStruct):
    m_pImplementation(new cImpl(oInitStruct))
{
}

cJSONFEPClientConnector::~cJSONFEPClientConnector()
{
    delete m_pImplementation;
}

void cJSONFEPClientConnector::SendRPCMessage(const std::string& message, std::string& result)
    throw(jsonrpc::JsonRpcException)
{
    if (m_pImplementation->m_oInit._rpc)
    {
        fep::IRPC* pRPC = m_pImplementation->m_oInit._rpc;
        cStringResponse oResponse(result);
        Result oRes = pRPC->Connect(m_pImplementation->m_oInit._server_name.c_str(),
                                    m_pImplementation->m_oInit._object_name.c_str());
        if (isFailed(oRes))
        {
            throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_CONNECTOR,
                "error while performing call, connection lost");
        }
        oRes = pRPC->SendRequest(m_pImplementation->m_oInit._server_name.c_str(),
                          m_pImplementation->m_oInit._object_name.c_str(),
                          message.c_str(),
                          &oResponse);
        if (isFailed(oRes))
        {
            auto error_string = oRes.getDescription();
            auto error_label = oRes.getErrorLabel();
            auto error_code = oRes.getErrorCode();
            throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_CONNECTOR,
                a_util::strings::format("error while performing call ((%d) %s: %s)", error_code, error_label, error_string).c_str());
        }
   }
}


} //ns detail

 
}//ns fep
