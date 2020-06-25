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
#include <cstddef>
#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#include <bits/stl_tree.h>  // workaround for a Gnu C++ 5.2.0 library bug
#endif
#include <mutex>
#include <string>
#include <a_util/concurrency/fast_mutex.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>
#include <czmq_library.h>

#include "transmission_adapter/zmq/fep_zmq_receiver.h"
#include "fep_errors.h"

using namespace fep::zmq;

cZMQReceive::cZMQReceive():
    m_bIsMuted(false),
    m_bIsActivated(false),
    m_pCallback(NULL),
    m_pCallee(NULL),
    m_pLoggingFunc(NULL),
    m_pCalleeLogging(NULL)
{
}

cZMQReceive::~cZMQReceive()
{
}

fep::Result cZMQReceive::Enable()
{
    std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
    m_bIsActivated = true;
    return ERR_NOERROR;
}

fep::Result cZMQReceive::Disable()
{
    std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
    m_bIsActivated = false;
    return ERR_NOERROR;
}

fep::Result cZMQReceive::Mute()
{
    m_bIsMuted = true;
    return ERR_NOERROR;
}


fep::Result cZMQReceive::Unmute()
{
    m_bIsMuted = false;
    return ERR_NOERROR;
}


fep::Result cZMQReceive::SetReceiver(tCallbackFuncPtr pCallback, void * pCallee)
{
    fep::Result nResult = ERR_POINTER;
    if( (NULL != pCallback && NULL != pCallee)
        || (NULL == pCallback && NULL == pCallee) )
    {
        m_pCallback = pCallback;
        m_pCallee = pCallee;
        nResult = ERR_NOERROR;
    }
    return nResult;
}

void cZMQReceive::HandleMessage(zmsg_t* msg)
{
    std::unique_lock<a_util::concurrency::fast_mutex> oSync(m_mtxMsgHandler);
    std::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oActivationGuard);
    if (!m_bIsActivated)
    {
        // wrong state --> report and return loan
        std::string strMsg = a_util::strings::format("%s : Received data while not enabled - "
            "dropping packet.", m_strSignalName.c_str());
        LogMessage(strMsg.c_str(), fep::SL_Warning);
    }
    else
    {
        if(NULL != msg && !m_bIsMuted)
        {
            void* pData = zframe_data(zmsg_first(msg));
            size_t szMsg = zmsg_content_size(msg);
            if(NULL != m_pCallback && NULL != m_pCallee)
            {
                m_pCallback(m_pCallee, pData, szMsg);
            }
        }
    }
    zmsg_destroy(&msg);
}


fep::Result cZMQReceive::RegisterLogging(ITransmissionDriver::tLoggingFuncPtr pLoggingFunc, void * pCallee)
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

void cZMQReceive::LogMessage(const char* strMessage, fep::tSeverityLevel eServLevel)
{
    if( NULL != m_pLoggingFunc && NULL != m_pCalleeLogging)
    {
        m_pLoggingFunc(m_pCalleeLogging, strMessage, eServLevel);
    }
}
