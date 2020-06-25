/**
 *
 * Run elemnts api header
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
#ifndef __RUN_ELEMENTS_API_HEADER
#define __RUN_ELEMENTS_API_HEADER

extern "C"
{
    typedef int (start_fep_elements_f)(const int domain_id, const int nElements, const char* pElementNameArgs[]);
    typedef int (stop_fep_elements_f)();
}

#endif // __RUN_ELEMENTS_API_HEADER

