/**
* Implementation of the Class TimingMaster.
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

#include "timing_master.h"
#include <cstddef>
#include <string>
#include <a_util/base/detail/delegate_decl.h>
#include <a_util/base/detail/delegate_impl.h>
#include <a_util/concurrency/chrono.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_functions.h>
#include <a_util/system/detail/timer_impl.h>
#include <a_util/system/system.h>
#include "step_trigger_strategy.h"
#include "timing_client.h"
#include "_common/fep_optional.h"
#include "_common/fep_schedule_list.h"
#include "_common/fep_schedule_list_intf.h"
#include "_common/fep_timestamp.h"
#include "data_access/fep_data_access.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/propertytreebase_intf.h"
#include "fep3/components/legacy/timing/common_timing.h"
#include "fep3/components/legacy/timing/locked_step_legacy/schedule_map.h"
#include "fep3/components/legacy/timing/locked_step_legacy/step_trigger_strategy_intf.h"
#include "fep_errors.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "messages/fep_command_get_schedule.h"
#include "messages/fep_notification_schedule_intf.h"
#include "signal_registry/fep_signal_struct.h"
#include "statemachine/fep_statemachine_intf.h"
#include "transmission_adapter/fep_data_sample.h"
#include "transmission_adapter/fep_signal_direction.h"
#include "transmission_adapter/fep_signal_serialization.h"
#include "transmission_adapter/fep_transmission.h"
#include "transmission_adapter/fep_user_data_sample_intf.h"

namespace fep
{
class IStepTrigger;
}

// For Debug purposes only
//#include <iostream>
//#define DBG_ONLY(x) x
#define DBG_ONLY(x) 

const char* const fep::timing::g_stepTriggerSignalType = "tFEP_StepTrigger";
const char* const fep::timing::g_stepTriggerSignalName = "_StepTrigger";

using namespace fep;
using namespace fep::timing;


#define INVOKE_INCIDENT(Handler,Code,Serv,Descr)\
    Handler->InvokeIncident(Code,Serv,Descr,"TimingMaster",0, NULL)

static const int64_t watchdog_period = 100 * 1000;

TimingMaster::TimingMaster()
    : _transmission_adapter_private(nullptr)
    , _property_tree_base(nullptr)
    , _timing_client_private(nullptr)
    , _is_timing_master_flag(false)
    , _is_initial_schedule_flag(false)
    , _schedule_map()
    , _current_time(-1)
    , _schedule_configs()
    , _trigger_output_signal_handle(nullptr)
    , _trigger_data_sample()
    , _ack_input_signal_handle(nullptr)
    , _step_trigger_strategy(nullptr)
    , _user_step_trigger(nullptr)
    , _worker_thread()
    , _shutdown_worker_semaphore()
    , _completition_barrier()
    , _tick_barrier()
    , _last_acknowledge_received_timestamp(0)
    , _acknowledge_wait_timeout(1000000 * 10)
    , _acknowledge_wait_timer()
{
}

TimingMaster::~TimingMaster()
{
}

fep::Result TimingMaster::initialize(IUserDataAccessPrivate* data_access_private,
    ITransmissionAdapterPrivate* transmission_adapter_private,
    IIncidentHandler* incident_handler,
    IPropertyTreeBase* property_tree_base,
    ITimingClientPrivate* timing_client_private,
    IStateMachine* state_machine)
{
    fep::Result nRes = ERR_NOERROR;

    if (fep::isOk(nRes))
    {
        if (!data_access_private || !transmission_adapter_private || !incident_handler || !property_tree_base || !timing_client_private || !state_machine)
        {
            nRes = ERR_INVALID_ARG;
        }
    }

    if (fep::isOk(nRes))
    {
        _data_access_private = data_access_private;
        _transmission_adapter_private = transmission_adapter_private;
        _incident_handler = incident_handler;
        _property_tree_base = property_tree_base;
        _timing_client_private = timing_client_private;
        _state_machine = state_machine;
    }

    if (fep::isOk(nRes))
    {
        nRes = _transmission_adapter_private->RegisterNotificationListener(this);
    }

  //  if (fep::isOk(nRes))
  //  {
        // set default values (even after a restart)
   //     _property_tree_base->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, "");

        // Default is "Internal" clock with factor 1.0 and 1 second acknowledgement wait timeout
  //      _property_tree_base->SetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, "SYSTEM_TIME");
   //     _property_tree_base->SetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, 1.0);
  //      _property_tree_base->SetPropertyValue(FEP_TIMING_MASTER_CLIENT_TIMEOUT, 10); // 10 s
  //      _property_tree_base->SetPropertyValue(FEP_TIMING_MASTER_MIN_TRIGGER_TIME, 100); // 100 ms
  //  }

    return nRes;
}


fep::Result TimingMaster::finalize()
{
    fep::Result nRes = ERR_NOERROR;

    if (fep::isOk(nRes) && _transmission_adapter_private)
    {
        _transmission_adapter_private->UnregisterNotificationListener(this);
    }

    if (fep::isOk(nRes))
    {
        _data_access_private = nullptr;
        _transmission_adapter_private = nullptr;
        _incident_handler = nullptr;
        _property_tree_base = nullptr;
        _timing_client_private = nullptr;
        _state_machine = nullptr;
    }

    return nRes;
}

fep::Result TimingMaster::configure()
{
    fep::Result result = ERR_NOERROR;

    // Clear things
    _schedule_configs.clear();

    // Get timing master name
    const char* strMasterElement = NULL;
    if (fep::isFailed(_property_tree_base->GetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, strMasterElement)))
    {
        return ERR_NOERROR;
    }

    // Get own name
    const char* strElementName;
    if (fep::isFailed(_property_tree_base->GetPropertyValue(
        fep::g_strElementHeaderPath_strElementName, strElementName)))
    {
        result = ERR_FAILED;
    }

    // Am I the Timingmaster
    _is_timing_master_flag = a_util::strings::getLength(strMasterElement) > 0 && a_util::strings::isEqual(strMasterElement, strElementName);

    // Only if master
    if (_is_timing_master_flag)
    {
        // Get trigger mode
        const char* strTriggerMode = nullptr;
        if (fep::isFailed(_property_tree_base->GetPropertyValue(FEP_TIMING_MASTER_TRIGGER_MODE, strTriggerMode)))
        {
            // No trigger mode
            strTriggerMode = nullptr;
        }

        // Decide on trigger mode
        if (!strTriggerMode || !strTriggerMode[0])
        {
            result = fep::Result(ERR_UNEXPECTED, "TriggerMode is required, but it is unset or empty.", __LINE__, __FILE__, __FUNCTION__);
        }
        else if (a_util::strings::isEqual(strTriggerMode, "AFAP"))
        {
            cAFAPStepTriggerStrategy* pAFAPStepTrigger = new cAFAPStepTriggerStrategy();
            _step_trigger_strategy = pAFAPStepTrigger;
            int32_t prop_readout;
            if (fep::isOk(_property_tree_base->GetPropertyValue(FEP_TIMING_MASTER_CLIENT_TIMEOUT, prop_readout)))
            {
                if (prop_readout > 0)
                {
                    _acknowledge_wait_timeout = prop_readout * 1000 * 1000;
                    _acknowledge_wait_timer.setPeriod(static_cast<uint64_t>(watchdog_period));
                    _acknowledge_wait_timer.setCallback(&TimingMaster::CheckAckTimeout, *this);
                }
                else
                {
                    result = fep::Result(ERR_INVALID_ARG, "Acknowledgement wait timeout is set to invalid value.", __LINE__, __FILE__, __FUNCTION__);
                }
            }
        }
        else if (a_util::strings::isEqual(strTriggerMode, "SYSTEM_TIME"))
        {
            cInternalStepTriggerStrategy* pInternalStepTrigger = new cInternalStepTriggerStrategy();
            pInternalStepTrigger->initialize(_property_tree_base);
            _step_trigger_strategy = pInternalStepTrigger;
            int32_t prop_readout;
            if (fep::isOk(_property_tree_base->GetPropertyValue(FEP_TIMING_MASTER_CLIENT_TIMEOUT, prop_readout)))
            {
                if (prop_readout > 0)
                {
                    _acknowledge_wait_timeout = prop_readout * 1000 * 1000;
                    _acknowledge_wait_timer.setPeriod(static_cast<uint64_t>(watchdog_period));
                    _acknowledge_wait_timer.setCallback(&TimingMaster::CheckAckTimeout, *this);
                }
                else
                {
                    result = fep::Result(ERR_INVALID_ARG, "Acknowledgement wait timeout is set to invalid value.", __LINE__, __FILE__, __FUNCTION__);
                }
            }
        }
        else if (a_util::strings::isEqual(strTriggerMode, "USER_IMPLEMENTATION"))
        {
            // User trigger mode -> There must be an m_pStepTrigger
            if (_user_step_trigger)
            {
                cUserStepTriggerStrategy* pUserStepTrigger = new cUserStepTriggerStrategy();
                pUserStepTrigger->initialize(_user_step_trigger);
                _step_trigger_strategy = pUserStepTrigger;
            }
            else 
            {
                result = fep::Result(ERR_UNEXPECTED, "TriggerMode was set to \"USER_IMPLEMENTATION\", but no trigger class was set.", __LINE__, __FILE__, __FUNCTION__);
            }
        }
        else if (a_util::strings::isEqual(strTriggerMode, "EXTERNAL_CLOCK"))
        {
            cExternalStepTriggerStrategy* pExternalStepTrigger = new cExternalStepTriggerStrategy();
            pExternalStepTrigger->initialize(_data_access_private, _transmission_adapter_private);
            _step_trigger_strategy = pExternalStepTrigger;
        }        
        else
        {
            // Invalid configuration value
            result = fep::Result(ERR_UNEXPECTED, "TriggerMode has invalid value.", __LINE__, __FILE__, __FUNCTION__);
        }

        if (fep::isOk(result))
        {
            if (!_step_trigger_strategy->NeedToWaitForTrigger() && !_step_trigger_strategy->NeedToWaitForCompletition())
            {
                // This combination makes no sense ... has to 
                result = fep::Result(ERR_UNEXPECTED, "Trigger has invalid combination (Neither waiting for ticks nor for completition).", __LINE__, __FILE__, __FUNCTION__);
            }
        }

        if (fep::isOk(result))
        {
            // Get schedule config: Send command
            if (fep::isOk(result))
            {
                cGetScheduleCommand oCmd(strElementName, "*", GetTimeStampMicrosecondsUTC(), 0);
                // transmit control command
                if (fep::isFailed(_transmission_adapter_private->TransmitCommand(&oCmd)))
                {
                    result = ERR_FAILED;
                }
            }

            if (fep::isOk(result) && _step_trigger_strategy->NeedToWaitForCompletition())
            {
                // Ack signal is only needed if there is no step trigger cllass 
                tSignal oSignal;
                oSignal.strSignalName = timing::s_trigger_ack_signal_name;
                oSignal.szSampleSize = timing::s_trigger_ack_signal_size;
                oSignal.bIsReliable = true;
                oSignal.bIsRaw = true;
                oSignal.eSerialization = SER_Raw;
                oSignal.eDirection = fep::SD_Input;
                oSignal.bIsMapped = false;
                oSignal.strSignalDesc = "";
                oSignal.szSampleBacklog = 0;
                oSignal.strSignalType = "";         
                oSignal.bRTIAsyncPub = false;
                oSignal.bRTILowLat = true;
                oSignal.strRTIMulticast = std::string("");

                if (fep::isOk(result))
                {
                    result = _transmission_adapter_private->RegisterSignal(oSignal, _ack_input_signal_handle);
                }

                if (fep::isOk(result))
                {
                    result = _data_access_private->RegisterDataListener(this, _ack_input_signal_handle);
                }
            }

            if (fep::isOk(result))
            {
                tSignal oSignal;
                oSignal.strSignalName = timing::s_trigger_tick_signal_name;
                oSignal.szSampleSize = timing::s_trigger_tick_signal_size;
                oSignal.bIsReliable = true;
                oSignal.bIsRaw = true;
                oSignal.eSerialization = SER_Raw;
                oSignal.eDirection = fep::SD_Output;
                oSignal.bIsMapped = false;
                oSignal.strSignalDesc = "";
                oSignal.szSampleBacklog = 0;
                oSignal.strSignalType = "";
                oSignal.bRTIAsyncPub = false;
                oSignal.bRTILowLat = true;
                oSignal.strRTIMulticast = std::string("");

                if (fep::isOk(result))
                {
                    result = _transmission_adapter_private->RegisterSignal(oSignal, _trigger_output_signal_handle);

                    if (fep::isOk(result))
                    {
                        _trigger_data_sample.SetFrameId(0);
                        _trigger_data_sample.SetSampleNumberInFrame(0);
                        _trigger_data_sample.SetSyncFlag(true);
                        _trigger_data_sample.SetSignalHandle(_trigger_output_signal_handle);
                        _trigger_data_sample.SetSize(sizeof(TriggerTick));
                    }
                }
            }
        }
    }

    return result;
}

fep::Result TimingMaster::reset()
{
    fep::Result result = ERR_NOERROR;

    if (_is_timing_master_flag)
    {
        // Delete the step trigger
        if (_step_trigger_strategy)
        {
            delete _step_trigger_strategy;
            _step_trigger_strategy = nullptr;
        }

        _acknowledge_wait_timer.setPeriod(0);
        _last_acknowledge_received_timestamp = 0;

        // Unregister Signals
        if (fep::isOk(result))
        {
            result = _transmission_adapter_private->UnregisterSignal(_trigger_output_signal_handle);
        }

        if (fep::isOk(result) && _ack_input_signal_handle)
        {
            if (fep::isOk(result))
            {
                result = _data_access_private->UnregisterDataListener(this, _ack_input_signal_handle);
            }

            if (fep::isOk(result))
            {
                result = _transmission_adapter_private->UnregisterSignal(_ack_input_signal_handle);
            }
        }
    }

    return result;
}

Result TimingMaster::start()
{
    Result result = ERR_NOERROR;

    if (_is_timing_master_flag)
    {
        _schedule_map.reset();

        // Add schedule from own schedule client
        {
            cScheduleList scheduleList;
            _timing_client_private->FillScheduleList(scheduleList);
            for (size_t i = 0; i < scheduleList.GetListSize(); ++i)
            {
                const ScheduleConfig* pSchedule = scheduleList.GetScheduleAt(i);
                _schedule_configs.insert(*pSchedule);
            }
        }

        // Add additional schedule
        int32_t prop_readout;
        if (fep::isOk(_property_tree_base->GetPropertyValue(FEP_TIMING_MASTER_MIN_TRIGGER_TIME, prop_readout)))
        {
            if (prop_readout > 0)
            {
                if (_step_trigger_strategy->HasSupportDummyTimeTrigger())
                {
                    const ScheduleConfig schedule_config("", prop_readout * 1000);
                    _schedule_configs.insert(schedule_config);
                }
                else
                {
                    // Not supported. No notification is done.
                }
            }
        }

        // Check if configuration of the schedule map succeeded
        if (!_schedule_map.configure(_schedule_configs)) {

            // Invoke an incident because the resulting schedule map would consume a lot of memory
            INVOKE_INCIDENT(_incident_handler, FSI_GENERAL_CRITICAL_GLOBAL_FAILURE, SL_Critical_Global,
                "ERROR: Schedule map will not be computed because the resulting lcm and gcd values of the given timing frequencies will be very large. Small frequencies should be rounded up or down.");

            // Go to error state
            _state_machine->ErrorEvent();

            return fep::ERR_FAILED;
        }
        DBG_ONLY(_schedule_map.printToStream(std::cerr)); 

        // If need to wait for tick is active: -1 marks no tick was received yet
        if (_step_trigger_strategy->NeedToWaitForTrigger())
        {
            _current_time = -1;
        }
        else
        {
            _current_time = 0;
        }

        // Mark initial schedule
        _is_initial_schedule_flag = true;

        // Reset duration
        _duration_since_last_tick = 0;

        // Register the trigger
        result = _step_trigger_strategy->RegisterStrategyTrigger(_schedule_map.getCycleTime(), this);
        if (isOk(result))
        {
            //std::cerr << "ScheduleMap: "; m_oScheduleMap.printToStream(std::cerr);

            // create the worker thread
            _worker_thread.reset(new a_util::concurrency::thread(&TimingMaster::ThreadFunc, this));

            result = _step_trigger_strategy->Start();
        }

        if (isOk(result))
        {
            // check whether we need to start ack wait timer (i.e. period is not equal 0)
            if (_acknowledge_wait_timer.getPeriod() != 0)
            {
                _acknowledge_wait_timer.start();
            }
        }
    }

    return result;
}

fep::Result TimingMaster::stop()
{
    fep::Result nRes = ERR_NOERROR;

    if (_is_timing_master_flag)
    {
        if (_step_trigger_strategy)
        {
            nRes = _step_trigger_strategy->Stop();
        }

        if (_acknowledge_wait_timer.isRunning())
        {
            _acknowledge_wait_timer.stop();
        }
        
        if (_worker_thread)
        {
            _shutdown_worker_semaphore.notify();
            _worker_thread->join();
            _worker_thread.reset();
            _shutdown_worker_semaphore.reset();
        }
    }

    return nRes;
}

fep::Result TimingMaster::sendTickSignalToClients()
{
    TriggerTick* pTrigger= reinterpret_cast<TriggerTick*>(_trigger_data_sample.GetPtr());
    pTrigger->simTimeStep = _duration_since_last_tick; 
    pTrigger->currentTime = _current_time;

    DBG_ONLY(std::cerr << "TimingMaster::SendTrigger(): currentTime=" << pTrigger->currentTime << " simTimeStep = " << pTrigger->simTimeStep << std::endl);


    // Before sending: convert timestamp
    convertTriggerTickToNetworkByteorder(*pTrigger);
    return _transmission_adapter_private->TransmitData(&_trigger_data_sample);
}

fep::Result TimingMaster::SetStepTrigger(IStepTrigger* pUserStepTrigger)
{
    fep::Result nRes = ERR_NOERROR;
    
    _user_step_trigger = pUserStepTrigger;
 
    return nRes;
}

fep::Result TimingMaster::Update(const IUserDataSample* poSample)
{
    fep::Result nRes = ERR_NOERROR;

    if (poSample && poSample->GetSignalHandle() == _ack_input_signal_handle && poSample->GetSize() == sizeof(TriggerAck))
    {
        int64_t old_expected = _last_acknowledge_received_timestamp;
        while (!_last_acknowledge_received_timestamp.compare_exchange_weak(old_expected, 0))
        {
            old_expected = _last_acknowledge_received_timestamp;
        }

        TriggerAck* pAck = reinterpret_cast<TriggerAck*>(poSample->GetPtr());
        convertTriggerAckToHostByteorder(*pAck);

        std::string uuid_str(pAck->uuid_str, 36);

       // std::cout << pAck->uuid_str << " received ack - time ----" << _current_time << "---" << std::endl;
        DBG_ONLY(std::cerr << "TimingMaster::Update: " << "" << uuid_str << ": " << " Ack for " << _current_time << std::endl);


        if (_schedule_map.markStepForCurrentSchedule(uuid_str))
        {
            _completition_barrier.notify();
        }
    }

    return nRes;
}

fep::Result TimingMaster::Update(IScheduleNotification const * pNotification)
{
    IScheduleList* poScheduleList;
    fep::Result nRes = const_cast<IScheduleNotification*>(pNotification)->TakeScheduleList(poScheduleList);

    if (fep::isOk(nRes))
    {
        //std::cerr << "IScheduleNotification: " << pNotification->ToString() << std::endl;
        for (size_t i = 0; i < poScheduleList->GetListSize(); ++i)
        {
            const ScheduleConfig* pSchedule= poScheduleList->GetScheduleAt(i);
            _schedule_configs.insert(*pSchedule);
        }

        delete poScheduleList;
    }

    return nRes;
}

bool TimingMaster::DoNextSchedule()
{
    // Do not increment if first schedule
    if (_is_initial_schedule_flag)
    {
        _is_initial_schedule_flag = false;
    }
    else
    {
        _schedule_map.incrementCurrentSchedule();
    }

    bool need_to_wait_for_step_complete = _schedule_map.isStepInCurrentSchedule();

    if (need_to_wait_for_step_complete)
    {
        need_to_wait_for_step_complete = _schedule_map.isConfiguredStepInCurrentSchedule();
        sendTickSignalToClients();
        _duration_since_last_tick = 0;
    }
    
    _duration_since_last_tick += _schedule_map.getCycleTime();
    _current_time += _schedule_map.getCycleTime();

    return need_to_wait_for_step_complete;
}

void TimingMaster::ThreadFunc()
{
    static const a_util::chrono::microseconds wait_time(100000);

    a_util::system::HighResSchedulingSupport highres;

    if (_step_trigger_strategy->NeedToWaitForCompletition() && _step_trigger_strategy->NeedToWaitForTrigger())
    {
        bool need_to_wait_for_step_complete = false;

        // Waiting for Trigger and Completition (Internal Mode, but not AFAP)
        while (!_shutdown_worker_semaphore.is_set())
        {
            if (need_to_wait_for_step_complete)
            {
                if (_completition_barrier.wait_for(wait_time))
                {
                    if (_schedule_map.isCurrentScheduleComplete())
                    {
                        while (!_shutdown_worker_semaphore.is_set())
                        {
                            if (_tick_barrier.wait_for(wait_time))
                            {
                                need_to_wait_for_step_complete = DoNextSchedule();
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                if (_tick_barrier.wait_for(wait_time))
                {
                    need_to_wait_for_step_complete = DoNextSchedule();
                }
            }    
        }
    }
    else if (_step_trigger_strategy->NeedToWaitForCompletition())
    {
        bool need_to_wait_for_step_complete = false;

        // Waiting for Completition, but not for Trigger (Internal Mode using AFAP)
        while (!_shutdown_worker_semaphore.is_set())
        {
            if (need_to_wait_for_step_complete)
            {
                if (_completition_barrier.wait_for(wait_time))
                {
                    if (_schedule_map.isCurrentScheduleComplete())
                    {
                        need_to_wait_for_step_complete = DoNextSchedule();
                    }
                }
            }
            else
            {
                need_to_wait_for_step_complete = DoNextSchedule(); 
            }
        }
    }
    else if (_step_trigger_strategy->NeedToWaitForTrigger())
    {
        // Only trigger (User Mode, External Mode)
        while (!_shutdown_worker_semaphore.is_set())
        {
            if (_tick_barrier.wait_for(wait_time))
            {
                DoNextSchedule();
            }
        }
    }
    else
    {
        // Invalid mode. Check and Error is already done in "configure"   
    }
}

fep::Result TimingMaster::InternalTrigger(const timestamp_t triggerTime)
{
    // Take initial time
    if (_current_time < 0)
    {
        _current_time = triggerTime;
    }
    _tick_barrier.notify();

    return ERR_NOERROR;
}

void TimingMaster::CheckAckTimeout()
{
    if (_last_acknowledge_received_timestamp > _acknowledge_wait_timeout)
    {
        _state_machine->ErrorEvent();
       
        INVOKE_INCIDENT(_incident_handler, FSI_TIMING_MASTER_ACKNOWLEDGEMENT_RECEPTION_TIMEOUT, SL_Critical_Global,
            "Timing Master did not receive any acknowledgements during the configured timeout.");
    }
    _last_acknowledge_received_timestamp += watchdog_period;
}
