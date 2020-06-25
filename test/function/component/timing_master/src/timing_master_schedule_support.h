/*
*
* Implementation of the testfixture for the stm test
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

#ifndef _TESTER_TIMING_MASTER_SUPPORT_H_INC_
#define _TESTER_TIMING_MASTER_SUPPORT_H_INC_



#include  "components/timing_legacy/common_timing.h"
#include  "components/timing_legacy/schedule_map.h"
#include  "components/timing_legacy/locked_step_legacy/_master.h"

#include "messages/fep_notification_schedule.h"
#include "transmission_adapter/fep_data_sample.h"
#include "_common/fep_schedule_list.h"
#include "_common/fep_timestamp.h"

#include <a_util/system.h>


using namespace fep;
using namespace fep::timing;

static ScheduleConfig make_Schedule(const std::string& step_uuid, const timestamp_t cycle_time)
{
    ScheduleConfig schedule;

    schedule._step_uuid = step_uuid;
    schedule._cycle_time_us = cycle_time;

    return schedule;
}

static void fillAckDataSample(cDataSample& oDataSample, const TimingMaster& oTM, const std::string& step_uuid)
{
    oDataSample.SetFrameId(0);
    oDataSample.SetSampleNumberInFrame(0);
    oDataSample.SetSyncFlag(true);
    oDataSample.SetSignalHandle(oTM.GetAckInputSignalHandle());
    oDataSample.SetSize(sizeof(TriggerAck));
    TriggerAck* pAck = reinterpret_cast<TriggerAck*>(oDataSample.GetPtr());
    pAck->currSimTime = 0;
    pAck->operationalTime = 1;
    memcpy(pAck->uuid_str, step_uuid.c_str(), 36);
}

class TestCallbackClass : public IStepTriggerStrategyListener
{
public:
    Result InternalTrigger(const timestamp_t nCurrentSimulationTime)
    {
        received_ticks.push_back(nCurrentSimulationTime);
        return ERR_NOERROR;
    }

public:
    std::list<timestamp_t> received_ticks;
};

template <class STEP_TRIGGER> class TestCallbackClassSendAck : public IStepTriggerListener
{
public:
    TestCallbackClassSendAck(STEP_TRIGGER* step_trigger)
        : _step_trigger(step_trigger)
    {
    }

public:
    Result Trigger(const timestamp_t nCurrentSimulationTime)
    {
        received_ticks.push_back(nCurrentSimulationTime);

        
        return ERR_NOERROR;
    }

public:
    std::list<timestamp_t> received_ticks;
    STEP_TRIGGER* _step_trigger;
};



#endif // _TESTER_TIMING_MASTER_SUPPORT_H_INC_
