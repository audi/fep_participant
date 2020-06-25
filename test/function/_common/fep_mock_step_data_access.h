/**
 * Implementation of step data access mockup used by FEP functional test cases!
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
 
#ifndef _FEP_TEST_MOCK_STEPDATAACCESS_H_INC_
#define _FEP_TEST_MOCK_STEPDATAACCESS_H_INC_

class cMockStepDataAccess : public IStepDataAccess , public IStepDataAccessPrivate
{
public:
    cMockStepDataAccess() { }
    virtual ~cMockStepDataAccess() { }
    virtual fep::Result CopyRecentData(handle_t hSignalHandle, IUserDataSample*& poSample)
    {
        return ERR_NOERROR;
    }
    virtual fep::Result CopyDataBefore(handle_t hSignalHandle, timestamp_t tmSimTime, IUserDataSample*& poSample)
    {
        return ERR_NOERROR;
    }
    virtual Result TransmitData(IUserDataSample* poSample)
    {
        return ERR_NOERROR;
    }
    virtual fep::Result ConfigureInput(const std::string& strName, const timing::InputConfig& oInputConfig)
    {
        return ERR_NOERROR;
    }
    virtual fep::Result ConfigureOutput(const std::string& strName, const timing::OutputConfig& oOutputConfig)
    {
        return ERR_NOERROR;
    }
    virtual fep::Result Clear()
    {
        return ERR_NOERROR;
    }
    virtual inline void SetCycleTime(timestamp_t tmCycleTime)
    {

    }
    virtual inline void SetWaitTimeForInputs(timestamp_t tmWaitTime)
    {

    }
    virtual fep::Result ValidateInputs(timestamp_t tmCurrSimTime, a_util::concurrency::semaphore& oSem)
    {
        return ERR_NOERROR;
    }
    virtual fep::Result Transmit()
    {
        return ERR_NOERROR;
    }
    virtual inline void SetSkip()
    {

    }
};

#endif // _FEP_TEST_MOCK_STEPDATAACCESS_H_INC_