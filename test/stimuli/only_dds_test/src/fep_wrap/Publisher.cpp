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

PublisherSample::PublisherSample(Publisher& publisher, const std::size_t sample_size)
{
    fep::Result result;
    
    result= publisher.m_my_participant.GetUserDataAccess()->CreateUserDataSample(m_poSample, publisher.m_hSignalHandle);
    assert(fep::isOk(result));

    result = m_poSample->SetSize(sample_size);
    assert(fep::isOk(result));
}

PublisherSample::~PublisherSample()
{
    delete m_poSample;
}

Publisher::Publisher(const DdsConfig& dds_config, Participant& my_participant, Topic& my_topic)
    : m_my_participant(my_participant)
    , m_my_topic(my_topic)
{
    fep::cUserSignalOptions oUserSignalOptions;
    oUserSignalOptions.SetAsyncPublisher(dds_config.async_mode_enable);
    oUserSignalOptions.SetLowLatencyProfile(true);
    oUserSignalOptions.SetReliability(dds_config.reliability_mode == ReliableReliabilityMode);
    oUserSignalOptions.SetSignalDirection(fep::SD_Output);
    oUserSignalOptions.SetSignalRaw();
    oUserSignalOptions.SetSignalName(m_my_topic.getTopicName().c_str());

    fep::Result result = m_my_participant.GetSignalRegistry()->RegisterSignal(oUserSignalOptions, m_hSignalHandle);
    assert(fep::isOk(result));
}

Publisher::~Publisher()
{
    fep::Result result = m_my_participant.GetSignalRegistry()->UnregisterSignal(m_hSignalHandle);
    assert(fep::isOk(result));
}

void Publisher::Transmit(PublisherSample* data_sample)
{
    fep::Result result = m_my_participant.GetUserDataAccess()->TransmitData(data_sample->m_poSample, true);
    assert(fep::isOk(result));
}
