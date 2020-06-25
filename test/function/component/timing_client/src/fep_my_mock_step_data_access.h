/**
* Implementation of adapted step data access mockup used by this test
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

#ifndef __FEP_MY_MOCK_STEP_DATA_ACCESS_H_
#define __FEP_MY_MOCK_STEP_DATA_ACCESS_H_

#include "function/_common/fep_mock_step_data_access.h"

class cMyMockStepDataAccess : public cMockStepDataAccess
{
public:
    cMyMockStepDataAccess()
        : m_tmCycle(0)
        , m_tmWait(0)
        , m_bTransmitReceived(false)
        , m_bSkipReceived(false)
    {

    }
    ~cMyMockStepDataAccess()
    {

    }
    void SetCycleTime(timestamp_t tmCycle)
    {
        m_tmCycle = tmCycle;
    }
    void SetWaitTimeForInputs(timestamp_t tmWait)
    {
        m_tmWait = tmWait;
    }
    fep::Result ValidateInputs(timestamp_t tmCurrSimTime, const bool validate_inputs)
    {
        return ERR_NOERROR;
    }
    fep::Result TransmitAllOutputs()
    {
        m_bTransmitReceived = true;
        return ERR_NOERROR;
    }
    void SetSkip()
    {
        m_bSkipReceived = true;
    }

public:
    timestamp_t m_tmCycle;
    timestamp_t m_tmWait;
    bool m_bTransmitReceived;
    bool m_bSkipReceived;
};

#endif // __FEP_MY_MOCK_STEP_DATA_ACCESS_H_