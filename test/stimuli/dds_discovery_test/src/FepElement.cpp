/**
 * Implementation of the Class FepElement.
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

#define NOMINMAX

#include "stdafx.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>

#include "FepElement.h"

using namespace fep;

FepElement::FepElement()
{
}

FepElement::~FepElement()
{
}

fep::Result FepElement::ProcessStartupEntry(const fep::tState eOldState)
{
    GetStateMachine()->StartupDoneEvent();

    return ERR_NOERROR;
}

fep::Result FepElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    GetStateMachine()->InitDoneEvent();

    return ERR_NOERROR;
}
