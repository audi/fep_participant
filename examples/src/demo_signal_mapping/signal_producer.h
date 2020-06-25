/**
 * Implementation of the signal mapping demo
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

#ifndef _EXAMPLE_SIGNAL_MAPPING_PRODUCER_H_
#define _EXAMPLE_SIGNAL_MAPPING_PRODUCER_H_

#include <fep_participant_sdk.h>
#include <a_util/system/timer.h>

class cSignalProducer : public fep::cModule
{
public:
    virtual ~cSignalProducer();
    void RunCyclic();

public: // overrides cModule / cStateEntryListener
    fep::Result ProcessStartupEntry(const fep::tState eOldState);
    fep::Result ProcessIdleEntry(const fep::tState eOldState);
    fep::Result ProcessInitializingEntry(const fep::tState eOldState);
    fep::Result ProcessRunningEntry(const fep::tState eOldState);
    fep::Result ProcessErrorEntry(const fep::tState eOldState);

private:
    handle_t m_hOrientation;
    fep::IUserDataSample* m_pSampleOrientation;
    handle_t m_hPosition;
    fep::IUserDataSample* m_pSamplePosition;
    handle_t m_hObject;
    fep::IUserDataSample* m_pSampleObject;
    int m_nSampleCounter;
    a_util::system::Timer m_oTimer;
};

#endif
