/**
* Declaration of the Class cTimedDataAccess.
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

#if !defined(FEP_TIMED_DATA_ACCESS__INCLUDED_)
#define FEP_TIMED_DATA_ACCESS__INCLUDED_

#include <map>
#include <mutex>
#include <string>
#include <a_util/base/types.h>
#include <a_util/concurrency/semaphore.h>

#include "data_access/fep_step_data_access_intf.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep3/components/legacy/timing/common_timing.h"
#include "fep_participant_export.h"
#include "fep_result_decl.h"

namespace fep
{
    class IIncidentHandler;
    class IStateMachine;
    class IUserDataAccessPrivate;
    class IUserDataSample;
    class cDataSampleBuffer;

    ///@cond nodoc
    ///
    class FEP_PARTICIPANT_EXPORT IStepDataAccessPrivate
    {
    public:
        virtual ~IStepDataAccessPrivate() = default;

    public:
        virtual fep::Result ConfigureInput(const std::string& strName, const timing::InputConfig& oInputConfig) =0;
        virtual fep::Result ConfigureOutput(const std::string& strName, const timing::OutputConfig& oOutputConfig) =0;
        virtual fep::Result Clear() =0;
        virtual void SetCycleTime(timestamp_t tmCycleTime) =0;
        virtual void SetWaitTimeForInputs(timestamp_t tmWaitTime) =0;
        virtual fep::Result ValidateInputs(timestamp_t tmCurrSimTime, a_util::concurrency::semaphore& thread_shutdown_semaphore) =0;
        virtual fep::Result TransmitAllOutputs() =0;
        virtual void SetSkip() =0;
    };
    ///@endcond nodoc


    /**
    * The \ref cStepDataAccess implements the interface of \ref IStepDataAccess.
    */
    class FEP_PARTICIPANT_EXPORT cStepDataAccess : public IStepDataAccess, public IStepDataAccessPrivate
    {
    private:
        ///@cond nodoc
        struct tInput
        {
            std::string name;
            timestamp_t tmValidAge;
            timestamp_t tmDelay;
            cDataSampleBuffer* pBuffer;
        };
        
        struct ReverseInputViolationStrategyLess
        {
            bool operator()(const timing::InputViolationStrategy& lhs, const timing::InputViolationStrategy& rhs)
            {
                return lhs > rhs;
            }
        };

        typedef std::unique_lock<std::mutex> InputGuard;
        typedef std::map<handle_t, IUserDataSample*> OutputMap;
        typedef std::multimap<timing::InputViolationStrategy, tInput, ReverseInputViolationStrategyLess> InputMap;
        ///@endcond nodoc

    public:
        /// CTOR
        cStepDataAccess(IUserDataAccessPrivate* pUserDataAccessPrivate, IStateMachine* pStateMachinePrivate, IIncidentHandler* pIncidentHandler);

        ///DTOR
        ~cStepDataAccess();

    private: // Implements IStepDataAccess
        /// @copydoc IStepDataAccess::CopyRecentData
        Result CopyRecentData(handle_t hSignalHandle, IUserDataSample*& poSample);

        /// @copydoc IStepDataAccess::CopyDataAt
        Result CopyDataBefore(handle_t hSignalHandle, timestamp_t tmUpperBound, IUserDataSample*& poSample);

        /// @copydoc IStepDataAccess::TransmitData
        Result TransmitData(IUserDataSample* poSample);

    private: // Implements IStepDataAccessPrivate
        Result ConfigureInput(const std::string& strName, const timing::InputConfig& oInputConfig);
        Result ConfigureOutput(const std::string& strName, const timing::OutputConfig& oOutputConfig);
        Result Clear();
        inline void SetCycleTime(timestamp_t tmCycleTime) { m_tmCycle = tmCycleTime; }
        inline void SetWaitTimeForInputs(timestamp_t tmWaitTime) { m_tmWaitTime = tmWaitTime; }
        Result ValidateInputs(timestamp_t tmCurrSimTime, a_util::concurrency::semaphore& thread_shutdown_semaphore);
        Result ApplyInputViolationStrat(const std::string& strName, timing::InputViolationStrategy eStrategy);
        inline void SetSkip() { m_bNeedToSkip = true; }

    private:
        Result TransmitAllOutputs();

    private:
        bool m_bNeedToSkip;
        timestamp_t m_tmCurrSimTime;
        timestamp_t m_tmCycle;
        timestamp_t m_tmWaitTime;
        InputMap m_oInputs;
        OutputMap m_oOutputs;
        IUserDataAccessPrivate* m_pUserDataAccessPrivate;
        IStateMachine*  m_pStateMachine;
        IIncidentHandler* m_pIncidentHandler;
    };
}

#endif
