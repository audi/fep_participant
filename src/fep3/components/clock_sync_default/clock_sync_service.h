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
#ifndef __FEP_CLOCK_SYNC_SERVICE_H
#define __FEP_CLOCK_SYNC_SERVICE_H

#include <memory>
#include <utility>
#include "fep_result_decl.h"
#include "fep3/components/base/component_base.h"
#include "fep3/components/clock_sync_default/clock_sync_service_intf.h"
#include "master_on_demand_clock_client.h"

namespace fep
{
class ClockBase;

namespace detail
{

class ClockSynchronizationService : public IClockSyncService,
                                    public ComponentBase
{
    public:
        explicit ClockSynchronizationService();
        ~ClockSynchronizationService();

    public:
        fep::Result create() override;
        fep::Result initializing() override;
        fep::Result deinitializing() override;

        fep::Result start() override;
        fep::Result stop() override;

        void* getInterface(const char* iid) override;

    private:
        //configured clock synchronizer
        std::pair<std::unique_ptr<ClockBase>, FarClockUpdater*> _slave_clock;
};

}
}
#endif // __FEP_CLOCK_SYNC_SERVICE_H
