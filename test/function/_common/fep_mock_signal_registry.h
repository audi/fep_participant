/**
 * Implementation of tx adapter mockup used by FEP functional test cases!
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

#ifndef _FEP_TEST_MOCK_SIGNAL_REGISTRY_H_INC_
#define _FEP_TEST_MOCK_SIGNAL_REGISTRY_H_INC_

#include "signal_registry/fep_signal_registry.h"

using namespace fep;

class cMockSignalRegistryPrivate : public ISignalRegistryPrivate
{
public: // implements ISignalRegistryPrivate
    virtual bool IsMappedSignal(const handle_t hHandle) const
    {
        return false;
    }

    virtual fep::Result GetSignalSampleSize(const handle_t hHandle, size_t & szSize) const
    {
        return ERR_NOERROR;
    }

    virtual fep::Result GetSignalSampleBacklog(handle_t hSignal, size_t& szSampleBacklog) const
    {
        return ERR_NOERROR;
    }
    virtual fep::Result GetSignalHandleFromName(const char* strSignalName, tSignalDirection eDirection, handle_t &hSignalHandle) const
    {
        return ERR_NOERROR;
    }

    virtual fep::Result GetSignalNameFromHandle(handle_t const hSignal, char const *& strSignal) const
    {
        return ERR_NOERROR;
    }

    virtual fep::Result SetSignalSampleBacklog(handle_t hSignal, size_t szSampleBacklog)
    {
        return ERR_NOERROR;
    }};

#endif // _FEP_TEST_MOCK_SIGNAL_REGISTRY_H_INC_