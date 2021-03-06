/**
* Declaration of build-in configurations paths of the FEP Framework.
* Declaration of the FEP participant header paths.
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
*
* @file
*/

#ifndef _MODULE_HEADER_CONFIG_H_
#define _MODULE_HEADER_CONFIG_H_

#include "fep_participant_export.h"

namespace fep
{

    /**********************************************************************************/
    /* ANY CHANGES MADE HERE MUST BE APPLIED TO src/doxygen/fep_configs.doxygen ASWELL*/
    /**********************************************************************************/


    //@{
    /// Property base path of all participant header properties
    #define FEP_PARTICIPANT_HEADER "Header"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderBase;
    //@}
    //@{
    /// participant header property path for element version
    #define FEP_PARTICIPANT_HEADER_VERSION FEP_PARTICIPANT_HEADER".ElementVersion"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_fElementVersion;
    //@}
    //@{
    /// participant header property path for element name
    #define FEP_PARTICIPANT_HEADER_NAME FEP_PARTICIPANT_HEADER".ElementName"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementName;
    //@}
    //@{
    /// participant header property path for element description
    #define FEP_PARTICIPANT_HEADER_DESCRIPTION FEP_PARTICIPANT_HEADER".ElementDescription"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementDescription;
    //@}
    //@{
    /// participant header property path for FEP version
    #define FEP_PARTICIPANT_HEADER_FEP_VERSION  FEP_PARTICIPANT_HEADER".FEPVersion"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_fFEPVersion;
    //@}
    //@{
    /// participant header property path for element's platform
    #define FEP_PARTICIPANT_HEADER_PLATFORM  FEP_PARTICIPANT_HEADER".Platform"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementPlatform;
    //@}
    //@{
    /// participant header property path for element's context
    #define FEP_PARTICIPANT_HEADER_CONTEXT FEP_PARTICIPANT_HEADER".ElementContext"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementContext;
    //@}
    //@{
    /// participant header property path for element's context's version
    #define FEP_PARTICIPANT_HEADER_CONTEXT_VERSION FEP_PARTICIPANT_HEADER".ElementContextVersion"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_fElementContextVersion;
    //@}
    //@{
    /// participant header property path for element vendor
    #define FEP_PARTICIPANT_HEADER_VENDOR FEP_PARTICIPANT_HEADER".ElementVendor"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementVendor;
    //@}
    //@{
    /// participant header property path for element display name
    #define FEP_PARTICIPANT_HEADER_DISPLAY_NAME FEP_PARTICIPANT_HEADER".ElementDisplayName"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementDisplayName;
    //@}
    //@{
    /// participant header property path for element compilation date
    #define FEP_PARTICIPANT_HEADER_COMPILATION_DATE FEP_PARTICIPANT_HEADER".ElementCompilationDate"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementCompilationDate;
    //@}
    //@{
    /// participant header property path for current element state
    #define FEP_PARTICIPANT_HEADER_CURRENT_STATE FEP_PARTICIPANT_HEADER".CurrentState"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementCurrentState;
    //@}
    //@{
    /// participant header field name for current element state
    #define FEP_PARTICIPANT_HEADER_CURRENT_STATE_FIELD "CurrentState"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderField_strElementCurrentState;
    //@}
    //@{
    /// participant header property path for the hostname of the element
    #define FEP_PARTICIPANT_HEADER_HOST FEP_PARTICIPANT_HEADER".Host"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementHost;
    //@}
    //@{
    /// participant header property path for the autogenerated uuid of the element
    #define FEP_PARTICIPANT_HEADER_INSTANCE_ID FEP_PARTICIPANT_HEADER".InstanceID"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strInstanceID;
    //@}
    //@{
    /// participant header property path for the author defined uuid of the element
    #define FEP_PARTICIPANT_HEADER_TYPE_ID FEP_PARTICIPANT_HEADER".TypeID"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strTypeID;
    //@}
    //@{
    /// participant header property path for current muting state of the element
    #define FEP_PARTICIPANT_HEADER_GLOBAL_MUTE FEP_PARTICIPANT_HEADER".GlobalMute"
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_bGlobalMute;
    //@}
}

#endif //_MODULE_HEADER_CONFIG_H_
