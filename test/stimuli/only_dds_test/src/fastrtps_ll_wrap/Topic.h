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

#include <string>

#include "fastrtps/attributes/TopicAttributes.h"

namespace fastrtps_ll_wrap
{
    class Participant;
    class Publisher;
    class Subscriber;

    class Topic
    {
        friend class Publisher;
        friend class Subscriber;

    public:
        Topic(Participant& my_participant, const std::string& topic_name);
        ~Topic();

    private:
        Participant& m_my_participant;
        eprosima::fastrtps::TopicAttributes m_topic_attributes;
    };
}
