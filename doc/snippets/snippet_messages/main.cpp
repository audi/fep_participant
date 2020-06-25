/**
 * Implementation of an snippet hosting FEP Participant :P
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
#include <fep_participant_sdk.h>

#include <iostream>

#include "snippet_messages.h"

/**
 *
 * Main function of the FEP application
 *
 * This merely serves as a "script" to drive several use-cases involving
 * incidents.
 *
 * @param [in] nArgc Argument count
 * @param [in] pArgv Argument pointer
 *
 * @return Returns a standard system result code.
 */
int main(int nArgc, const char* pArgv[])
{
    cMyFEPElement oElement;
    if(0 != oElement.Create("BadlyCodedElement"))
    {
        return 1;
    }

    std::cout << ">>>> MAIN::Example run complete. Exiting." << std::endl;
    return 0;
}
