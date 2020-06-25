/**
 *
 * Bus Compat Stimuli: Bus Check for Custom Command
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

#ifndef _BUS_COMPAT_STIMULI_BUS_CHECK_INCIDENTS_H_INCLUDED_
#define _BUS_COMPAT_STIMULI_BUS_CHECK_INCIDENTS_H_INCLUDED_

#include "stdafx.h"
#include "bus_check_base.h"


class cBusCheckIncidents : public cBusCheckBase
{
public:
    cBusCheckIncidents(const int16_t nFEPIncident, const fep::tSeverityLevel eSeverity, const char* strDescription, const int nLine, const char* strFile);

protected:  // implements fep::cNotificationListener
    fep::Result Update(fep::IIncidentNotification const * poNotification); 

private:
    fep::Result DoSend();
    
private:
    int16_t m_nFEPIncident;
    fep::tSeverityLevel m_eSeverity;
    std::string m_strDescription;
    int m_nLine;
    std::string m_strFile;
};

#endif // _BUS_COMPAT_STIMULI_BUS_CHECK_INCIDENTS_H_INCLUDED_
