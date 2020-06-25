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
#include "fep_zmq_abstract_transceiver.h"
#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#include <bits/stl_tree.h>  // workaround for a Gnu C++ 5.2.0 library bug
#endif
#include <mutex>
#include <a_util/concurrency/fast_mutex.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>
#include <zyre.h>

#include "fep_errors.h"
#include "transmission_adapter/fep_signal_options.h"

cAbstractZMQTranceiver::cAbstractZMQTranceiver() :
    m_pNode(NULL),
    m_szSignalSize(0),
    m_bIsVariableSignalSize(false),
    m_bIsReliable(false)
{
}

cAbstractZMQTranceiver::~cAbstractZMQTranceiver()
{
    std::unique_lock<a_util::concurrency::fast_mutex> oSync2(m_mtxMsgHandler);
    zyre_leave(m_pNode, m_strGroupName.c_str());
}

fep::Result cAbstractZMQTranceiver::Initialize(fep::cSignalOptions oOptions, zyre_t * &pNode, std::string strModuleName,
                                                int dDomainId)
{
    fep::Result nResult = fep::ERR_INVALID_STATE;
    nResult = fep::ERR_INVALID_ARG;
    if(NULL != pNode && !strModuleName.empty())
    {
        m_strModuleName = strModuleName;
        if(oOptions.GetOption("SignalName", m_strSignalName)
            && oOptions.GetOption("SignalSize", m_szSignalSize))
        {
            m_strInstanceName = a_util::strings::format("%s::%s", m_strModuleName.c_str(),
                m_strSignalName.c_str());
            if(false == oOptions.GetOption("IsReliable", m_bIsReliable))
            {
                m_bIsReliable = false;
            }

            if(false == oOptions.GetOption("IsVariableSignalSize", m_bIsVariableSignalSize))
            {
                m_bIsVariableSignalSize = false;
            }

            m_strGroupName = a_util::strings::format("FEP_SIG_%d_%s", dDomainId, m_strSignalName.c_str());
            m_strModuleName = strModuleName;
            m_pNode = pNode;

            bool bIsInitialized = (zyre_join(m_pNode, m_strGroupName.c_str()) == 0);
            if (bIsInitialized)
            {
                nResult = fep::ERR_NOERROR;
            }
            else
            {
                nResult = fep::ERR_FAILED;
            }
            
        }
    }

    return nResult;
}
