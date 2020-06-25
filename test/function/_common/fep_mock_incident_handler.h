/**
 * Implementation of incident handler mockup used by FEP functional test cases!
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

#ifndef _FEP_TEST_MOCK_INCIDENTHANDLER_H_INC_
#define _FEP_TEST_MOCK_INCIDENTHANDLER_H_INC_

class cMockUpIncidentHandler : public fep::IIncidentHandler
{
public:
    cMockUpIncidentHandler() { }
    virtual ~cMockUpIncidentHandler() { }
    virtual fep::Result AssociateStrategy(const int16_t nFEPIncident,
                                const fep::tIncidentStrategy eStrategyDelegate,
                                const fep::tStrategyAssociation eAssociation) { return ERR_NOERROR; }
    virtual fep::Result AssociateStrategy(const int16_t nFEPIncident,
                                fep::IIncidentStrategy* pStrategyDelegate,
                                const char* strConfigurationPath = "",
                                const fep::tStrategyAssociation eAssociation = fep::SA_REPLACE) { return ERR_NOERROR; }
    virtual fep::Result AssociateCatchAllStrategy(
                    fep::IIncidentStrategy* pStrategyDelegate,
                    const char* strConfigurationPath,
                    const fep::tStrategyAssociation eAssociation = fep::SA_APPEND) { return ERR_NOERROR; }
    virtual fep::Result DisassociateStrategy(
                    const int16_t nFEPIncident,
                    fep::IIncidentStrategy* pStrategyDelegate) { return ERR_NOERROR; }
    virtual fep::Result DisassociateStrategy(
                    const int16_t nFEPIncident,
                    const fep::tIncidentStrategy eStrategyDelegate) { return ERR_NOERROR; }
    virtual fep::Result DisassociateCatchAllStrategy(
                    fep::IIncidentStrategy* pStrategyDelegate) { return ERR_NOERROR; }
    virtual fep::Result InvokeIncident(int16_t nFEPIncident,
                    fep::tSeverityLevel eSeverity,
                    const char* strDescription,
                    const char* strOrigin,
                    int nLine,
                    const char* strFile) { return ERR_NOERROR; }
    virtual fep::Result GetLastIncident(
                const fep::tIncidentEntry** ppIncidentEntry) { return ERR_NOERROR; }
    virtual fep::Result RetrieveIncidentHistory(
                fep::tIncidentListConstIter& io_iterHistBegin,
                fep::tIncidentListConstIter& io_iterHistEnd) { return ERR_NOERROR; }
    virtual fep::Result FreeIncidentHistory() { return ERR_NOERROR; }
    
};
#endif // _FEP_TEST_MOCK_INCIDENTHANDLER_H_INC_
