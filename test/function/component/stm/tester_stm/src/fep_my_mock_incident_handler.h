/**
 * Implementation of adapted signal mapping mockup used by this test
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

#ifndef _FEP_TEST_MY_MOCK_INCIDENT_HANDLER_H_INC_
#define _FEP_TEST_MY_MOCK_INCIDENT_HANDLER_H_INC_

#include "function/_common/fep_mock_incident_handler.h"

using namespace fep;

class cMyMockIncidentHandler : public cMockUpIncidentHandler
{
public:
    struct incidentContainer
    {
        int16_t nFEPIncident;
        fep::tSeverityLevel eSeverity;
        std::string strDesc;
        std::string strOrigin;
    };

public:
    virtual fep::Result InvokeIncident(int16_t nFEPIncident,
        fep::tSeverityLevel eSeverity,
        const char* strDescription,
        const char* strOrigin,
        int nLine,
        const char* strFile) {

        m_oIncident.nFEPIncident = nFEPIncident;
        m_oIncident.eSeverity = eSeverity;
        m_oIncident.strDesc = strDescription;
        m_oIncident.strOrigin = strOrigin;

        return ERR_NOERROR;
    }

public:
    const incidentContainer& GetLastIncident() const
    {
        return m_oIncident; 
    }

public:
    incidentContainer m_oIncident;
};

#endif // _FEP_TEST_MY_MOCK_INCIDENT_HANDLER_H_INC_
