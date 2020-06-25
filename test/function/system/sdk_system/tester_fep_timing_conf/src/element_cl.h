/**
* Implementation of timing client / client element used for integration testing
*

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
* @file
*
*/

#ifndef _TEST_TIMING_CLIENT_CLIENT_H_
#define _TEST_TIMING_CLIENT_CLIENT_H_

#include <fep_participant_sdk.h>

class cClientElement : public fep::cModule
{
public:
    cClientElement();
    ~cClientElement();

public: // overwrites state entry listener of cModule
    fep::Result ProcessStartupEntry(const fep::tState eOldState);
    fep::Result ProcessIdleEntry(const fep::tState eOldState);
    fep::Result ProcessInitializingEntry(const fep::tState eOldState);

private:
    void SendRequest(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    static void SendRequest_caller(void* _instance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<cClientElement*>(_instance)->SendRequest(tmSimulation, pStepDataAccess);
    }

    void ProcessResponse(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    static void ProcessResponse_caller(void* _instance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<cClientElement*>(_instance)->ProcessResponse(tmSimulation, pStepDataAccess);
    }

public: // Test Support
    int64_t getCount() const
    {
        return m_nExpectedResponseValue - 1;
    }

    int64_t getStepCount() const
    {
        return m_nNumberOfSteps;
    }

    int64_t getErrorCount() const
    {
        return m_errorCount;
    }

private:
    /// handle
    handle_t m_hRequestOutput;
    /// handle
    handle_t m_hResponseInput;
    /// data sample 
    fep::IUserDataSample* m_pSampleRequestOutput;
    /// data sample 
    fep::IUserDataSample* m_pSampleResponseInput;

    int64_t m_nNextRequestValue;
    int64_t m_nExpectedResponseValue;
    int64_t m_nNumberOfSteps;
    int64_t m_errorCount;
};

#endif // __EXAMPLE_TIMING_ELEMENT_DRIVER_H__