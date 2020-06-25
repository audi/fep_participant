/**
 * Declaration of the Class cState and enumeration tState.
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

#if !defined(EA_CB29D3B4_A302_466a_B8AB_4D213BB672EF__INCLUDED_)
#define EA_CB29D3B4_A302_466a_B8AB_4D213BB672EF__INCLUDED_

namespace fep
{
    /**
     * Enumeration representing the available states in FEP.
     * See \ref fep_state_machine for more information about states.
     */
    enum tState
    /* see "rules for changing an enumeration" (#27200) before doing any change! */
    {
        /// The STARTUP state.
        FS_STARTUP = 0,
        /// The IDLE state.
        FS_IDLE = 1,
        /// The INITIALIZING state.
        FS_INITIALIZING = 2,
        /// The READY state.
        FS_READY = 3,
        /// The RUNNING state.
        FS_RUNNING = 4,
        /// The ERROR state.
        FS_ERROR = 5,
        /// The SHUTDOWN state
        FS_SHUTDOWN = 6,
        /// The UNKNOWN state (only remote states can be unknown e.g. GetRemoteState timed out)
        FS_UNKNOWN = 7
    };

}
#endif // !defined(EA_CB29D3B4_A302_466a_B8AB_4D213BB672EF__INCLUDED_)
