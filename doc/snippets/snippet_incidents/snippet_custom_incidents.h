/**
 * Declaration of user-defined incidents.
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

#ifndef _FEP_SNIPPET_USER_DEFINED_INCIDENTS_
#define _FEP_SNIPPET_USER_DEFINED_INCIDENTS_

namespace fep
{
    //! [CustomIncidCodeDef]
    // Definition of several custom incident codes.
    typedef enum cCustomIncidents
    {
        CI_CriticalHalt = -100,            //!< Arbitrary incident indicating a critical halt
        CI_CriticalFailSilent = -101,      //!< Arbitrary incident indicating a fail silent handling
        CI_CriticalFailSafe = -102,        //!< Arbitrary incident indicating a fail safe handling
        CI_UncriticalError = -103,         //!< Arbitrary critical incident which is being ignored.
        CI_DatabaseFileNotFound = -104,    //!< Custom Incident indicating a missing file.
        CI_RealtimeViolation = -105        //!< Custom Incident indicating a realtime violation warning during runtime
    } tCustomIncidents;
    //! [CustomIncidCodeDef]
}
#endif // _FEP_SNIPPET_USER_DEFINED_INCIDENTS_
