/**
 * Implementation of adapted signal mapping mockup used by this test
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

#ifndef _FEP_TEST_MY_MOCK_SIGNAL_MAPPING_H_INC_
#define _FEP_TEST_MY_MOCK_SIGNAL_MAPPING_H_INC_

#include "function/_common/fep_mock_signal_mapping.h"
class cMyMockSignalMappingPrivate : public cMockSignalMappingPrivate
{
private:
    struct cSignalEntry
    {
        std::string strSignalName;
        std::string strSignalType;
    };

public:
    ~cMyMockSignalMappingPrivate()
    {
        MockReset();
    }

public: // implements ISignalMappingPrivate
    virtual fep::Result RegisterSignal(const tSignal& oSignal, handle_t& hHandle)
    {
        cSignalEntry* pSignalEntry = new cSignalEntry();
        pSignalEntry->strSignalName = oSignal.strSignalName;
        pSignalEntry->strSignalType = oSignal.strSignalType;

        hHandle = pSignalEntry;

        m_oSignalMap.insert(std::make_pair(hHandle, pSignalEntry));

        return ERR_NOERROR;
    }

public:
    void MockReset()
    {
        for (std::map<handle_t, cSignalEntry*>::iterator it= m_oSignalMap.begin(); it != m_oSignalMap.end(); ++it)
        {
            delete it->second;
        }
        m_oSignalMap.clear();
    }

private:
    std::map<handle_t, cSignalEntry*> m_oSignalMap;
};

#endif // _FEP_TEST_MY_MOCK_SIGNAL_MAPPING_H_INC_
