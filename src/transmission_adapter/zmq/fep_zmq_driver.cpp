/**

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
 */
/// Someone should add a header here some time

#include "transmission_adapter/zmq/fep_zmq_driver.h"
#include <cstddef>                                          // for NULL
#include <utility>                                           // for pair
#include <a_util/concurrency/fast_mutex.h>                   // for fast_mut...
#include <a_util/result/result_type.h>                       // for Result::...
#include <a_util/strings/strings_format.h>                   // for format
#include <a_util/strings/strings_functions.h>                // for compare
#include <czmq_library.h>                                    // for ziflist_t
#include <zyre.h>                                            // for zyre_stop

#include "_common/fep_networkaddr.h"                         // for cAddress
#include "fep_errors.h"                                      // for ERR_FAILED
#include "fep_zmq_driver_options_verifier.h"                 // for cZMQDriv...
#include "fep_zmq_receiver.h"                                // for cZMQReceive
#include "fep_zmq_signal_options_verifier.h"                 // for ZMQSigna...
#include "fep_zmq_transmitter.h"                             // for cZMQTran...
#include "transmission_adapter/fep_driver_options.h"         // for cDriverO...
#include "transmission_adapter/fep_options_verifier_intf.h"  // for IOptions...
#include "transmission_adapter/fep_signal_options.h"         // for cSignalO...

namespace fep {
class IReceive;
class ITransmit;
}

uint32_t fep::zmq::cZMQDriver::s_ZSysUseCount = 0;
a_util::concurrency::mutex fep::zmq::cZMQDriver::m_mtxSysInit;

fep::zmq::cZMQDriver::cZMQDriver() :
    m_ZSysWasInitialized(false),
    m_pReceiveNode(NULL),
    m_pTransmitNode(NULL),
    m_pPoller(NULL),
    m_dDomainId(0),
    m_pLoggingFunc(NULL),
    m_pCalleeLogging(NULL)
{
}

fep::zmq::cZMQDriver::~cZMQDriver()
{
    Deinitialize();
}


fep::Result fep::zmq::cZMQDriver::Initialize(const cDriverOptions oDriverOptions)
{
    fep::Result nResult = ERR_NOERROR;
    if (!oDriverOptions.GetOption("DomainID", m_dDomainId)
        || !oDriverOptions.GetOption("ModuleName", m_strModuleName))
    {
        nResult = ERR_FAILED;
    }
    oDriverOptions.GetOption("AllowedInterfaces", m_strAllowedInterfaces);
    if (fep::isOk(nResult))
    {
        //TOD: Implement Interface selection
        nResult = ZMQSystemInit(m_strAllowedInterfaces.c_str());
       
        if (fep::isOk(nResult))
        {
            //Create new nodes
            m_pReceiveNode = zyre_new(a_util::strings::format("%s_Recv", m_strModuleName.c_str()).c_str());
            m_pTransmitNode = zyre_new(a_util::strings::format("%s_Tx", m_strModuleName.c_str()).c_str());

            if (NULL == m_pReceiveNode)
            {
                nResult = ERR_FAILED;
            }
            if (NULL == m_pTransmitNode)
            {
                nResult = ERR_FAILED;
            }

            if (a_util::result::isOk(nResult))
            {
                //create new poller
                m_pPoller = zpoller_new(zyre_socket(m_pReceiveNode), NULL);
            }

            if (NULL == m_pPoller)
            {
                nResult = ERR_FAILED;
            }

            if (a_util::result::isOk(nResult))
            {
                if (zyre_start(m_pReceiveNode) != 0)
                {
                    nResult = ERR_FAILED;
                }
            }

            if (a_util::result::isOk(nResult))
            {
                if (zyre_start(m_pTransmitNode) != 0)
                {
                    zyre_stop(m_pReceiveNode);
                    nResult = ERR_FAILED;
                }
            }

            if (a_util::result::isOk(nResult))
            {
                m_pReceptionThread.reset(new std::thread(
                    &cZMQDriver::ReceiveAndDistributeMessages, this));
            }
        }
    }
    return nResult;
}

