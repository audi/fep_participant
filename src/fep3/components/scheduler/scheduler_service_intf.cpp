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
#include "fep3/components/scheduler/scheduler_service_intf.h"

namespace fep
{

IScheduler::JobInfo::JobInfo(std::string name, timestamp_t cycle_time_us) :
    _name(std::move(name)),
    _configuration(cycle_time_us)
{
}

IScheduler::JobInfo::JobInfo(std::string name, fep::JobConfiguration configuration) :
    _name(std::move(name)),
    _configuration(std::move(configuration))
{
}

const char * IScheduler::JobInfo::getName() const
{
    return _name.c_str();
}

const fep::JobConfiguration & IScheduler::JobInfo::getConfig() const
{
    return _configuration;
}

} // ns fep