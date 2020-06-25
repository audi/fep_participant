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

#include <utility>
#include <a_util/result/result_type.h>
#include <a_util/result/error_def.h>
#include <a_util/strings/strings_functions.h>
#include "fep_errors.h"
#include "fep_rpc_object_registry.h"

namespace fep
{
namespace detail
{

class cResponseWrapper : public fep::IRPCResponse
{
    ::rpc::IResponse& m_oResponse;
public:
    cResponseWrapper(::rpc::IResponse& oResponse) : m_oResponse(oResponse)
    {
    }
    virtual fep::Result Set(const char* strResponse)
    {
        m_oResponse.Set(strResponse, a_util::strings::getLength(strResponse));
        return fep::Result();
    }
};

class cResponse : public ::rpc::IResponse
{
    std::string& m_strRefString;
    public:
        explicit cResponse(std::string& strRefString) : m_strRefString(strRefString)
        {
        }
        void Set(const char* strResponse, size_t nResponseSize)
        {
            m_strRefString = strResponse;
        }
};




cFEPObjectServerWrapper::cFEPObjectServerWrapper(fep::IRPCObjectServer& oObjectServer) :
        m_oObjectServer(oObjectServer)
{
}

cFEPObjectServerWrapper::~cFEPObjectServerWrapper()
{
}

a_util::result::Result cFEPObjectServerWrapper::HandleCall(const char* strRequest,
    size_t nRequestSize,
    ::rpc::IResponse& oResponse)
{
    cResponseWrapper oRespWrapper(oResponse);
    return m_oObjectServer.HandleRequest("fep/json", strRequest, oRespWrapper);
}

fep::IRPCObjectServer& cFEPObjectServerWrapper::GetObjectServer() const
{
    return m_oObjectServer;
}
}   //end detail



cRPCObjectRegistry::cRPCObjectRegistry() : ::rpc::cRPCObjectsRegistry()
{
}

cRPCObjectRegistry::~cRPCObjectRegistry()
{
}


fep::Result cRPCObjectRegistry::ProcessRequest(const std::string& strObjectName,
                                               const std::string& strRequest,
                                               std::string& strResponse)
{
    ::rpc::cRPCObjectsRegistry::cLockedRPCObject oObject = ::rpc::cRPCObjectsRegistry::GetRPCObject(strObjectName.c_str());
    if (!oObject)
    {
        RETURN_ERROR_DESCRIPTION(fep::ERR_INVALID_ARG, "Object not found");
    }
    else
    {
        std::string strResponseType;
        detail::cResponse oResponse(strResponse);
        return oObject->HandleCall(strRequest.c_str(), strRequest.length(), oResponse);
    }
}

fep::Result cRPCObjectRegistry::RegisterObjectServer(const char* strName,
                                                     IRPCObjectServer& oRPCObjectServerInstance)
{
    std::lock_guard<a_util::concurrency::mutex> m_oLock(m_csMapLock);
    auto it = m_mapObjects.find(strName);
    if (it != m_mapObjects.end())
    {
        RETURN_ERROR_DESCRIPTION(fep::ERR_INVALID_ARG, "Already exists");
    }
    detail::cFEPObjectServerWrapper* pNew = new detail::cFEPObjectServerWrapper(oRPCObjectServerInstance);
    auto Res = ::rpc::cRPCObjectsRegistry::RegisterRPCObject(strName, pNew);
    if (a_util::result::isFailed(Res))
    {
        delete pNew;
        return Res;
    }
    oRPCObjectServerInstance.SetRPCServerObjectName(strName);
    m_mapObjects[strName].reset(pNew);
    return Result();
}

fep::Result cRPCObjectRegistry::UnregisterObjectServer(const char* strName)
{
    std::lock_guard<a_util::concurrency::mutex> m_oLock(m_csMapLock);
    auto it = m_mapObjects.find(strName);
    if (it == m_mapObjects.end())
    {
        RETURN_ERROR_DESCRIPTION(fep::ERR_NOT_FOUND, "%s is not registered", strName);
    }
    ::rpc::cRPCObjectsRegistry::UnregisterRPCObject(strName);
    m_mapObjects.erase(strName);
    return Result();
}

std::vector<std::string> cRPCObjectRegistry::GetObjects() const
{
    std::lock_guard<a_util::concurrency::mutex> m_oLock(m_csMapLock);
    std::vector<std::string> lstRes;
    for (const auto& it : m_mapObjects)
    {
        lstRes.push_back(it.first);
    }
    return lstRes;
}

std::vector<std::string> cRPCObjectRegistry::GetRPCIIDsForObject(const std::string& strObject) const
{
    std::lock_guard<a_util::concurrency::mutex> m_oLock(m_csMapLock);
    const auto& it = m_mapObjects.find(strObject);
    std::vector<std::string> lstRes;
    if (it != m_mapObjects.cend())
    {
        lstRes.push_back(it->second->GetObjectServer().GetRPCServerIID());
    }
    return lstRes;
}

std::string cRPCObjectRegistry::GetRPCInterfaceDefinition(const std::string& strIID, const std::string& strObject) const
{
    std::lock_guard<a_util::concurrency::mutex> m_oLock(m_csMapLock);
    const auto& it = m_mapObjects.find(strObject);
    if (it != m_mapObjects.cend())
    {
        if (a_util::strings::compare(it->second->GetObjectServer().GetRPCServerIID(), strIID.c_str()) == 0)
        {
            return it->second->GetObjectServer().GetRPCInterfaceDefinition();
        }
    }
    return std::string();
}


}//ns rpc
