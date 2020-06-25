/**
* Declaration of the Timing Client.
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

#ifndef __FEP_OLD_TIMING_COMPONENT_H
#define __FEP_OLD_TIMING_COMPONENT_H

#include <map>
#include <unordered_map>
#include <memory>
#include <string>

#include <a_util/base/types.h>
#include "data_access/fep_step_data_access_intf.h"
#include "fep3/components/base/component_base_legacy.h"
#include "fep3/components/scheduler/jobs/datajob.h"
#include "fep3/components/legacy/timing/common_timing.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep3/components/legacy/timing/timing_master_intf.h"
#include "fep_result_decl.h"

namespace fep
{
class DataReader;
class DataWriter;
class IClockService;
class IModule;
class ISchedulerService;
class ISignalRegistry;
class IStepTrigger;
class IUserDataAccess;
class IUserDataSample;
class JobConfiguration;

namespace legacy
{

    class ITimingInterfaceLegacy : public ITiming,
                                   public ITimingMaster 
    {
        public: 
            FEP_COMPONENT_IID("ITimingInterfaceLegacy");
    };


    class TimingInterfaceLegacy : public ITimingInterfaceLegacy,
                                  public ComponentBaseLegacy
    {
        public:
            TimingInterfaceLegacy(const IModule& module_ref);
            ~TimingInterfaceLegacy();

            fep::Result create() override;
            fep::Result destroy() override;
            void* getInterface(const char* iid) override;
            fep::Result initializing() override;
            fep::Result deinitializing() override;
        protected: //ITiming
            timestamp_t GetTime() const override;
            fep::Result SetSystemTimeout(const timestamp_t tmSystemTimeout) override;
            fep::Result RegisterStepListener(const char* strStepListenerName, const StepConfig& tConfiguration, ScheduleFunc pCallback, void* pCallee) override;
            fep::Result UnregisterStepListener(const char* strStepListenerName) override;
        protected: //ITimingMaster
            fep::Result SetStepTrigger(IStepTrigger* pUserStepTrigger) override;

        private:
            IClockService*     _clock_service;
            ISchedulerService* _scheduler_service;
            

            class JobToStepTrigger : public DataJob,
                                     public IStepDataAccess
            {
                public:
                    explicit JobToStepTrigger(fep::ITiming::ScheduleFunc callback_fc,
                                              void* callee,
                                              const char* name,
                                              JobConfiguration config,
                                              IUserDataAccess& user_data_access,
                                              IClockService&   clock_service);
                    fep::Result addInput(const std::string& name,
                        const InputConfig& inputs,
                        ISignalRegistry& signal_registry);
                    fep::Result addOutput(const std::string& name,
                        const OutputConfig& inputs,
                        ISignalRegistry& signal_registry);
                    fep::Result addInputs(const InputMap& inputs,
                                          ISignalRegistry& signal_registry);
                    fep::Result addOutputs(const OutputMap& outputs,
                                           ISignalRegistry& signal_registry);

                private:
                    //fep::Result processIn(timestamp_t time_of_execution) override;
                    fep::Result process(timestamp_t time_of_execution) override;
                    fep::Result CopyRecentData(handle_t hSignalHandle,
                                               IUserDataSample*& poSample) override;
                    fep::Result CopyDataBefore(handle_t hSignalHandle,
                                               timestamp_t tmUpperBound,
                                               IUserDataSample*& poSample) override;
                    fep::Result TransmitData(IUserDataSample* poSample) override;

                 private:
                    fep::ITiming::ScheduleFunc _callback_fc;
                    void* _callee;
                    std::unordered_map<handle_t, DataReader*> _handled_readers;
                    std::unordered_map<handle_t, DataWriter*> _handled_writers;
                    IUserDataAccess* _user_data_access;
                    timestamp_t      _current_job_time;
                    IClockService*   _clock_service;
            };
            friend fep::Result reconfigureByTimingConfiguration(TimingInterfaceLegacy::JobToStepTrigger& data_job,
                                                                const std::string& participant_name,
                                                                const timing::TimingConfiguration& timing_configuration,
                                                                ISignalRegistry& signal_registry);
            fep::Result reconfigureByTimingConfiguration(JobToStepTrigger& data_job);
            fep::timing::TimingConfiguration _timing_configuration;
            bool _timing_configuration_file_loaded;
            std::map<std::string, std::unique_ptr<JobToStepTrigger>> _step_trigger;
    };

} //namespace legacy
}

#endif // __FEP_OLD_TIMING_COMPONENT_H
