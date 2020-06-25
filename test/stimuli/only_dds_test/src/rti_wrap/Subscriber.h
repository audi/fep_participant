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

#include "ndds/ndds_cpp.h"

namespace rti_wrap
{
    class Topic;
    class Participant;

    class SubscriberSample
    {
    public:
        DDS_OctetsSeq m_dds_octet_seq;
        DDS_SampleInfoSeq m_info_seq;
    };

    struct SampleInfo
    {
        timestamp_t dds_source_timestamp;
        timestamp_t dds_reception_timestamp;
        timestamp_t llv_reception_timestamp;
    };

    class ISubscriberListener
    {
    public:
        virtual ~ISubscriberListener() { }

    public:
        virtual void receivedSample(const void* data, const int data_size, const SampleInfo& sample_info) = 0;
    };
}

#if RTI_DDS_USE_WAITSET
#include "SubscriberWithWaitset.h"
#else
#include "SubscriberWithQueue.h"    
#endif
