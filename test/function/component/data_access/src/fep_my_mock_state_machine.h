/**
* Implementation of adapted state machine mockup used by this test
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

#ifndef __FEP_MY_MOCK_STATEMACHINE_H_
#define __FEP_MY_MOCK_STATEMACHINE_H_

#include "function/_common/fep_mock_state_machine.h"

class cMyMockStateMachine : public cMockStateMachine
{
public:
    cMyMockStateMachine() : m_bErrorEventReceived(false)
    {

    }
    ~cMyMockStateMachine()
    {

    }
    fep::Result ErrorEvent()
    {
        m_bErrorEventReceived = true;
        return ERR_NOERROR;
    }
public:
    bool m_bErrorEventReceived;
};

#endif // __FEP_MY_MOCK_STATEMACHINE_H_