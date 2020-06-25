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

#include <string>
#include "participant_info_intf.h"
#include "fep3/components/base/component_base.h"

namespace fep
{
    class FEP_PARTICIPANT_EXPORT ParticipantInfo 
        : public IParticipantInfo
        , public ComponentBase
    {
        public:
            explicit ParticipantInfo(const std::string& name, const std::string& version_info);
            ~ParticipantInfo() = default;

            std::string getName() const override;
            std::string getVersionInfo() const override;

            void* getInterface(const char* iid) override;

        private:
            std::string _name;
            std::string _version_info;
    };

}
