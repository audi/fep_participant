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

#include "fep_participant_sdk.h"

namespace fep_wrap
{
    enum ReliabilityMode
    {
        BestEffortReliabilityMode= 0,
        ReliableReliabilityMode= 1,

        MaximumNumberForReliabilityMode = 1
    };

    struct DdsConfig
    {
            std::size_t sample_size;
            ReliabilityMode reliability_mode;
            bool multicast_enable;
            bool async_mode_enable;
    };

}
