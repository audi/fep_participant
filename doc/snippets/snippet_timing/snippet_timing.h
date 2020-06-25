/**
*  Declaration of an exemplary FEP Timing Client Participant
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

#ifndef __TIMING_SNIPPET_H__
#define __TIMING_SNIPPET_H__ 

#include <fep_participant_sdk.h>

class cTimingSnippetElement : public fep::cModule
{
public:
    fep::Result ProcessStartupEntry(const fep::tState eOldState);
    fep::Result ProcessInitializingEntry(const fep::tState eOldState);
    fep::Result ProcessIdleEntry(const fep::tState eOldState);

public:
    //! [CallbackFunctionPattern]
    void SimpleListenerCallbackFunc(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    static void SimpleListenerCallbackFunc_caller(void* pInstance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    void ConfiguredListenerCallbackFunc(timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    static void ConfiguredListenerCallbackFunc_caller(void* pInstance, timestamp_t tmSimulation, fep::IStepDataAccess* pStepDataAccess);
    //! [CallbackFunctionPattern]

private:
    handle_t m_hInputSignal;
    handle_t m_hOutputSignal;
    fep::IUserDataSample* pInputSample;
    fep::IUserDataSample* pOutputSample;
    
};

#endif // __TIMING_SNIPPET_H__