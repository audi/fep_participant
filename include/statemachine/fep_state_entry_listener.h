/**
 * Declaration of the Class cStateEntryListener.
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

#if !defined(EA_3676C217_6582_481d_A852_C461C640F2A7__INCLUDED_)
#define EA_3676C217_6582_481d_A852_C461C640F2A7__INCLUDED_

#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "fep_state_entry_listener_intf.h"
#include "fep3/base/states/fep2_state.h"

namespace fep
{
    /**
     * Empty implementation of IStateEntryListener for convenient inheritance
     */
    class FEP_PARTICIPANT_EXPORT cStateEntryListener : 
        public fep::IStateEntryListener
    {
    public:
        /// Default constructor
        cStateEntryListener();
        /// Default destructor
        virtual ~cStateEntryListener();

    public: // implements IStateEntryListener
        /**
         * Empty implementation - does nothing and returns ERR_NOERROR
         * @retval ERR_NOERROR Always
         */
        virtual fep::Result CleanUp(const fep::tState);
        /**
         * Empty implementation - does nothing and returns ERR_NOERROR
         * @retval ERR_NOERROR Always
         */
        virtual fep::Result ProcessStartupEntry(const fep::tState);
        /**
         * Empty implementation - does nothing and returns ERR_NOERROR
         * @retval ERR_NOERROR Always
         */
        virtual fep::Result ProcessIdleEntry(const fep::tState);
        /**
         * Empty implementation - does nothing and returns ERR_NOERROR
         * @retval ERR_NOERROR Always
         */
        virtual fep::Result ProcessInitializingEntry(const fep::tState);
        /**
         * Empty implementation - does nothing and returns ERR_NOERROR
         * @retval ERR_NOERROR Always
         */
        virtual fep::Result ProcessReadyEntry(const fep::tState);
        /**
         * Empty implementation - does nothing and returns ERR_NOERROR
         * @retval ERR_NOERROR Always
         */
        virtual fep::Result ProcessRunningEntry(const fep::tState);
        /**
         * Empty implementation - does nothing and returns ERR_NOERROR
         * @retval ERR_NOERROR Always
         */
        virtual fep::Result ProcessShutdownEntry(const fep::tState);
        /**
         * Empty implementation - does nothing and returns ERR_NOERROR
         * @retval ERR_NOERROR Always
         */
        virtual fep::Result ProcessErrorEntry(const fep::tState);
    };
} /* namespace fep */
#endif // !defined(EA_3676C217_6582_481d_A852_C461C640F2A7__INCLUDED_)
