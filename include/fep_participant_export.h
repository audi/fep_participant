/**
 * Export header for the FEP SDK.
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
 
#ifndef _FEP_SDK_EXPORT_H_INCLUDED_
#define _FEP_SDK_EXPORT_H_INCLUDED_

#include "fep_sdk_participant_version.h"    // #define FEP_SDK_PARTICIPANT_SHARED_LIB ...WTF!

#ifdef FEP_SDK_PARTICIPANT_SHARED_LIB
#ifdef WIN32
    #ifdef FEP_SDK_PARTICIPANT_DO_EXPORT
        /// Macro switching between export / import of the fep participant shared object
        #define FEP_PARTICIPANT_EXPORT __declspec( dllexport )
        #define FEP_PARTICIPANT_DEPRECATED
    #else   // FEP_SDK_PARTICIPANT_DO_EXPORT
        /// Macro switching between export / import of the fep participant shared object
        #define FEP_PARTICIPANT_EXPORT __declspec( dllimport )
        #define FEP_PARTICIPANT_DEPRECATED __declspec( deprecated )
    #endif
#else   // WIN32
    #ifdef FEP_SDK_PARTICIPANT_DO_EXPORT
        /// Macro switching between export / import of the fep participant shared object
        #define FEP_PARTICIPANT_EXPORT __attribute__ ((visibility("default")))
        #define FEP_PARTICIPANT_DEPRECATED
    #else   // FEP_SDK_PARTICIPANT_DO_EXPORT
        /// Macro switching between export / import of the fep participant shared object
        #define FEP_PARTICIPANT_EXPORT
        #define FEP_PARTICIPANT_DEPRECATED __attribute__ ((deprecated))
    #endif
#endif
#else
// No need to export in a static library
#ifdef WIN32
#define FEP_PARTICIPANT_EXPORT
#define FEP_PARTICIPANT_DEPRECATED __declspec( deprecated )
#else
#define FEP_PARTICIPANT_EXPORT
#define FEP_PARTICIPANT_DEPRECATED __attribute__ ((deprecated)) 
#endif
#endif

#endif // _FEP_SDK_EXPORT_H_INCLUDED_
