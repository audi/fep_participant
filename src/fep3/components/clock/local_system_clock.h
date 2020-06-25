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
#ifndef __FEP_LOCAL_SYSTEM_CLOCK_H
#define __FEP_LOCAL_SYSTEM_CLOCK_H

#include <a_util/base/types.h>
#include "fep3/components/clock/clock_base.h"

namespace fep
{

class LocalSystemRealClock : public ContinuousClock
{
    public:
        LocalSystemRealClock();
        ~LocalSystemRealClock() = default;

      public:
          timestamp_t getNewTime() const override;
          timestamp_t resetTime() override;

    private:
        mutable timestamp_t _current_offset;


};

}
#endif //__FEP_LOCAL_SYSTEM_CLOCK_H