fep::Result fep::zmq::cZMQDriver::Deinitialize()
{
    m_oShutdownSignal.notify();
    if(m_pReceptionThread)
    {
        m_pReceptionThread->join();
        m_pReceptionThread.reset();
    }
    m_oShutdownSignal.reset();
	a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oSync(m_mtxRecvMap);
    std::map<std::string,cZMQReceive*>::iterator it;
    for(it = m_mapReceiverGroups.begin(); it != m_mapReceiverGroups.end(); ++it)
    {
        delete it->second;
    }
    m_mapReceiverGroups.clear();

    vector<cZMQTransmit*>::iterator iter;
    for(iter = m_vecTransmitters.begin(); iter != m_vecTransmitters.end(); ++iter)
    {
        delete *iter;
    }
    m_vecTransmitters.clear();

    // destroy the poller
    if(NULL != m_pPoller)
    {
        zpoller_destroy(&m_pPoller);
    }
    // gracefully stop zyre
    if(NULL != m_pTransmitNode)
    {
        zyre_stop(m_pTransmitNode);
    }
    if(NULL != m_pReceiveNode)
    {
        zyre_stop(m_pReceiveNode);
    }
    // destroy the node
    if(NULL != m_pTransmitNode)
    {
        zyre_destroy(&m_pTransmitNode);
        m_pTransmitNode= NULL;
    }
    if(NULL != m_pReceiveNode)
    {
        zyre_destroy(&m_pReceiveNode);
        m_pReceiveNode= NULL;
    }
    // prevent atexit() problems by explicitly shutting down platform
    ZMQSystemDeInit();
    return ERR_NOERROR;
}

fep::Result fep::zmq::cZMQDriver::CreateReceiver(IReceive *&pIReceiver, cSignalOptions oOptions)
{
    fep::Result nResult = ERR_FAILED;
    cZMQReceive* pReceiver = new cZMQReceive();

    if(pReceiver)
    {
        if(fep::isOk(pReceiver->Initialize(oOptions, m_pReceiveNode, m_strModuleName,m_dDomainId)))
        {
            if(NULL != m_pCalleeLogging && NULL != m_pLoggingFunc)
            {
                if(fep::isOk(nResult = pReceiver->RegisterLogging(m_pLoggingFunc, m_pCalleeLogging)))
                {
                    nResult = ERR_NOERROR;
                    a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oSync(m_mtxRecvMap);
                    m_mapReceiverGroups[pReceiver->m_strGroupName] = pReceiver;
                    pIReceiver = pReceiver;
                }
            }
        }
        else
        {
            delete pReceiver;
            pReceiver = NULL;
        }
    }

    return nResult;
}


fep::Result fep::zmq::cZMQDriver::CreateTransmitter(fep::ITransmit *&pITransmit, cSignalOptions oOptions)
{
    fep::Result nResult = ERR_FAILED;
    cZMQTransmit* pTransmitter = new cZMQTransmit();

    if(pTransmitter)
    {
        if(fep::isOk(pTransmitter->Initialize(oOptions, m_pTransmitNode, m_strModuleName, m_dDomainId)))
        {
            if(NULL != m_pCalleeLogging && NULL != m_pLoggingFunc)
            {
                if(fep::isOk(nResult = pTransmitter->RegisterLogging(m_pLoggingFunc, m_pCalleeLogging)))
                {
                    nResult = ERR_NOERROR;
                    m_vecTransmitters.push_back(pTransmitter);
                    pITransmit = pTransmitter;
                }
            }
        }
        else
        {
            delete pTransmitter;
            pTransmitter = NULL;
        }
    }
    return nResult;
}

fep::Result fep::zmq::cZMQDriver::DestroyReceiver(IReceive *pIReceiver)
{
    fep::Result nResult = ERR_NOT_FOUND;
    cZMQReceive* pReceiver = static_cast<cZMQReceive*>(pIReceiver);
    std::map<std::string,cZMQReceive*>::iterator it;
	a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oSync(m_mtxRecvMap);
    it = m_mapReceiverGroups.find(pReceiver->m_strGroupName);
    if(it != m_mapReceiverGroups.end())
    {
        delete it->second;
        m_mapReceiverGroups.erase(it);
    }
    return nResult;
};

fep::Result fep::zmq::cZMQDriver::DestroyTransmitter(ITransmit *pITransmitter)
{
    fep::Result nResult = ERR_NOT_FOUND;
    vector<cZMQTransmit*>::iterator it;
    for(it = m_vecTransmitters.begin(); it != m_vecTransmitters.end(); ++it)
    {
        if(static_cast<ITransmit *>(*it) == pITransmitter)
        {
            delete (*it);
            m_vecTransmitters.erase(it);
            nResult = ERR_NOERROR;
            break;
        }
    }
    return nResult;
};

fep::IOptionsVerifier * fep::zmq::cZMQDriver::GetSignalOptionsVerifier()
{
    static ZMQSignalOptionsVerifier s_SignalOptionsVerifier;
    return &s_SignalOptionsVerifier;

}

fep::IOptionsVerifier * fep::zmq::cZMQDriver::GetDriverOptionsVerifier()
{
    static cZMQDriverOptionsVerifier s_DriverOptionsVerifier;
    return &s_DriverOptionsVerifier;

}

