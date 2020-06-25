/**
 * Implementation of timing mockup used by FEP functional test cases!
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

#ifndef _FEP_TEST_MOCK_TIMING_CLIENT_PRIVATE_H_INC_
#define _FEP_TEST_MOCK_TIMING_CLIENT_PRIVATE_H_INC_

#include "fep3/components/legacy/timing/locked_step_legacy/timing_client.h"

class cMockTimingClientPrivate : public fep::timing::ITimingClientPrivate
{
public:
    cMockTimingClientPrivate() {}
    virtual ~cMockTimingClientPrivate(){}
public:
    void FillScheduleList(cScheduleList& scheduleList)
    {
        // Do nothing
    }

};

#endif // _FEP_TEST_MOCK_TIMING_CLIENT_PRIVATE_H_INC_
