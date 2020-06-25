/**
 * Include type header for the FEP SDK.
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
 
#ifndef _FEP_TYPES_H_INCLUDED_
#define _FEP_TYPES_H_INCLUDED_

#include "fep_participant_export.h"

#include <cstddef>
#include <a_util/base/types.h>
#define timestamp_t_DEFINED
#include <limits>
#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
// using std::numeric_limits<...> causes lots of -Wattributes warnings with this gcc
#define INVALID_timestamp_t_fep INT64_MIN
#else
#ifdef min 
#undef min
#endif
#define INVALID_timestamp_t_fep std::numeric_limits<timestamp_t>::min()
#endif
#include "fep_errors.h"

#endif // _FEP_TYPES_H_INCLUDED_
