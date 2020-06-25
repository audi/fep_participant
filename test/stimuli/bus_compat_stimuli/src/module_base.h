/**
 *
 * Bus Compat Stimuli: Base Module Header 
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

#ifndef _BUS_COMPAT_STIMULI_MODULE_BASE_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_MODULE_BASE_H_INCLUDED_

#include "stdafx.h"

class cModuleBase : public fep::cModule
{
public:
    cModuleBase();

public:
    fep::Result ProcessStartupEntry(const fep::tState eOldState);
};


#endif // _BUS_COMPAT_STIMULI_MODULE_BASE_H_INCLUDED_
