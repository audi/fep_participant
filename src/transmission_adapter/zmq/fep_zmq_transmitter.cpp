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
#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#include <bits/stl_tree.h>  // workaround for a Gnu C++ 5.2.0 library bug
#endif
#include <mutex>
#include <string>
#include <a_util/concurrency/fast_mutex.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>
#include <czmq_library.h>
#include <zyre.h>

#include "transmission_adapter/zmq/fep_zmq_transmitter.h"
#include "fep_errors.h"

using namespace fep::zmq;

cZMQTransmit::cZMQTransmit() : 
    m_bIsMuted(false),
    m_pLoggingFunc(NULL),
    m_pCalleeLogging(NULL)
{
}

cZMQTransmit::~cZMQTransmit()
{
}

fep::Result cZMQTransmit::Transmit(const void *pData, size_t szSize)
{
    fep::Result nResult = ERR_NOERROR;
    std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
    if(!m_bIsActivated)
    {
        LogMessage(a_util::strings::format("%: Transmission failure - transmitter is not enabled.",
            m_strSignalName.c_str()).c_str(), fep::SL_Warning);
        nResult = ERR_INVALID_STATE;
    }
    else if(NULL == pData || 0 >= szSize)
    {
        nResult = ERR_INVALID_ARG;
    }
    else if(!m_bIsVariableSignalSize && szSize != m_szSignalSize)
    {
        nResult= ERR_FAILED;
    }
    
    else if(!m_bIsMuted && fep::isOk(nResult))
    {
        zmsg_t* msg = zmsg_new();
        zmsg_addmem(msg, pData, szSize);
        if(0 != zyre_shout(m_pNode, m_strGroupName.c_str(), &msg))
        {
            nResult = ERR_FAILED;
        }
    }

    return nResult;
}

fep::Result cZMQTransmit::Enable()
{
    std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
    m_bIsActivated = true;
    return ERR_NOERROR;
}

fep::Result cZMQTransmit::Disable()
{
    std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
    m_bIsActivated = false;
    return ERR_NOERROR;
}

fep::Result cZMQTransmit::Mute()
{
    m_bIsMuted = true;
    return ERR_NOERROR;
}

fep::Result cZMQTransmit::Unmute()
{
    m_bIsMuted = false;
    return ERR_NOERROR;
}

fep::Result cZMQTransmit::RegisterLogging(ITransmissionDriver::tLoggingFuncPtr pLoggingFunc, void * pCallee)
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

void cZMQTransmit::LogMessage(const char* strMessage, fep::tSeverityLevel eServLevel)
{
    if( NULL != m_pLoggingFunc && NULL != m_pCalleeLogging)
    {
        m_pLoggingFunc(m_pCalleeLogging, strMessage, eServLevel);
    }
}
