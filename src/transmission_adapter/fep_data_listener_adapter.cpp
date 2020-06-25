/**
 * Implementation of the Class cDataListenerAdapter.
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

#include <cassert>
#include <cstddef>
#include <string>
#include <a_util/base/types.h>
#include <a_util/result/result_type.h>

#include "fep_transmission_adapter_common.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "transmission_adapter/fep_data_listener_adapter.h"
#include "transmission_adapter/fep_preparation_data_sample_intf.h"
#include "transmission_adapter/fep_user_data_listener_intf.h"

using namespace fep;

static const std::string s_strIncidentDescription("A missing data sample was detected");

cDataListenerAdapter::cDataListenerAdapter(IUserDataListener * poUserDataListener,
    IIncidentHandler* poIncidentHandler,
    handle_t hSignal):
    m_nMissedDataSamples(0),
    m_bPrevSyncFlag(true),
    m_nPrevFrameId(0),
    m_nPrevSampleNumber(0)
{
    m_poUserDataListener = poUserDataListener;
    m_poIncidentHandler = poIncidentHandler;
    m_hSignal = hSignal;

    assert(NULL != m_poIncidentHandler);
}

cDataListenerAdapter::~cDataListenerAdapter()
{
    m_poUserDataListener = NULL;
    m_poIncidentHandler  = NULL;
    m_hSignal = NULL;
}

fep::Result cDataListenerAdapter::Update(const IPreparationDataSample* poPreparationSample)
{
    //    Skipped complete frame
    // or Skipped sample within a frame
    // or Missed data sample with sync flag
    if ( (poPreparationSample->GetFrameId() > (m_nPrevFrameId + 1))
      || (poPreparationSample->GetSampleNumberInFrame() > (m_nPrevSampleNumber + 1))
      || ( (poPreparationSample->GetFrameId() > m_nPrevFrameId)
           && !m_bPrevSyncFlag) )
    {
        m_nMissedDataSamples++;
        INVOKE_INCIDENT(m_poIncidentHandler,FSI_TRANSM_RX_MISSING_DATASAMPLE, SL_Warning,
                                            s_strIncidentDescription.c_str());
    }
    m_bPrevSyncFlag = poPreparationSample->GetSyncFlag();
    m_nPrevFrameId = poPreparationSample->GetFrameId();
    m_nPrevSampleNumber = poPreparationSample->GetSampleNumberInFrame();

    return m_poUserDataListener->Update(poPreparationSample);
}

fep::IUserDataListener * cDataListenerAdapter::GetUserDataListener()
{

    return m_poUserDataListener;
}

handle_t cDataListenerAdapter::GetSignalHandle()
{
    return m_hSignal;
}
