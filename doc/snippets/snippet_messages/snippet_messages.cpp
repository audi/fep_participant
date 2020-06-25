/************************************************************************
 * Snippets hosting FEP Participant ... nothing else. :P
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
 *
 */
#include "stdafx.h"
#include "snippet_messages.h"

//![IncidentLog]
fep::Result cMyFEPElement::SomeMethod()
{
    // [...]
    GetIncidentHandler()->InvokeIncident(fep::FSI_GENERAL_INFORMATION, fep::SL_Info,
        "My Log Message here....","SomeComponentName",__LINE__,__FILE__);
    // [...]
    return fep::ERR_NOERROR;
}
//![IncidentLog]
