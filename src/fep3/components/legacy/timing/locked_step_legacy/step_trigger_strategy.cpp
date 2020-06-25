/**
* Implementation of the Class TimingClient.
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

#include "step_trigger_strategy.h"
#include <limits>
#include <string>
#include <a_util/concurrency/chrono.h>
#include <a_util/strings/strings_format.h>
#include <a_util/system/system.h>
#include <codec/access_element.h>
#include <codec/static_codec.h>

#include "_common/fep_optional.h"
#include "data_access/fep_data_access.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/propertytreebase_intf.h"
#include "fep3/components/legacy/timing/locked_step_legacy/step_trigger_strategy_intf.h"
#include "fep3/components/legacy/timing/timing_master_intf.h"
#include "signal_registry/fep_signal_struct.h"
#include "transmission_adapter/fep_signal_direction.h"
#include "transmission_adapter/fep_signal_serialization.h"
#include "transmission_adapter/fep_transmission.h"
#include "transmission_adapter/fep_user_data_sample_intf.h"

using namespace fep::timing;
using namespace fep;

// Internal step trigger description 
// Must match description/fep_timing.description
static const char* privStepTriggerSignalDescTemplate = "\
<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>\n\
<adtf:ddl xmlns:adtf=\"adtf\">\n\
 <header>\n\
  <language_version>3.00</language_version>\n\
  <author>AUDI AG</author>\n\
  <date_creation>04.10.2017</date_creation>\n\
  <date_change>04.10.2017</date_change>\n\
  <description>FEP Clock standard interfaces</description>\n\
 </header>\n\
 <units />\n\
 <datatypes>\n\
  <datatype description=\"predefined ADTF tBool datatype\" max=\"tTrue\" min=\"tFalse\" name=\"tBool\" size=\"8\" />\n\
  <datatype description=\"predefined ADTF tChar datatype\" max=\"127\" min=\"-128\" name=\"tChar\" size=\"8\" />\n\
  <datatype description=\"predefined ADTF tUInt8 datatype\" max=\"255\" min=\"0\" name=\"tUInt8\" size=\"8\" />\n\
  <datatype description=\"predefined ADTF tInt8 datatype\" max=\"127\" min=\"-128\" name=\"tInt8\" size=\"8\" />\n\
  <datatype description=\"predefined ADTF tUInt16 datatype\" max=\"65535\" min=\"0\" name=\"tUInt16\" size=\"16\" />\n\
  <datatype description=\"predefined ADTF tInt16 datatype\" max=\"32767\" min=\"-32768\" name=\"tInt16\" size=\"16\" />\n\
  <datatype description=\"predefined ADTF tUInt32 datatype\" max=\"4294967295\" min=\"0\" name=\"tUInt32\" size=\"32\" />\n\
  <datatype description=\"predefined ADTF tInt32 datatype\" max=\"2147483647\" min=\"-2147483648\" name=\"tInt32\" size=\"32\" />\n\
  <datatype description=\"predefined ADTF tUInt64 datatype\" max=\"18446744073709551615\" min=\"0\" name=\"tUInt64\" size=\"64\" />\n\
  <datatype description=\"predefined ADTF tInt64 datatype\" max=\"9223372036854775807\" min=\"-9223372036854775808\" name=\"tInt64\" size=\"64\" />\n\
  <datatype description=\"predefined ADTF tFloat32 datatype\" max=\"3.402823e+38\" min=\"-3.402823e+38\" name=\"tFloat32\" size=\"32\" />\n\
  <datatype description=\"predefined ADTF tFloat64 datatype\" max=\"1.797693e+308\" min=\"-1.797693e+308\" name=\"tFloat64\" size=\"64\" />\n\
 </datatypes>\n\
 <enums />\n\
 <structs>\n\
  <struct alignment=\"1\" name=\"%s\" version=\"1\">\n\
    <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"currentTime_us\" type=\"tInt64\" />\n\
    <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"8\" name=\"validity_us\" type=\"tInt64\" />\n\
  </struct>\n\
 </structs>\n\
 <streams>\n\
 </streams>\n\
</adtf:ddl>\n\
";

static std::string privStepTriggerSignalDesc = a_util::strings::format(privStepTriggerSignalDescTemplate, g_stepTriggerSignalType);


cBaseStepTriggerStrategy::cBaseStepTriggerStrategy()
    : _step_trigger_strategy_listener(nullptr)
    , _cycle_time(0)
{
}

Result cBaseStepTriggerStrategy::RegisterStrategyTrigger(const timestamp_t cycle_time, IStepTriggerStrategyListener* step_trigger_strategy_listener)
{
    // Valid arguments ?
    if ((cycle_time <= 0) || (nullptr == step_trigger_strategy_listener))
    {
        return ERR_INVALID_ARG;
    }

    // Already registered ?
    if (_step_trigger_strategy_listener)
    {
        return ERR_UNEXPECTED;
    }

    _step_trigger_strategy_listener = step_trigger_strategy_listener;
    _cycle_time = cycle_time;

    return ERR_NOERROR;
}

Result cBaseStepTriggerStrategy::UnregisterStrategyTrigger(IStepTriggerStrategyListener* step_trigger_strategy_listener)
{
    // Valid arguments ?
    if (nullptr == step_trigger_strategy_listener)
    {
        return ERR_INVALID_ARG;
    }

    // Registered ?
    if (_step_trigger_strategy_listener != step_trigger_strategy_listener)
    {
        return ERR_UNEXPECTED;
    }

    _step_trigger_strategy_listener = nullptr;
    _cycle_time = 0;

    return ERR_NOERROR;
}

void cBaseStepTriggerStrategy::doCall(const timestamp_t current_simulation_time)
{
    if (_step_trigger_strategy_listener)
    {
        _step_trigger_strategy_listener->InternalTrigger(current_simulation_time);
    }
}

cInternalStepTriggerStrategy::cInternalStepTriggerStrategy()
    : _thread()
    , _shutdown_semaphore()
    , _real_time_scale(1000000)
{
}

Result cInternalStepTriggerStrategy::initialize(IPropertyTreeBase* property_tree)
{
    Result result = ERR_NOERROR;

    // Get timing master name
    double time_scale = 1.0;
    if (fep::isOk(property_tree->GetPropertyValue(FEP_TIMING_MASTER_TIME_FACTOR, time_scale)))
    {
        if (time_scale < std::numeric_limits<double>::epsilon())
        {
            _real_time_scale = 0;
        }
        else if (time_scale > std::numeric_limits<double>::epsilon())
        {
            _real_time_scale = static_cast<timestamp_t>(1000000.0 / time_scale);
        }
        else
        {
            result = ERR_INVALID_ARG;
        }
    }

    return result;
}

Result cInternalStepTriggerStrategy::reset()
{
    Result result = ERR_NOERROR;

    return result;
}

Result cInternalStepTriggerStrategy::Start()
{
    _shutdown_semaphore.reset();

    if (NeedToWaitForTrigger())
    {
        // Thread is only needed if ticking mode is used
        _thread.reset(new a_util::concurrency::thread(&cInternalStepTriggerStrategy::Work, this));
    }

    return ERR_NOERROR;
}

Result cInternalStepTriggerStrategy::Stop()
{
    if (_thread)
    {
        _shutdown_semaphore.notify();
        _thread->join();
        _thread.reset();
        _shutdown_semaphore.reset();
    }

    return ERR_NOERROR;
}

void cInternalStepTriggerStrategy::Work()
{
    timestamp_t next_cycle_timestamp = 0;
    timestamp_t next_cylcle_local_ref = a_util::system::getCurrentMicroseconds();
    timestamp_t real_time_cycle = (_real_time_scale * getCycleTime()) / 1000000;

    while (!_shutdown_semaphore.is_set())
    {
        timestamp_t wait_time = next_cylcle_local_ref - a_util::system::getCurrentMicroseconds();
        if (wait_time >= 0)
        {
            a_util::system::sleepMicroseconds(wait_time);
        }
        doCall(next_cycle_timestamp);
        next_cycle_timestamp += getCycleTime();
        next_cylcle_local_ref += real_time_cycle;
    }
}

cAFAPStepTriggerStrategy::cAFAPStepTriggerStrategy()
{
}

Result cAFAPStepTriggerStrategy::Start()
{
    return ERR_NOERROR;
}

Result cAFAPStepTriggerStrategy::Stop()
{
    return ERR_NOERROR;
}

cExternalStepTriggerStrategy::cExternalStepTriggerStrategy()
    : _data_access_private(nullptr)
    , _transmission_adapter_private(nullptr)
    , _step_trigger_input_signal_handle(nullptr)
    , _thread()
    , _shutdown_semaphore()
    , _time_update_cv()
    , _time_update_mutex()
    , _next_cycle_timestamp(-1)
    , _next_cylcle_local_ref(-1)
    , _run_cycle_limit(0)
    , _codec_factory("tFEP_StepTrigger", privStepTriggerSignalDesc.c_str())
{
}

cExternalStepTriggerStrategy::~cExternalStepTriggerStrategy()
{
    reset();
}

Result cExternalStepTriggerStrategy::initialize(IUserDataAccessPrivate* data_access_private, ITransmissionAdapterPrivate* transmission_adapter_private)
{
    Result result = ERR_NOERROR;

    if (!transmission_adapter_private || !data_access_private)
    {
        result = ERR_INVALID_ARG;
    }

    if (isOk(result))
    {
        _data_access_private = data_access_private;
        _transmission_adapter_private = transmission_adapter_private;
        tSignal oSignal;
        oSignal.strSignalName = timing::g_stepTriggerSignalName;
        oSignal.szSampleSize = _codec_factory.getStaticBufferSize();
        oSignal.bIsReliable = false;
        oSignal.bIsRaw = false; 
        oSignal.eSerialization = SER_Ddl;
        oSignal.eDirection = fep::SD_Input;
        oSignal.bIsMapped = false;
        oSignal.strSignalDesc = "";
        oSignal.szSampleBacklog = 0;
        oSignal.strSignalType = g_stepTriggerSignalType;
        oSignal.strSignalDesc = privStepTriggerSignalDesc.c_str();

        if (isOk(result))
        {
            result = _transmission_adapter_private->RegisterSignal(oSignal, _step_trigger_input_signal_handle);
        }

        if (isOk(result))
        {
            result = _data_access_private->RegisterDataListener(this, _step_trigger_input_signal_handle);
        }
    }

    return result;
}

Result cExternalStepTriggerStrategy::reset()
{
    Result result = ERR_NOERROR;

    if (!_transmission_adapter_private || _data_access_private)
    {
        result = ERR_INVALID_ARG;
    }

    if (isOk(result))
    {
        result = _data_access_private->UnregisterDataListener(this, _step_trigger_input_signal_handle);
    }

    if (isOk(result))
    {
        result = _transmission_adapter_private->UnregisterSignal(_step_trigger_input_signal_handle);
    }

    _transmission_adapter_private = nullptr;
    _data_access_private = nullptr;

    return result;
}

Result cExternalStepTriggerStrategy::Start()
{
    _next_cycle_timestamp= -1;
    _run_cycle_limit= 0;

    _shutdown_semaphore.reset();
    _thread.reset(new a_util::concurrency::thread(&cExternalStepTriggerStrategy::Work, this));

    return ERR_NOERROR;
}

Result cExternalStepTriggerStrategy::Stop()
{
    if (_thread)
    {
        _shutdown_semaphore.notify();
        _thread->join();
        _thread.reset();
        _shutdown_semaphore.reset();
    }

    return ERR_NOERROR;
}

Result cExternalStepTriggerStrategy::Update(const IUserDataSample* poSample)
{
    Result result = ERR_NOERROR;

    if (poSample && poSample->GetSignalHandle() == _step_trigger_input_signal_handle)
    {
        timestamp_t currentTime_us = 0;
        timestamp_t validity_us = 0;

        ddl::StaticCodec output_coder = _codec_factory.makeStaticCodecFor(poSample->GetPtr(), poSample->GetSize());
        result = output_coder.isValid();
        if (fep::isOk(result))
        {
            result = ddl::access_element::get_value(output_coder, "currentTime_us", &currentTime_us);
            result |= ddl::access_element::get_value(output_coder, "validity_us", &validity_us);
        }

        if (fep::isOk(result))
        {
            a_util::concurrency::unique_lock<a_util::concurrency::mutex> locker(_time_update_mutex);

            if (_next_cycle_timestamp < 0)
            {
                _next_cylcle_local_ref = a_util::system::getCurrentMicroseconds();
                _next_cycle_timestamp = currentTime_us;
            }
            _run_cycle_limit = currentTime_us + validity_us;
            _time_update_cv.notify_one();
        }
    }

    return result;
}

void cExternalStepTriggerStrategy::Work()
{
    // Wait for first tick to arive
    while (!_shutdown_semaphore.is_set())
    {
        LOCKER_TYPE locker(_time_update_mutex);

        if (_time_update_cv.wait_for(locker, a_util::chrono::milliseconds(10)) == a_util::concurrency::cv_status::no_timeout)
        {
            break;
        }

        if (_next_cycle_timestamp >= 0)
        {
            break;
        }
    }
    
    timestamp_t this_cycle_timestamp;
    timestamp_t this_cycle_local_ref;
    while (!_shutdown_semaphore.is_set())
    {
        {
            LOCKER_TYPE locker(_time_update_mutex);

            this_cycle_timestamp = _next_cycle_timestamp;
            this_cycle_local_ref = _next_cylcle_local_ref;
            _next_cycle_timestamp += getCycleTime();
            _next_cylcle_local_ref += getCycleTime();

            while (this_cycle_timestamp >= _run_cycle_limit)
            {
                if (_time_update_cv.wait_for(locker, a_util::chrono::milliseconds(10)) == a_util::concurrency::cv_status::no_timeout)
                {
                    break;
                }
                if (_shutdown_semaphore.is_set())
                {
                    break;
                }
            }
        }
            
        if (_shutdown_semaphore.is_set())
        {
            break;
        }
                
        timestamp_t wait_time = this_cycle_local_ref - a_util::system::getCurrentMicroseconds();
        if (wait_time >= 0)
        {
            a_util::system::sleepMicroseconds(wait_time);
        }

        doCall(this_cycle_timestamp);
    }
}

cUserStepTriggerStrategy::cUserStepTriggerStrategy()
    : _user_step_trigger(nullptr)
    , _current_simulation_time(0)
    , _trigger_was_called_flag(false)
{
}

fep::Result cUserStepTriggerStrategy::initialize(IStepTrigger* pUserStepTrigger)
{
    fep::Result result = ERR_NOERROR;

    if (pUserStepTrigger)
    {
        _user_step_trigger = pUserStepTrigger;
    }
    else
    {
        result = ERR_INVALID_ARG;
    }

    _current_simulation_time = 0;
    _trigger_was_called_flag = false;

    return result;
}

fep::Result cUserStepTriggerStrategy::reset()
{
    fep::Result result = ERR_NOERROR;

    if (_user_step_trigger)
    {
        _user_step_trigger = nullptr;
    }
    else
    {
        result = ERR_INVALID_ARG;
    }

    _current_simulation_time = 0;
    _trigger_was_called_flag = false;

    return result;
}


fep::Result cUserStepTriggerStrategy::RegisterStrategyTrigger(const timestamp_t cycle_time, IStepTriggerStrategyListener* step_trigger_strategy_listener)
{
    fep::Result result = cBaseStepTriggerStrategy::RegisterStrategyTrigger(cycle_time, step_trigger_strategy_listener);

    if (isOk(result))
    {
        if (_user_step_trigger)
        {
            result = _user_step_trigger->RegisterTrigger(cycle_time, this);
        }
        else
        {
            result = ERR_INVALID_ARG;
        }
    }

    return result;
}

fep::Result cUserStepTriggerStrategy::UnregisterStrategyTrigger(IStepTriggerStrategyListener* step_trigger_strategy_listener)
{
    fep::Result result = ERR_NOERROR;

    if (_user_step_trigger)
    {
        result= _user_step_trigger->UnregisterTrigger(this);
    }
    else
    {
        result= ERR_INVALID_ARG;
    }


    fep::Result second_result = cBaseStepTriggerStrategy::UnregisterStrategyTrigger(step_trigger_strategy_listener);

    if (isOk(result))
    {
        result = second_result;
    }

    return result;
}

timestamp_t cUserStepTriggerStrategy::Trigger()
{
    _trigger_was_called_flag = true;

    timestamp_t current_simulation_time = _current_simulation_time;

    doCall(current_simulation_time);

    _current_simulation_time += getCycleTime();
 
    return current_simulation_time;
}

fep::Result cUserStepTriggerStrategy::SetInitialSimulationTime(const timestamp_t nInitalSimulationTime)
{
    fep::Result result = ERR_NOERROR;

    if (_trigger_was_called_flag)
    {
        // Already triggered ... too late to change initial simulation time
        result = ERR_UNEXPECTED;
    }
    else
    {
        _current_simulation_time = nInitalSimulationTime;
    }

    return result;
}
