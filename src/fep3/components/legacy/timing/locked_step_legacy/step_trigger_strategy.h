/**
* Declaration of the Internal Step Trigger Implementation.
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

#ifndef __FEP_TIMING_STEP_TRIGGER_STRATEGY_H
#define __FEP_TIMING_STEP_TRIGGER_STRATEGY_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <a_util/base/types.h>
#include <a_util/concurrency/semaphore.h>
#include <a_util/result/result_type.h>
#include <codec/codec_factory.h>

#include "step_trigger_strategy_intf.h"
#include "fep3/components/legacy/timing/step_trigger_intf.h"
#include "fep_participant_export.h"
#include "fep_errors.h"
#include "fep_result_decl.h"
#include "transmission_adapter/fep_user_data_listener_intf.h"

namespace fep
{
    class IPropertyTreeBase;
    class ITransmissionAdapterPrivate;
    class IUserDataAccessPrivate;
    class IUserDataSample;

    namespace timing
    {
        /// Step trigger strategy base class. 
        /// Common parts for more concrete step trigger strategies.
        class FEP_PARTICIPANT_EXPORT cBaseStepTriggerStrategy : public IStepTriggerStrategy
        {
        public:
            // CTOR
            cBaseStepTriggerStrategy();

        public:
            // @copydoc IStepTrigger::RegisterTrigger
            fep::Result RegisterStrategyTrigger(const timestamp_t cycle_time, IStepTriggerStrategyListener* step_trigger_strategy_listener);

            // @copydoc IStepTrigger::UnregisterTrigger
            fep::Result UnregisterStrategyTrigger(IStepTriggerStrategyListener* step_trigger_strategy_listener);

        protected:
            /**
             * Internal helper. Actually call the registered trigger
             * 
             * @param[in] current_simulation_time simualtion time for this step current simualtion time for this step
             */
            void doCall(const timestamp_t current_simulation_time);

            /**
             * Get cycle time of the registed listener
             * 
             * @return cyle time used for triggering
             */
            timestamp_t getCycleTime() const { return _cycle_time;  }

        private:
            /// listener registered
            IStepTriggerStrategyListener* _step_trigger_strategy_listener;
            /// cycle time
            timestamp_t _cycle_time;
        };

        /// Internal Step Trigger Strategy
        class FEP_PARTICIPANT_EXPORT cInternalStepTriggerStrategy : public cBaseStepTriggerStrategy
        {
        public:
            /// CTOR
            cInternalStepTriggerStrategy();

        public:
            /**
            * Initialize the step trigger
            *
            * @param[in]  property_tree property tree base pointer
            * @return Standard result code.
            */
            fep::Result initialize(IPropertyTreeBase* property_tree);

            /**
            * Reset the step trigger
            *
            * @return Standard result code.
            */
            fep::Result reset();

        public:
            /// @copydoc IStepTriggerStrategy::Start
            fep::Result Start();

            /// @copydoc IStepTriggerStrategy::Stop
            fep::Result Stop();

            /// @copydoc IStepTriggerStrategy::NeedToWaitForTrigger
            bool NeedToWaitForTrigger() const { return true; }

            /// @copydoc IStepTriggerStrategy::NeedToWaitForCompletition
            bool NeedToWaitForCompletition() const { return true; }

            /// @copydoc IStepTriggerStrategy::DoesSupportDummyTimeTrigger
            bool HasSupportDummyTimeTrigger() const { return true; }

        private:
            ///@cond nodoc
            void Work();
            ///@endcond nodoc

        private:
            /// the thread
            a_util::memory::unique_ptr<a_util::concurrency::thread> _thread;
            /// Thread shutdown signal
            a_util::concurrency::semaphore _shutdown_semaphore;
            /// Scale factor: scale is in _timestamp_ticks / s
            /// If value is 1000000 the scale is 1.0
            /// If value is 2000000 the scale is 2.0
            /// If value is 0 ticking is disabled
            timestamp_t _real_time_scale;
        };

        /// AFAP Step Trigger Strategy
        class FEP_PARTICIPANT_EXPORT cAFAPStepTriggerStrategy : public cBaseStepTriggerStrategy
        {
        public:
            /// CTOR
            cAFAPStepTriggerStrategy();

        public:
            /// @copydoc IStepTriggerStrategy::Start
            fep::Result Start();

            /// @copydoc IStepTriggerStrategy::Stop
            fep::Result Stop();

            /// @copydoc IStepTriggerStrategy::NeedToWaitForTrigger
            bool NeedToWaitForTrigger() const { return false; }

            /// @copydoc IStepTriggerStrategy::NeedToWaitForCompletition
            bool NeedToWaitForCompletition() const { return true; }

            /// @copydoc IStepTriggerStrategy::DoesSupportDummyTimeTrigger
            bool HasSupportDummyTimeTrigger() const { return false; }
        };

        class FEP_PARTICIPANT_EXPORT cExternalStepTriggerStrategy : public cBaseStepTriggerStrategy, public IUserDataListener
        {
        private:
            /// Used lock type
            typedef a_util::concurrency::mutex LOCK_TYPE;
            /// Used condition variable type
            typedef a_util::concurrency::condition_variable CONDITION_VARIABLE_TYPE;
            /// Used locker type
            typedef a_util::concurrency::unique_lock<LOCK_TYPE> LOCKER_TYPE;

        public:
            /// CTOR
            cExternalStepTriggerStrategy();

            /// DTOR
            ~cExternalStepTriggerStrategy();

        public:
            /**
            * Initialize the step trigger
            *
            * @param[in]  data_access_private pointer to data access
            * @param[in]  transmission_adapter_private pointer to transmission adapter
            * @return Standard result code.
            */
            fep::Result initialize(IUserDataAccessPrivate* data_access_private, ITransmissionAdapterPrivate* transmission_adapter_private);

            /**
            * Reset the step trigger
            *
            * @return Standard result code.
            */
            fep::Result reset();

        public:
            /// @copydoc IStepTriggerStrategy::Start
            fep::Result Start();

            /// @copydoc IStepTriggerStrategy::Stop
            fep::Result Stop();

            /// @copydoc IStepTriggerStrategy::NeedToWaitForTrigger
            bool NeedToWaitForTrigger() const { return true; }

            /// @copydoc IStepTriggerStrategy::NeedToWaitForCompletition
            bool NeedToWaitForCompletition() const { return false; }

            /// @copydoc IStepTriggerStrategy::DoesSupportDummyTimeTrigger
            bool HasSupportDummyTimeTrigger() const { return false; }

        public: // implements IUserDataListener
            fep::Result Update(const IUserDataSample* poSample);

        private:
            ///@cond nodoc
            void Work();
            ///@endcond nodoc

        private:
            /// Data access pointer
            IUserDataAccessPrivate* _data_access_private; 
            /// Transmission adapter pointer
            ITransmissionAdapterPrivate* _transmission_adapter_private;
            /// Step input signal handle
            handle_t _step_trigger_input_signal_handle;
            /// the thread
            a_util::memory::unique_ptr<a_util::concurrency::thread> _thread;
            /// Thread shutdown signal
            a_util::concurrency::semaphore _shutdown_semaphore;
            /// Internal condition variable
            CONDITION_VARIABLE_TYPE _time_update_cv;
            /// Internal mutex
            LOCK_TYPE _time_update_mutex;
            /// Next planed cycle timestamp 
            timestamp_t _next_cycle_timestamp;
            /// Next planed cycle local time
            timestamp_t _next_cylcle_local_ref;
            /// End of scheduling sycles
            volatile timestamp_t _run_cycle_limit;
            // Use codec
            ddl::CodecFactory _codec_factory;
        };

        /// User Step trigger Strategy
        class FEP_PARTICIPANT_EXPORT cUserStepTriggerStrategy : public cBaseStepTriggerStrategy, public IStepTriggerListener
        {
        public:
            /// CTOR
            cUserStepTriggerStrategy();

        public:
            /**
            * Initialize the step trigger
            *
            * @param[in]  user_step_trigger pointer to step trigger 
            * @return Standard result code.
            */

            fep::Result initialize(IStepTrigger* user_step_trigger);

            /**
            * Reset the step trigger
            *
            * @return Standard result code.
            */
            fep::Result reset();

        public:
            // @copydoc fep::IStepTrigger::RegisterTrigger
            fep::Result RegisterStrategyTrigger(const timestamp_t cycle_time, IStepTriggerStrategyListener* step_trigger_strategy_listener);

            // @copydoc fep::IStepTrigger::UnregisterTrigger
            fep::Result UnregisterStrategyTrigger(IStepTriggerStrategyListener* step_trigger_strategy_listener);

        public:
            /// @copydoc IStepTriggerStrategy::Start
            fep::Result Start() { return ERR_NOERROR; }

            /// @copydoc IStepTriggerStrategy::Stop
            fep::Result Stop() { return ERR_NOERROR; }

            /// @copydoc IStepTriggerStrategy::NeedToWaitForTrigger
            bool NeedToWaitForTrigger() const { return true; }

            /// @copydoc IStepTriggerStrategy::NeedToWaitForCompletition
            bool NeedToWaitForCompletition() const { return false; }

            /// @copydoc IStepTriggerStrategy::DoesSupportDummyTimeTrigger
            bool HasSupportDummyTimeTrigger() const { return false; }

        public: // implements IStepTriggerListener
            /// @copydoc IStepTriggerListener::Trigger
            timestamp_t Trigger();

            /// @copydoc IStepTriggerListener::setInitialSimulationTime
            fep::Result SetInitialSimulationTime(const timestamp_t nInitalSimulationTime);

        private:
            /// Pointer to user step trigger
            IStepTrigger* _user_step_trigger;
            /// Next planed cycle timestamp 
            timestamp_t _current_simulation_time;
            /// Trigger started flag
            bool _trigger_was_called_flag;
        };
    }
}

#endif // __FEP_TIMING_STEP_TRIGGER_STRATEGY_H
