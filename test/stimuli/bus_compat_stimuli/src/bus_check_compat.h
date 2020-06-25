/**
 *
 * Bus Compat Stimuli: Base class for all checks
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

#ifndef _BUS_COMPAT_STIMULI_BUS_CHECK_COMPAT_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_BUS_CHECK_COMPAT_H_INCLUDED_

#include "stdafx.h"

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR <= 2)
namespace fep
{
    typedef tResult Result;

    inline bool isOk(const Result& result)
    {
        return IS_OK(result);
    }

    inline bool isFailed(const Result& result)
    {
        return IS_FAILED(result);
    }

}
typedef tHandle handle_t;
typedef tTimeStamp timestamp_t;
#else
#endif



#endif // _BUS_COMPAT_STIMULI_BUS_CHECK_COMPAT_H_INCLUDED_
