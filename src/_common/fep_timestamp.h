/**
 * Implementation of FEP Timestamp functions
 *

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
 * @file
 */

#ifndef _FEP_TIMESTAMP_
#define _FEP_TIMESTAMP_

#include <a_util/base/types.h>
#include "fep_participant_export.h"

namespace fep
{ 
    /// @return Milliseconds since 00:00 hours, Jan 1, 1970 UTC
    FEP_PARTICIPANT_EXPORT timestamp_t GetTimeStampMicrosecondsUTC();
};

#endif //_FEP_TIMESTAMP_
