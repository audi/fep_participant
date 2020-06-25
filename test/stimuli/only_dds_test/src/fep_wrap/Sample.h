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
#pragma once

#include <cstdint>
#include "Config.h"

typedef int64_t timestamp_t;

#pragma pack(push, 1)
// Total size is 16 Bytes
struct sSampleHeader
{
    uint32_t sample_number;
    uint16_t client_id;
    uint16_t server_id;
    timestamp_t usr_pub_send_timestamp;
    timestamp_t usr_sub_recv_timestamp;
    timestamp_t usr_sub_send_timestamp;
    timestamp_t llv_sub_recv_timestamp;
    timestamp_t dds_pub_send_timestamp;
    timestamp_t dds_sub_recv_timestamp;
};
#pragma pack(pop)

//static const std::size_t MAX_SAMPLE_SIZE = MAX_PAYLOAD_SIZE - sizeof(sSampleHeader);
