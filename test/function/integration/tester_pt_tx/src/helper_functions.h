/**
 * Implementation of the helper functions for the tester for the integration of
 * FEP PropertyTree with FEP Transmission Adapter
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
#ifndef _TESTER_INTEGRATION_PT_TX_HELPER_H_
#define _TESTER_INTEGRATION_PT_TX_HELPER_H_
inline std::string make_big_string(const size_t& xSize)
{
    std::string strRes;

    for (size_t i= 0; i< xSize; ++i)
    {
        strRes.push_back('A' + (rand() % 26));
    }

    return strRes;
}

#endif