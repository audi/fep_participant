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
#ifndef FEP_INCIDENT_HANDLER_COMMON_H
#define FEP_INCIDENT_HANDLER_COMMON_H 

#include <cstddef>
#include <cstdint>
#include <string>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "incident_handler/fep_severity_level.h"

namespace fep
{

/**
    * @brief formatIncidentString
    *
    *  Internal Helper-function to create a unified formated incident string message
    *  for diffrent handling strategies
    *
    * @param [in] incidentString std::string that will contain the formated string.
    * @param [in] strSource The name of the issuing FEP Element.
    * @param [in] strHostname Hostname of the incidents host.
    * @param [in] nIncident The invoked incident code.
    * @param [in] severity The severity level of the received incident.
    * @param [in] strDescription An description of the invoked incident.
    * @param [in] strOrigin The module that invoked the incident. This is NOT the fep element.
    * @param [in] nLine The line from where the incident was invoked
    * @param [in] tmSimTime Time stamp of the simulation time at the incident invokation.
    * @param [in] strFile The filepath of the incident invoking file
    *
    * @return ERR_NOERROR Everything went fine.
    * @return ERR_INVALID_ARG severity invalid
    */
fep::Result formatIncidentString(std::string &incidentString, const char *strSource,
                             const char *strHostname, const int16_t nIncident,
                             const  tSeverityLevel severity,
                             const char* strOrigin,
                             int nLine,
                             const char* strFile,
                             const timestamp_t tmSimTime,
                             const char* strDescription);


/**
    * @brief formatIncidentStringCSV
    *
    *  Internal Helper-function to create a unified csv formated incident string message
    *  for diffrent handling strategies
    *
    * @param [in] incidentString std::string that will contain the formated string.
    * @param [in] strSource The name of the issuing FEP Element.
    * @param [in] strHostname Hostname of the incidents host.
    * @param [in] nIncident The invoked incident code.
    * @param [in] severity The severity level of the received incident.
    * @param [in] strDescription An description of the invoked incident.
    * @param [in] strOrigin The module that invoked the incident. This is NOT the fep element.
    * @param [in] nLine The line from where the incident was invoked
    * @param [in] tmSimTime Time stamp of the simulation time at the incident invokation.
    * @param [in] strFile The filepath of the incident invoking file
    *
    * @return ERR_NOERROR Everything went fine.
    * @return ERR_INVALID_ARG severity invalid
    */
fep::Result formatIncidentStringCSV(std::string &incidentString, const char *strSource,
                                const char *strHostname, const int16_t nIncident,
                                const  tSeverityLevel severity,
                                const char* strOrigin,
                                int nLine,
                                const char* strFile,
                                const timestamp_t tmSimTime,
                                const char* strDescription);
}

#define INVOKE_INCIDENT(Code,Serv,Descr)\
    InvokeIncident(Code,Serv,Descr,"IncidentHandler",0, NULL)

#endif // FEP_INCIDENT_HANDLER_COMMON_H
