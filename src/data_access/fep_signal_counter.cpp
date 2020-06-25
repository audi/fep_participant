/**
* Implementation of the Class cSignalCounter.
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

#include <cstddef>
#include <mutex>
#include <string>
#include <a_util/result/result_type.h>

#include "fep_errors.h"
#include "fep_result_decl.h"
#include "signal_registry/fep_signal_struct.h"
#include "transmission_adapter/fep_preparation_data_sample_intf.h"
#include "transmission_adapter/fep_transmission_adapter_intf.h"
#include "data_access/fep_signal_counter.h"

namespace fep {

class IPropertyTreeBase;
using namespace component_config;

/* public *********************************************************************/
cSignalCounter::cSignalCounter()
    : m_poTransmissionAdapter(NULL),
    m_poPropertyTree(NULL),
    m_oStrSignalName(""),
    m_hSignalHandle(NULL),
    m_bPrevSync(true),
    m_nFrameId(0),
    m_nSampleNumber(0)
{
}

cSignalCounter::~cSignalCounter()
{
    /* clean up */
    UnregisterSignal();

    /* "foreign" objects must not be deleted here => just set to NULL */
    m_poTransmissionAdapter = NULL;
    m_poPropertyTree = NULL;
}

fep::Result cSignalCounter::Create(const tSignal& oSignal,
    fep::IPropertyTreeBase * poPropertyTree, fep::ITransmissionAdapter * poTransmissionAdapter)
{
    if (!poTransmissionAdapter ||
        !poPropertyTree ||
        oSignal.strSignalName.empty())
    {
        return ERR_POINTER;
    }

    /* store all references we need */
    m_poPropertyTree = poPropertyTree;
    m_poTransmissionAdapter = poTransmissionAdapter;
    m_oStrSignalName.append(oSignal.strSignalName);

    /* register signal to "real" transmission adapter */
    RETURN_IF_FAILED(m_poTransmissionAdapter->RegisterSignal(oSignal,
        m_hSignalHandle));

    /* set up the sample counter */
    ConfigureSampleCounter();

    return ERR_NOERROR;
}

handle_t cSignalCounter::GetInternalSignalHandle() const
{
    return m_hSignalHandle;
}

fep::Result cSignalCounter::UnregisterSignal()
{
    /* we need to check for NULL to avoid crash in case Create() failed */
    if (!m_hSignalHandle) { return ERR_POINTER; }
    return m_poTransmissionAdapter->UnregisterSignal(m_hSignalHandle);
}

fep::Result cSignalCounter::ConfigureSampleCounter()
{
    m_bPrevSync = true;
    // frame count starts at 1 - sample count starts at 0
    m_nFrameId = 1;
    m_nSampleNumber = 0;
    return ERR_NOERROR;
}

fep::Result cSignalCounter::SendNow(IPreparationDataSample *poPreparationSample)
{
    /* this private method is only used in a way, that the given argument is
    * never NULL - thus we can save the time and do not need to check it */
    std::unique_lock<std::recursive_mutex> oSyncGuard(m_csSendProtection);
    m_bPrevSync = poPreparationSample->GetSyncFlag();
    poPreparationSample->SetFrameId(m_nFrameId);
    poPreparationSample->SetSampleNumberInFrame(m_nSampleNumber);
    handle_t hExtHandle = poPreparationSample->GetSignalHandle();
    poPreparationSample->SetSignalHandle(m_hSignalHandle);
    fep::Result nResult = m_poTransmissionAdapter->TransmitData(poPreparationSample);
    poPreparationSample->SetSignalHandle(hExtHandle);
    if (fep::isOk(nResult))
    {
        if (m_bPrevSync)
        {
            m_nSampleNumber = 0;
            m_nFrameId++;
        }
        else
        {
            m_nSampleNumber++;
        }
    }
    else
    {
        // failed transmits are not counted because the transmission adapter already reports
        // failure by means of incident and the frame counting shall detect reception errors
    }

    return nResult;
}

}  // namespace fep
