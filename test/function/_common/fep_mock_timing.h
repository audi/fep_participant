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

#ifndef _FEP_TEST_MOCK_TIMING_H_INC_
#define _FEP_TEST_MOCK_TIMING_H_INC_

class cMockTiming : public fep::ITiming
{
public:
    cMockTiming() {} 
    virtual ~cMockTiming(){}
public:
    virtual timestamp_t GetTime() const
    {
        return 0;
    }

    virtual fep::Result SetSystemTimeout(const timestamp_t tmSystemTimeout)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result RegisterStepListener(const char* name, const StepConfig& configuration, ScheduleFunc callback, void* callee)
    {
        return ERR_NOERROR;
    }

    virtual fep::Result UnregisterStepListener(const char* name)
    {
        return ERR_NOERROR;
    }

};

#endif // _FEP_TEST_MOCK_TIMING_H_INC_
