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

#include <a_util/datetime/datetime.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_format.h>

#include "fep_errors.h"
#include "fep_incident_handler_common.h"

using namespace fep;

fep::Result fep::formatIncidentString(std::string &incidentString, const char *strSource,
                                  const char *strHostname, const int16_t nIncident,
                                  const  tSeverityLevel severity,
                                  const char* strOrigin,
                                  int nLine,
                                  const char* strFile,
                                  const timestamp_t tmSimTime,
                                  const char* strDescription)
{
    fep::Result nResult = ERR_NOERROR;
    incidentString.append(a_util::strings::format("[%s - %s]: ",
                                a_util::datetime::getCurrentLocalDate().format("%d.%m.%Y").c_str(),
                                a_util::datetime::getCurrentLocalTime().format("%H:%M:%S").c_str()));

    incidentString.append(a_util::strings::format("%s@%s ", strSource, strHostname));
    incidentString.append(a_util::strings::format(" ST: %d[us]  ",tmSimTime));

#ifndef _DEBUG
    if(NULL != strOrigin){
        incidentString.append(a_util::strings::format("[%s] ",
                                    strOrigin));
    }
#else
    if(NULL != strOrigin){
        incidentString.append(a_util::strings::format("[%s",
                                    strOrigin));
        if(NULL != strFile && 0 <= nLine )
        {
            incidentString.append(a_util::strings::format(" (%s:%d)] ",strFile,nLine));
        }else{
            incidentString.append("] ");
        }
    }
#endif
    switch(severity)
    {
    case SL_Info:
        incidentString.append("Info");
        break;
    case SL_Warning:
        incidentString.append("Warning");
        break;
    case SL_Critical:
        incidentString.append("Critical");
        break;
    default:
        incidentString.append("<Unknown>");
        nResult = ERR_INVALID_ARG;
        break;
    }
    incidentString.append(a_util::strings::format(" %d: %s \n",
                                nIncident, strDescription));
    return nResult;
}


fep::Result fep::formatIncidentStringCSV(std::string &incidentString, const char *strSource,
                                     const char *strHostname, const int16_t nIncident,
                                     const  tSeverityLevel severity,
                                     const char* strOrigin,
                                     int nLine,
                                     const char* strFile,
                                     const timestamp_t tmSimTime,
                                     const char* strDescription
                                     )
{
    fep::Result nResult = ERR_NOERROR;
    incidentString.append(a_util::strings::format("%s,%s,%s,%s",
                                a_util::datetime::getCurrentLocalDate().format("%d.%m.%Y").c_str(),
                                a_util::datetime::getCurrentLocalTime().format("%H:%M:%S").c_str(),
                                strHostname,
                                strSource));

    incidentString.append(a_util::strings::format(",%d", tmSimTime));

    if(NULL != strOrigin){
        incidentString.append(a_util::strings::format(",%s",strOrigin));
    }
#ifdef _DEBUG
    if(NULL != strFile && 0 <= nLine )
    {
        incidentString.append(a_util::strings::format(",%d,",nLine));
        incidentString.append(strFile);
    }
#endif

    switch(severity)
    {
    case SL_Info:
        incidentString.append(",Info");
        break;
    case SL_Warning:
        incidentString.append(",Warning");
        break;
    case SL_Critical:
        incidentString.append(",Critical");
        break;
    default:
        incidentString.append(",<Unknown>");
        nResult = ERR_INVALID_ARG;
        break;
    }

    incidentString.append(a_util::strings::format(",%d,%s\n", nIncident, strDescription));
    return nResult;
}