fep::Result fep::zmq::cZMQDriver::RegisterLogging(tLoggingFuncPtr pLoggingFunc, void * pCallee)
{
    fep::Result nResult = ERR_INVALID_ARG;
    if( NULL != pLoggingFunc && NULL != pCallee)
    {
        m_pCalleeLogging = pCallee;
        m_pLoggingFunc = pLoggingFunc;
        nResult = ERR_NOERROR;
    }

    return nResult;
}

void fep::zmq::cZMQDriver::ReceiveAndDistributeMessages()
{
    while (!m_oShutdownSignal.is_set())
    {
        void* ret = zpoller_wait(m_pPoller, 50);
        if (zpoller_expired(m_pPoller) || zpoller_terminated(m_pPoller))
        {
            continue;
        }
        else if (ret == zyre_socket(m_pReceiveNode))
        {
            zyre_event_t* event = zyre_event_new(m_pReceiveNode);
            if (event && !m_oShutdownSignal.is_set())
            {
                const char*  strType = zyre_event_type(event);

                if (0 == a_util::strings::compare(strType, "SHOUT"))
                {
                    const char* strGroup = zyre_event_group(event);
                    std::map<std::string,cZMQReceive*>::iterator it;
                    a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oSync(m_mtxRecvMap);
                    it = m_mapReceiverGroups.find(strGroup);
                    if(it != m_mapReceiverGroups.end())
                    {
                        zmsg_t* msg = zyre_event_get_msg(event);
                        it->second->HandleMessage(msg);
                    }
                }

            }
            if(event)
            {
                zyre_event_destroy(&event);
            }
        }
    }
}

// =================================================
// ZSys (backend of ZMQ) is not really usable within a DLL on Windows
// since it uses atexit to shut itself down. Also, who uses threads anyway...
// =================================================
fep::Result fep::zmq::cZMQDriver::ZMQSystemInit(const char * strInterface)
{
    a_util::concurrency::unique_lock<a_util::concurrency::mutex> oSync(m_mtxSysInit);
    s_ZSysUseCount++;
    fep::Result nResult = ERR_NOERROR;
    if (s_ZSysUseCount == 1)
    {
#ifdef __QNX__
        setenv("ZSYS_SIGHANDLER", "false", 1);  // avoid not-interruptible process
#endif
        zsys_init();

        std::string strInterfacSelection;
        if (0 == a_util::strings::getLength(strInterface))
        {
            strInterfacSelection = "*";
        }
        else
        {
            fep::Result nLocalResult = selectInterface(strInterface, strInterfacSelection);
            if (fep::isFailed(nLocalResult))
            {
                strInterfacSelection = "*";
            }
        }
        if (fep::isOk(nResult))
        {
            zsys_set_interface(strInterfacSelection.c_str());
        }
        //else
        //{
        //    zsys_shutdown();
        //}
    }
    if (fep::isFailed(nResult))
    {
        nResult = ERR_FAILED;
    }
    m_ZSysWasInitialized= true;
    return nResult;
}

void fep::zmq::cZMQDriver::ZMQSystemDeInit()
{
    a_util::concurrency::unique_lock<a_util::concurrency::mutex> oSync(m_mtxSysInit);
    if (m_ZSysWasInitialized)
    {
        s_ZSysUseCount--;
        if (s_ZSysUseCount == 0)
        {
            zsys_shutdown();
        }
        m_ZSysWasInitialized = false;
    }
}

fep::Result fep::zmq::cZMQDriver::selectInterface(const char *strInterface, std::string &strSelection)
{
    fep::Result nResult = ERR_NOERROR;
    int found_index = -1;
    fep::networkaddr::cAddress oAddress(strInterface);
    if (!oAddress.isValid())
    {
        nResult = ERR_INVALID_ARG;
    }
    if (fep::isOk(nResult))
    {
        std::string strAddr = oAddress.GetHostAddrString();
        ziflist_t *iflist = ziflist_new_ipv6();
        if (!iflist)
        {
            nResult = ERR_FAILED;
        }
        else
        {
            const char *name = ziflist_first(iflist);
            {
                int idx = -1;
                while (name)
                {
                    idx++;
                    if (strAddr == ziflist_address(iflist))
                    {
                        found_index = idx;
                        break;
                    }
                    name = ziflist_next(iflist);
                }
            }
        }
        ziflist_destroy(&iflist);
    }
    if (found_index < 0)
    {
        nResult = ERR_FAILED;
    }
    else
    {
        strSelection = a_util::strings::format("%d", found_index);
    }
    return nResult;
}
