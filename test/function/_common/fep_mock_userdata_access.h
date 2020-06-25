/**
* Implementation of data access mockup used by FEP functional test cases!
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

#ifndef __FEP_TEST_MOCK_DATA_ACCESS_H_INC_
#define __FEP_TEST_MOCK_DATA_ACCESS_H_INC_

#include "data_access/fep_user_data_access_intf.h"
#include "data_access/fep_data_access.h"

class cMockDataAccess : public fep::IUserDataAccess, public fep::IUserDataAccessPrivate
{
public:
    cMockDataAccess()
    {

    }
    ~cMockDataAccess()
    {

    }
    virtual fep::Result LockData(handle_t hSignal, const fep::IUserDataSample*& poSample)
    {
        return ERR_NOERROR;
    }
    virtual fep::Result LockDataAt(handle_t hSignal, const fep::IUserDataSample*& poSample,
        timestamp_t tmSimulation, uint32_t eSelectionFlags = SS_NEAREST_SAMPLE)
    {
        return ERR_NOERROR;
    }
    virtual fep::Result RegisterDataListener(IUserDataListener* poDataListener,
        handle_t hSignalHandle)
    {
        return ERR_NOERROR;
    }
    virtual fep::Result TransmitData(fep::IUserDataSample* poSample, bool bSync)
    {
        return ERR_NOERROR;
    }
    virtual fep::Result UnregisterDataListener(IUserDataListener* poDataListener, const handle_t hSignalHandle)
    {
        return ERR_NOERROR;
    }
    virtual fep::Result CreateUserDataSample(IUserDataSample*& pSample,
        const handle_t hSignal = NULL) const
    {
        return ERR_NOERROR;
    }
    virtual fep::Result GetSampleBuffer(handle_t hSignalHandle, cDataSampleBuffer*& pBuffer)
    {
        return ERR_NOERROR;
    }
    virtual fep::Result UnlockData(const fep::IUserDataSample* poSample)
    {
        return ERR_NOERROR;
    }
    virtual fep::Result LockDataAtUpperBound(handle_t hSignalHandle, const fep::IUserDataSample*& poSample, bool& bSampleIsvalid,
        timestamp_t tmSimulationUpperBound)
    {
        return ERR_NOERROR;
    }
};

#endif // __FEP_TEST_MOCK_DATA_ACCESS_H_INC_
