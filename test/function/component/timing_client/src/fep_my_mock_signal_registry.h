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

using namespace fep;

#include "function/_common/fep_mock_signal_registry.h"

class cMyMockSignalRegistry : public cMockSignalRegistryPrivate
{
public:
    struct tMockSignal
    {
        handle_t hHandle;
        tSignalDirection eDirection;
        size_t szSize;
    };

public: // implements ISignalRegistryPrivate
    virtual fep::Result GetSignalSampleSize(const handle_t hHandle, size_t & szSize) const
    {
        fep::Result nResult = ERR_UNEXPECTED;
        for (std::map<std::string, tMockSignal>::const_iterator it = m_mapSignals.begin();
            it != m_mapSignals.end(); ++it)
        {
            if ((*it).second.hHandle == hHandle)
            {
                nResult = ERR_NOERROR;
                szSize = (*it).second.szSize;
            }
        }
        return nResult;
    }

    virtual fep::Result GetSignalHandleFromName(const char* strSignalName, tSignalDirection eDirection, handle_t &hSignalHandle) const
    {
        fep::Result nResult = ERR_NOT_FOUND;
        std::map<std::string, tMockSignal>::const_iterator it = m_mapSignals.find(strSignalName);
        if (it != m_mapSignals.end())
        {
            nResult = ERR_NOERROR;
            hSignalHandle = (*it).second.hHandle;
        }
        return nResult;
    }

public:
    void CreateSignal(const char* strName, const handle_t hHandle, const size_t& szSize, const tSignalDirection eDirection)
    {
        std::pair<std::string, tMockSignal> oEntry;
        oEntry.first = strName;
        oEntry.second.hHandle = hHandle;
        oEntry.second.szSize = szSize;
        oEntry.second.eDirection = eDirection;
        m_mapSignals.insert(oEntry);
    }

    void Reset()
    {
        m_mapSignals.clear();
    }

private:
    std::map<std::string, tMockSignal> m_mapSignals;
};

#endif // _FEP_TEST_MY_MOCK_SIGNAL_REGISTRY_H_INC_