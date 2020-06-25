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
#pragma once

#include <memory>
#include <vector>

#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "fep3/components/base/component_intf.h"
#include "fep3/participant/participant_intf.h"
#include "fep3/components/scheduler/jobs/job.h" // IWYU pragma: keep
#include "module/fep_module_options.h"

namespace fep
{
    class cModuleOptions;

    /**
     * This Participant takes care of the Data jobs given.
     *
     * This will create one single participant as gateway to the fep simulation and service bus
     * and adds the data job to the @ref page_fep_scheduler_service
     * and @ref page_fep_data_registry. 
     * 
     * This bundle of DataJob will be the \b Element.
     *
     * Usually the participant name (and so its address) set while creation call given with fep::cModuleOptions is equal to 
     * the loaded element name. Otherwise you have to reset the @ref FEP_PARTICIPANT_HEADER_NAME after creation.
     *
     * @see @ref service_bus_emulation
     *      @ref simulation_bus_emulation
     * 
     */
    class FEP_PARTICIPANT_EXPORT ParticipantFEP2 final : public IParticipant
    {
        public:
            /**
             * CTOR
             */
            ParticipantFEP2();
            /**
             * @brief Destroy the Participant
             * 
             */
            ~ParticipantFEP2();

            /**
             * @brief move construct a new Participant from another instance
             * 
             * @param other the other instance to move
             */
            ParticipantFEP2(ParticipantFEP2&& other) = default;
            /**
             * @brief move a new Participant from an other instance
             * 
             * @param other the other to move
             * @return ParticipantFEP2& 
             */
            ParticipantFEP2& operator=(ParticipantFEP2&& other) = default;

            /**
             * @brief Remove copy CTOR
             * 
             * @param other the other
             */
            ParticipantFEP2(const ParticipantFEP2& other) = delete;
            /**
             * @brief Remove copy operator
             * 
             * @param other the other
             * @return ParticipantFEP2& 
             */
            ParticipantFEP2& operator=(const ParticipantFEP2& other) = delete;

            /**
             * @brief Creates all necessary components. After creation the 
             * Participant is in FS_IDLE state.
             * 
             * @param module_option see fep::cModuleOptions for more information
             * @return fep::Result 
             */
            fep::Result Create(const fep::cModuleOptions& module_option);

            /**
             * @brief Creates all necessary components. After creation the 
             * Participant is in FS_IDLE state.
             * 
             * @param module_option see fep::cModuleOptions for more information
             * @param job the job the Participant will take care of.
             * @return fep::Result 
             */
            fep::Result Create(const fep::cModuleOptions& module_option,
                               std::shared_ptr<fep::Job> job);

            /**
             * @brief Creates all necessary components. After creation the 
             * Participant is in FS_IDLE state.
             * 
             * @param module_option see fep::cModuleOptions for more information
             * @param jobs the job the Participant will take care of.
             * @return fep::Result 
             */
            fep::Result Create(const fep::cModuleOptions& module_option,
                               const std::vector<std::shared_ptr<fep::Job>>& jobs);
        public:
            /**
             * @brief usually the participant is control from outside
             * this function will block until the participant is shutdown
             * 
             * @param module_option see fep::cModuleOptions for more information
             * @param jobs the job the Participant will take care of.
             * @return fep::Result 
             */
            fep::Result waitForShutdown() const override;
            /**
             * @brief returns the components registry access reference
             * 
             * @return fep::IComponents& the reference
             */
            fep::IComponents& getComponents() const override;

        private:
            /**
             * Internal impl
             * 
             */
            class Implementation;
            /**
             * @brief pimpl
             * 
             */
            std::unique_ptr<Implementation> _impl;
    };



}
