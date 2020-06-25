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
#ifndef _FEP_ZMQ_TRANSEIVER_H_
#define _FEP_ZMQ_TRANSEIVER_H_

#include <cstddef>
#include <string>
#include <a_util/concurrency/detail/fast_mutex_decl.h>

#include "fep_result_decl.h"

typedef struct _zyre_t zyre_t;

namespace fep
{
class cSignalOptions;
}

/**
 * @brief The cAbstractZMQTranceiver class
 * This is the base class for all zmq receiver and transmitter objects.
 * It provides the reference counting for topic registration/deregistration.
 * Furthermore it is responsible for the initialization of these objects.
 */
class cAbstractZMQTranceiver
{
    friend class cZMQDriver;
public:
    /**
    * CTOR
    */
    cAbstractZMQTranceiver();

    /**
    * DTOR
    */
    virtual ~cAbstractZMQTranceiver();

    /**
    * The method \ref Initialize registers a Transmitter or Receiver for a group
    *
    * @param [in] oOptions  SignalOptions
    * @param [in] pNode Pointer to the zmq node
    * @param [in] strModuleName Name of the fep module (legacy stuff)
    * @param [in] dDomainId DomainID
    * @return Standard Error code
    * @retval ERR_INVALID_STATE Already initialized
    * @retval ERR_FAILED Something went wrong
    * @retval ERR_NOERROR Everything went fine
    */
    virtual fep::Result Initialize(fep::cSignalOptions oOptions, zyre_t * &pNode, std::string strModuleName,
                                    int dDomainId);

protected:
    /// Pointer to ZMQ Node
    zyre_t * m_pNode;
    /// ZMQ Group name
    std::string m_strGroupName;
    /// Mutex protecting the zyre-message handling thread
    a_util::concurrency::fast_mutex m_mtxMsgHandler;
    /// Module Name
    std::string m_strModuleName;
    /// Signal name
    std::string m_strSignalName;
    /// Instance Name
    std::string m_strInstanceName;
    /// Signal Size
    size_t m_szSignalSize;
    /// Flag indicating that signal is of variable size
    bool m_bIsVariableSignalSize;
    ///Flag indicating that signal is of reliable type
    bool m_bIsReliable;
};
#endif //_FEP_ZMQ_TRANSEIVER_H_
