/************************************************************************
 * Comon declarations used by integration testing
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

#ifndef _TEST_TIMING_COMMON_H_INCLUDED
#define _TEST_TIMING_COMMON_H_INCLUDED

#pragma pack(push,1)
struct tRequest
{
    int64_t request_value;
};
#pragma pack(pop)

#pragma pack(push,1)
struct tResponse
{
    int64_t response_value;
};
#pragma pack(pop)

#define REQUEST_SIGNAL_NAME_PROPERTY "ElementConfig.strRequestSignalName"
#define RESPONSE_SIGNAL_NAME_PROPERTY "ElementConfig.strResponseSignalName"

#define DUMMY_ELEMENT_NUMBER_OF_STEPS_PROPERTY "ElementConfig.DummyElement.nNumberOfSteps"

#endif // _TEST_TIMING_COMMON_H_INCLUDED
