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

#ifndef _FEP_TEST_MY_MOCK_COMMNAD_ACCESS_H_INC_
#define _FEP_TEST_MY_MOCK_COMMNAD_ACCESS_H_INC_

#include "function/_common/fep_mock_command_access.h"

using namespace fep;

class cMyMockCommandAccess : public cMockCommandAccess
{
public:
    virtual fep::Result TransmitCommand(ICommand* poCommand)
    {
        m_strLastCommand = poCommand->ToString();

        return ERR_NOERROR;
    }

public:
    const char* GetLastCommand() const
    {
        return m_strLastCommand.c_str();
    }

public:
    std::string m_strLastCommand;
};

#endif // _FEP_TEST_MY_MOCK_COMMNAD_ACCESS_H_INC_
