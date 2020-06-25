/**
* Implementation of timing client / server element used for integration testing
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

#ifndef _TEST_TIMING_CLIENT_SERVER_H_
#define _TEST_TIMING_CLIENT_SERVER_H_

#include <fep_participant_sdk.h>

class cServerElement : public fep::cModule
{
public:
    cServerElement();
    ~cServerElement();

public: // overwrites state entry listener of cModule
    fep::Result ProcessStartupEntry(const fep::tState eOldState);
    fep::Result ProcessIdleEntry(const fep::tState eOldState);
    fep::Result ProcessInitializingEntry(const fep::tState eOldState);

public:
    int64_t getErrorCount() const
    {
        return m_errorCount;
    }

private:
    void ServRequest(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    static void ServRequest_caller(void* _instance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess)
    {
        reinterpret_cast<cServerElement*>(_instance)->ServRequest(tmSimulation, pStepDataAccess);
    }

private:
    /// handle
    handle_t m_hRequestInput;
    /// handle
    handle_t m_hResponseOutput;
    /// data sample 
    fep::IUserDataSample* m_pSampleRequestInput;
    /// data sample 
    fep::IUserDataSample* m_pSampleResponseOutput;

    int64_t m_errorCount;
};

#endif // _TEST_TIMING_CLIENT_SERVER_H_