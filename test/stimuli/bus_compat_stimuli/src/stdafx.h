/**
 *
 * Standard header file.
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
#ifndef __STD_INCLUDES_HEADER
#define __STD_INCLUDES_HEADER

#include <limits>
#include <cmath>
#include <cassert>
#include <iostream>

#include <fep_sdk_participant_version.h>

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
#include <stdint.h>
#include "a_utils.h"
#endif

#include <fep_participant_sdk.h>

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
#include <fep_participant_sdk_private.h>
#endif
#include "bus_check_compat.h"

#endif // __STD_INCLUDES_HEADER
