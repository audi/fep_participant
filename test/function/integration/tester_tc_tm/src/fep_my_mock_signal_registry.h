/**
 * Implementation of adapted signal registry mockup used by this test
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

#ifndef _FEP_TEST_MY_MOCK_SIGNAL_REGISTRY_H_INC_
#define _FEP_TEST_MY_MOCK_SIGNAL_REGISTRY_H_INC_

#include "function/_common/fep_mock_signal_registry.h"

using namespace fep;

class cMyMockSignalRegistryPrivate : public cMockSignalRegistryPrivate
{
public: // implements ISignalRegistryPrivate
    virtual fep::Result GetSignalSampleSize(const handle_t hHandle, size_t & szSize) const
    {
        if (m_hHandle != hHandle)
        {
            return ERR_UNEXPECTED;
        }

        szSize = m_szSize;
        return ERR_NOERROR;
    }

    virtual fep::Result GetSignalSampleBacklog(handle_t hSignal, size_t& szSampleBacklog) const
    {
        if (m_hHandle != hSignal)
        {
            return ERR_UNEXPECTED;
        }

        szSampleBacklog = m_szSampleBacklog;
        return ERR_NOERROR;
    }

public:
    void MockReset()
    {
        m_hHandle = NULL;
        m_szSize = 0;
        m_szSampleBacklog = 0;
    }

    void MockSetup(const handle_t hHandle, const size_t & szSize, const size_t& szSampleBacklog)
    {
        m_hHandle = hHandle;
        m_szSize = szSize;
        m_szSampleBacklog = szSampleBacklog;
    }

private:
    handle_t m_hHandle;
    size_t m_szSize;
    size_t m_szSampleBacklog;
};

#endif // _FEP_TEST_MY_MOCK_SIGNAL_REGISTRY_H_INC_