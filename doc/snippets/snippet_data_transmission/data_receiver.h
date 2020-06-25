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
//! [Receiving participant]
#include <fep_participant_sdk.h>
#include "data_processor.h"

using namespace fep;

class cDataReceptionElement : public cModule, public cStateExitListener
{
public:
    cDataReceptionElement();
    fep::Result ProcessIdleEntry(const fep::tState eOldState);
    fep::Result ProcessInitializingEntry(const fep::tState eOldState);
    fep::Result ProcessStartupEntry(const fep::tState eOldState);
    
private:
    handle_t m_hSignalDDL;
    handle_t m_hSignalRAW;
    cDataProcessor m_oDDLDataProcessor;
    cDataProcessor m_oRawDataProcessor;
};
//! [Receiving participant]