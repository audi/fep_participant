/**
 * Implementation of state machine mockup used by FEP functional test cases!
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

#ifndef _FEP_TEST_MOCK_STATEMACHINE_H_INC_
#define _FEP_TEST_MOCK_STATEMACHINE_H_INC_

class cMockStateMachine : public fep::IStateMachine
{
public:
    cMockStateMachine() { }
    virtual ~cMockStateMachine () { }
    virtual fep::Result StartupDoneEvent() 
    { 
      return ERR_NOERROR; 
    }
    virtual fep::Result InitializeEvent() 
    { 
      return ERR_NOERROR; 
    }
    virtual fep::Result InitDoneEvent() 
    { 
      return ERR_NOERROR;
    }
    virtual fep::Result StartEvent() 
    { 
      return ERR_NOERROR; 
    }
    virtual fep::Result StopEvent() 
    { 
      return ERR_NOERROR; 
    }
    virtual fep::Result ErrorEvent() 
    { 
      return ERR_NOERROR; 
    }
    virtual fep::Result ErrorFixedEvent() 
    { 
      return ERR_NOERROR; 
    }
    virtual fep::Result RestartEvent() 
    { 
      return ERR_NOERROR; 
    }
    virtual fep::Result ShutdownEvent() 
    { 
      return ERR_NOERROR; 
    }
    virtual fep::Result RegisterStateEntryListener(fep::IStateEntryListener* const poListener)
    {
      return ERR_NOERROR;
    }
    virtual fep::Result UnregisterStateEntryListener(fep::IStateEntryListener* const poListener)
    {
      return ERR_NOERROR;
    }
    virtual fep::Result RegisterStateExitListener(fep::IStateExitListener* const poListener)
    {
      return ERR_NOERROR;
    }
    virtual fep::Result UnregisterStateExitListener(fep::IStateExitListener* const poListener)
    {
      return ERR_NOERROR;
    }
    virtual fep::Result RegisterStateRequestListener(fep::IStateRequestListener* const poListener)
    {
      return ERR_NOERROR;
    }
    virtual fep::Result UnregisterStateRequestListener(fep::IStateRequestListener* const poListener)
    {
      return ERR_NOERROR;
    }
    virtual fep::Result RegisterStateCyclicListener(fep::IStateCyclicListener * const poListener,
                timestamp_t tmCyclePeriod)
    {
      return ERR_NOERROR;
    }
    virtual fep::Result UnregisterStateCyclicListener(fep::IStateCyclicListener * const poListener)
    {
      return ERR_NOERROR;
    }
    virtual fep::tState GetState() const 
    { 
      return fep::FS_UNKNOWN;
    }
    virtual fep::Result GetRemoteState(const char * strElementName,
                fep::tState & eState, timestamp_t tmTimeout) 
    { 
      return ERR_NOERROR; 
    }
    virtual fep::Result TriggerRemoteEvent(const fep::tControlEvent eEvent,
                const char * strElementName) 
    { 
      return ERR_NOERROR;
    }
    fep::Result WaitForState(const tState eState, const timestamp_t tmTimeout, bool error_is_failure) const
    {
        return ERR_NOERROR;
    }
};

#endif // _FEP_TEST_MOCK_STATEMACHINE_H_INC_
