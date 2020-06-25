/**

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
 */

#include "fep3/participant/participant_fep2.h"
#include <a_util/result/result_type.h>
#include "fep3/base/states/fep2_state.h"
#include "fep3/components/scheduler/jobs/job.h"
#include "fep_errors.h"
#include "module/fep_module.h"
#include "statemachine/fep_state_request_listener.h"
#include "statemachine/fep_statemachine_intf.h"

namespace fep
{
    class ParticipantFEP2::Implementation : public cModule,
                                            public cStateRequestListener
    {
        public:
            Implementation() = default;
            virtual ~Implementation() = default;

        public:
            fep::Result ProcessStartupEntry(const fep::tState) override
            {
                GetStateMachine()->RegisterStateRequestListener(this);
                return GetStateMachine()->StartupDoneEvent();
            }
            fep::Result ProcessInitializingEntry(const fep::tState) override
            {
                //workaround 
                return GetStateMachine()->InitDoneEvent();
            }
            fep::Result ProcessRunningRequest(const fep::tState) override
            {
                for (auto& current_job : _jobs)
                {
                    auto err = current_job->reset();
                    if (fep::isFailed(err))
                    {
                        return err;
                    }
                }
                return fep::Result();
            }
            fep::Result CleanUp(const fep::tState state) override
            {
                for (auto& current_job : _jobs)
                {
                    auto err = current_job->removeFromComponents(*GetComponents());
                }
                return cModule::CleanUp(state);
            }
            std::vector<std::shared_ptr<fep::Job>> _jobs;
    };

    ParticipantFEP2::ParticipantFEP2()
    {
        _impl.reset(new Implementation());
    }
    ParticipantFEP2::~ParticipantFEP2()
    {
        if (_impl)
        {
            _impl->Destroy();
        }
    }

    fep::Result ParticipantFEP2::Create(const fep::cModuleOptions& oModuleOptions)
    {
        RETURN_IF_FAILED(_impl->Create(oModuleOptions));

        return fep::Result();
    }

    fep::Result ParticipantFEP2::Create(const fep::cModuleOptions& oModuleOptions,
                                        std::shared_ptr<fep::Job> job)
    {
        _impl->_jobs.push_back(job);
        RETURN_IF_FAILED(_impl->Create(oModuleOptions));
        for (const auto& current_job : _impl->_jobs)
        {
            RETURN_IF_FAILED(current_job->addToComponents(getComponents()));
        }

        return fep::Result();
    }

    fep::Result ParticipantFEP2::Create(const fep::cModuleOptions& oModuleOptions,
        const std::vector<std::shared_ptr<fep::Job>>& jobs)
    {
        _impl->_jobs.insert(_impl->_jobs.end(), jobs.begin(), jobs.end());
        RETURN_IF_FAILED(_impl->Create(oModuleOptions));
        for (const auto& current_job : _impl->_jobs)
        {
            RETURN_IF_FAILED(current_job->addToComponents(getComponents()));
        }

        return fep::Result();
    }

    fep::Result ParticipantFEP2::waitForShutdown() const
    {
        return _impl->WaitForShutdown();
    }

    fep::IComponents& ParticipantFEP2::getComponents() const 
    {
        return *_impl->GetComponents();
    }

    

}
