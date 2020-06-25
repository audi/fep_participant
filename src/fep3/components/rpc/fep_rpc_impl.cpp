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

#include <atomic>
#include <cstdint>
#include <a_util/result/result_type.h>
#include <a_util/result/error_def.h>
#include <a_util/strings/strings_functions.h>
#include <a_util/system/system.h>
#include "_common/fep_timestamp.h"
#include "fep3/components/base/component_intf.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/components/rpc/fep_element_object_client.h"
#include "fep3/components/rpc/fep_rpc_element_object.h"
#include "fep3/components/rpc/fep_rpc_object_registry.h"
#include "fep3/components/rpc/fep_rpc_server_connection.h"
#include "fep3/components/clock/clock_service_intf.h"
#include "fep_errors.h"
#include "fep_rpc_impl.h"
#include "messages/fep_command_access_intf.h"
#include "messages/fep_command_rpc.h"
#include "messages/fep_command_rpc_intf.h"
#include "module/fep_module_intf.h"

#ifdef GetMessage
#undef GetMessage
#endif

namespace fep
{
namespace detail
{
    uint32_t get_request_id()
    {
#ifndef __QNX__
        static std::atomic<uint32_t> ui32Value{0};
#else
        static std::atomic_uint_fast32_t ui32Value{0};
#endif
        ui32Value++;
        return ui32Value;
    }
}
cRPC::cRPC(const IModule& module) :
    ComponentBaseLegacy(module),
    m_oElementObject(m_oRegistry),
    m_pCommandAccess(nullptr),
    m_pClockService(nullptr),
    m_pPropertyTree(nullptr),
    m_pRegistry(&m_oRegistry)
{
}

cRPC::~cRPC()
{
}

fep::Result cRPC::create()
{
    return Initialize(*_module->GetCommandAccess(),
                      *_components->getComponent<IPropertyTree>(),
                      _module->GetName());
}

fep::Result cRPC::destroy()
{
    Shutdown();
    return fep::Result();
}

void* cRPC::getInterface(const char* iid)
{
    if (fep::getComponentIID<IRPC>() == iid)
    {
        return static_cast<IRPC*>(this);
    }
    else if (fep::getComponentIID<IRPCInternal>() == iid)
    {
        return static_cast<IRPCInternal*>(this);
    }
    else
    {
        return nullptr;
    }
}


fep::Result cRPC::Initialize(ICommandAccess& oCommandAccess,
                      IPropertyTree& oPropertyTree,
                      const std::string& strStartupName)
{
    //default
    fep::setProperty(oPropertyTree, FEP_RPC_CLIENT_REMOTE_TIMEOUT_PATH, 5000);

    m_strLocalName = strStartupName;
    m_oRegistry.RegisterObjectServer(rpc::IRPCElementInfo::DEFAULT_NAME, m_oElementObject);
    m_pCommandAccess = &oCommandAccess;
    m_pPropertyTree  = &oPropertyTree;
    m_pCommandAccess->RegisterCommandListener(this);
    return fep::Result();
}

void cRPC::Shutdown()
{
    m_oServerConnections.StopAllConnections();
    if (m_pCommandAccess)
    {
        m_pCommandAccess->UnregisterCommandListener(this);
        m_pCommandAccess = nullptr;
    }
    m_oRegistry.UnregisterObjectServer(rpc::IRPCElementInfo::DEFAULT_NAME);
}

void cRPC::setClockService(IClockService* pClockService)
{
    //Henne Ei Problem ;-)
    m_pClockService = pClockService;
}

void cRPC::setLocalName(const std::string& strName)
{
    m_strLocalName = strName;
}

std::string cRPC::GetLocalName() const
{
    return m_strLocalName;
}

fep::Result wait_for_response(detail::cResponseQueue* pQueue,
                              uint32_t currentRequestID,
                              IRPCResponse& oResponse, 
                              int nTimeoutMS)
{
    timestamp_t tmBegin = a_util::system::getCurrentMilliseconds();
    bool bBreak = false;
    while(!bBreak)
    {
        bool bIsNotEmpty = pQueue->WaitForNotEmpty(nTimeoutMS / 2);
        if (bIsNotEmpty)
        {
            detail::cResponseQueue::Item oItemFound;
            if (pQueue->PopItemMatchesRequestId(currentRequestID, oItemFound))
            {
                if (oItemFound.m_eCode == detail::cResponseQueue::Item::Stop)
                {
                    return Result(ERR_CANCELLED);
                }
                else
                {
                    oResponse.Set(oItemFound.m_strResponse.c_str());
                    return Result();
                }
            }
        }
        timestamp_t tmEnd = a_util::system::getCurrentMilliseconds();
        // wait for timeout
        if (tmEnd - tmBegin > nTimeoutMS)
        {
            bBreak = true;
        }
    }
    return Result(ERR_TIMEOUT);
}

fep::Result cRPC::Connect(const char* strElement, const char* strServerObjectName)
{
    m_oServerConnections.AddForKey(strElement);
    //usually we can check if the strServerObjectName really exist, but wont do this
    return Result();
}

timestamp_t cRPC::GetClockTime() const
{
    if (m_pClockService)
    {
        return m_pClockService->getTime();
    }
    else
    {
        return 0;
    }
}

fep::Result cRPC::SendRequest(const char* strElement,
                              const char* strServerObjectName,
                              const char* strMessage,
                              IRPCResponse* pResponse) const
{
    if (m_pCommandAccess)
    {
        detail::cServerRPCConnections::cResponseQueueEasyRef oResponseQueue = m_oServerConnections.Get(strElement);
        if (oResponseQueue.Get())
        {
            uint32_t currentRequestID = detail::get_request_id();
            cRPCCommand oCommand(IRPCCommand::request,
                                 m_strLocalName,
                                 strElement,
                                 strServerObjectName,
                                 currentRequestID,
                                 strMessage, 
                                 fep::GetTimeStampMicrosecondsUTC(),
                                 GetClockTime());
            Result nTransmitRes = m_pCommandAccess->TransmitCommand(&oCommand);
            if (fep::isFailed(nTransmitRes))
            {
                return nTransmitRes;
            }
            else
            {
                // default value of this property is 5000 ms
                int nTimeoutMS = 0;
                m_pPropertyTree->GetPropertyValue(FEP_RPC_CLIENT_REMOTE_TIMEOUT_PATH, nTimeoutMS);
                if (nTimeoutMS < 1)
                {
                    return fep::ERR_INVALID_ARG;
                }
                Result nWaitRes = wait_for_response(oResponseQueue.Get(), currentRequestID, *pResponse, nTimeoutMS);
                return nWaitRes;
            }
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(fep::ERR_NOT_CONNECTED, "No Connect called");
        }
    }
    RETURN_ERROR_DESCRIPTION(fep::ERR_NOT_INITIALISED, "No CommandAccess Set");
}

IRPCObjectServerRegistry* cRPC::GetRegistry() const
{
     return m_pRegistry;
}

fep::Result cRPC::Update(IRPCCommand const * poCommand)
{
    if (a_util::strings::compare(poCommand->GetReceiver(), m_strLocalName.c_str()) == 0)
    {
        if (poCommand->GetType() == IRPCCommand::response)
        {
            return HandleResponse(poCommand);
        } 
        else if (poCommand->GetType() == IRPCCommand::request)
        {
            return HandleRequest(poCommand);
        }
    }
    return Result();
}

fep::Result cRPC::HandleRequest(IRPCCommand const * poCommand)
{
    //create own thread ???
    //But on Implementation site 
    std::string strResponse;
    m_oRegistry.ProcessRequest(poCommand->GetRPCServerObject(),
        poCommand->GetRPCContent(),
        strResponse);

    cRPCCommand oResponseCommand(IRPCCommand::response,
        m_strLocalName,
        poCommand->GetSender(),
        poCommand->GetRPCServerObject(),
        poCommand->GetRequestid(),
        strResponse,
        fep::GetTimeStampMicrosecondsUTC(),
        GetClockTime());

    m_pCommandAccess->TransmitCommand(&oResponseCommand);
    return Result();
}

fep::Result cRPC::HandleResponse(IRPCCommand const * poCommand)
{
    detail::cServerRPCConnections::cResponseQueueEasyRef oResponseQueue = m_oServerConnections.Get(poCommand->GetSender());
    uint32_t nRequestId = poCommand->GetRequestid();

    if (oResponseQueue.Get())
    {
        oResponseQueue.Get()->Push(detail::cResponseQueue::Item(nRequestId, poCommand->GetRPCContent()));
    }
    return Result();
}


}//ns fep
