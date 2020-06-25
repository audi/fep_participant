/**
* Implementation of adapted state machine mockup used by this test
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

#ifndef __FEP_MY_MOCK_INCIDENT_HANDLER_H_
#define __FEP_MY_MOCK_INCIDENT_HANDLER_H_

#include "function/_common/fep_mock_incident_handler.h"

class cMyMockIncidentHandler : public cMockUpIncidentHandler
{
public:
    cMyMockIncidentHandler()
        : _incidentReceived(false)
        , m_strDesc()
        , m_nCode(0)
        , m_eSeverity(SL_Info)
    { }
    ~cMyMockIncidentHandler()
    { }
    fep::Result InvokeIncident(int16_t nFEPIncident,
        fep::tSeverityLevel eSeverity,
        const char* strDescription,
        const char* strOrigin,
        int nLine,
        const char* strFile)
    {
        _incidentReceived = true;
        m_strDesc = strDescription;
        m_nCode = nFEPIncident;
        m_eSeverity = eSeverity;
        return ERR_NOERROR;
    }
public:
    bool _incidentReceived;
    std::string m_strDesc;
    int16_t m_nCode;
    fep::eFEPSeverityLevel m_eSeverity;
};

#endif // __FEP_MY_MOCK_INCIDENT_HANDLER_H_