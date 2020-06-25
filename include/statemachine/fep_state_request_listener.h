/**
 * Declaration of the Class cStateRequestListener.
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

#if !defined(EA_BBBF066B_15A0_4cdc_AC0D_586FD247B402__INCLUDED_)
#define EA_BBBF066B_15A0_4cdc_AC0D_586FD247B402__INCLUDED_

#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "fep3/base/states/fep2_state.h"
#include "statemachine/fep_state_request_listener_intf.h"

namespace fep
{
    /**
     * Base implementation of IStateRequestListener
     */
    class FEP_PARTICIPANT_EXPORT cStateRequestListener :
            public fep::IStateRequestListener
    {
    public:
        /// Default constructor
        cStateRequestListener();
        /// Default destructor
        virtual ~cStateRequestListener();

    public: // implements IStateRequestListener
        virtual fep::Result ProcessIdleRequest(const fep::tState eOldState);
        virtual fep::Result ProcessInitializingRequest(const fep::tState eOldState);
        virtual fep::Result ProcessReadyRequest(const fep::tState eOldState);
        virtual fep::Result ProcessRunningRequest(const fep::tState eOldState);
        virtual fep::Result ProcessShutdownRequest(const fep::tState eOldState);
        virtual fep::Result ProcessErrorRequest(const fep::tState eOldState);
    };
}
#endif // !defined(EA_BBBF066B_15A0_4cdc_AC0D_586FD247B402__INCLUDED_)
