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
#include "fastrtps_hl_wrap.h"


#include <assert.h>

using namespace fastrtps_hl_wrap;
using namespace eprosima::fastrtps;

Topic::Topic(fastrtps_hl_wrap::Participant& my_participant, const std::string& topic_name)
    : m_my_participant(my_participant)
{
    m_topic_attributes.topicKind = NO_KEY; // This is fixed. Topics are not used
    m_topic_attributes.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    m_topic_attributes.historyQos.depth = 1;

    m_topic_attributes.topicDataType = "OctetType";
    m_topic_attributes.topicName = topic_name;
}

Topic::~Topic()
{
}

