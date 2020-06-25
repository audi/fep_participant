/**
 * Implementation of adapted tx adapter mockup used by this test
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

#ifndef _FEP_TEST_MY_MOCK_TX_ADAPTER_H_INC_
#define _FEP_TEST_MY_MOCK_TX_ADAPTER_H_INC_

using namespace fep;

#include "function/_common/fep_mock_tx_adapter.h"

class cMyMockTxAdapter : public cMockTxAdapter
{
public:
    cMyMockTxAdapter()
        : m_pRecentSample(NULL)
    {
    }

    ~cMyMockTxAdapter()
    {
        if (m_pRecentSample)
        {
            //delete m_pRecentSample;
        }
    }

public: // implements IPreparationDataAccess
    virtual fep::Result GetRecentSample(handle_t hSignalHandle, fep::IPreparationDataSample* pSample) const
    {
        if (!m_pRecentSample)
        {
            return ERR_UNEXPECTED;
        }
        m_pRecentSample->CopyTo(*pSample);
        return ERR_NOERROR;
    }

public:
    void MockReset()
    {
        m_pRecentSample = NULL;
    }

    void MockSetRecentSample(fep::IUserDataSample* pSample)
    {
        if (m_pRecentSample)
        {
            //delete m_pRecentSample;
            m_pRecentSample = NULL;
        }
        m_pRecentSample = dynamic_cast<fep::IPreparationDataSample*>(pSample);
        ASSERT_NE(reinterpret_cast<fep::IPreparationDataSample*>(NULL), m_pRecentSample);
    }

private:
    fep::IPreparationDataSample* m_pRecentSample;
};

#endif // _FEP_TEST_MY_MOCK_TX_ADAPTER_H_INC_