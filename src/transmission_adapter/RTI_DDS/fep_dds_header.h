/**

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
 */
#ifndef _FEP_DDS_HEADER_H_
#define _FEP_DDS_HEADER_H_

#define DDS_DRIVER_PROTOCOL_VERSION 3
#define DDS_MAX_PACKET_SIZE 63000
#define FRAGMENTATION_BOUNDARY DDS_MAX_PACKET_SIZE - sizeof(fep::Fragment)

#endif //_FEP_DDS_HEADER_H_
