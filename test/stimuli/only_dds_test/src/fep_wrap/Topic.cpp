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
#include "fep_wrap.h"


#include <assert.h>

using namespace fep_wrap;

Topic::Topic(Participant& my_participant, const std::string& topic_name)
    : m_my_participant(my_participant)
    , m_dds_topic(topic_name)
{
}

Topic::~Topic()
{
}
