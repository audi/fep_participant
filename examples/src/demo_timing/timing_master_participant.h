/**
 * Implementation of the timing demo
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
 */

#ifndef _EXAMPLE_TIMING_MASTER_H_
#define _EXAMPLE_TIMING_MASTER_H_

#include <fep_participant_sdk.h>

/// Available timing master mode
enum MasterMode
{
    UNDEFINED_MODE = -1,
    AFAP_MODE = 0,
    SYSTEM_TIME_MODE = 1,
    EXTERNAL_CLOCK_MODE = 2,
    USER_IMPLEMENTATION_MODE = 3
};

/// Manual Step Trigger
/// Trigger next step by calling a method
class cManualStepTrigger : public fep::IStepTrigger
{
public:
    cManualStepTrigger();

public: // implement fep::IStepTrigger
    fep::Result RegisterTrigger(const timestamp_t cycle_time, fep::IStepTriggerListener* pStepTriggerListener) override;
    fep::Result UnregisterTrigger(fep::IStepTriggerListener* pStepTriggerListener) override;

public: // Support manual step
    timestamp_t triggerNextStep();

private:
    /// listener registered
    fep::IStepTriggerListener* _step_trigger_listener;
};

/// FEP Timing Master
class cTimingMasterElement : public fep::cModule
{
public:
    cTimingMasterElement(MasterMode eMode, double fScale);
    ~cTimingMasterElement();

public: // overrides cModule / cStateEntryListener
    fep::Result ProcessStartupEntry(const fep::tState eOldState) override;
    fep::Result ProcessInitializingEntry(const fep::tState eOldState) override;

public: // Support manual step
    timestamp_t triggerNextStep();

private:
    /// Trigger mode
    MasterMode m_eMode;
    /// Manual step trigger (only used if m_eMode == USER_IMPLEMENTATION_MODE)
    cManualStepTrigger m_oManualStepTrigger;
    /// time scale factor (only used if m_eMode == SYSTEM_TIME_MODE)
    double m_fScale;
};

#endif // _EXAMPLE_TIMING_MASTER_H_
