/**
 * Implementation of the tester for the Data Listener Adapter
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
/*
* Test Case:   DataListenerAdapterIncidents
* Test ID:     1.8
* Test Title:  Check incident handling
* Description: Verify that incidents are issued if a frame is missing
* Strategy:    Inject valid samples, vary frame ID and sample number
* Passed If:   Incidents are invoked as necessary
* Ticket:      -
* Requirement: FEPSDK-1512
*/
#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::component_config;

#include "transmission_adapter/fep_data_listener_adapter.h"
#include "transmission_adapter/fep_data_sample_factory.h"

// Begin of tests

class cUserDataListenerDummy: public fep::IUserDataListener
{
public:
    cUserDataListenerDummy() {}
    ~cUserDataListenerDummy() {}

    fep::Result Update(const IUserDataSample *poSample)
    {
        return ERR_NOERROR;
    }
};

class cIncidentHandlerStub : public fep::IIncidentHandler
{
public:
    cIncidentHandlerStub(): m_nIncident(0)
    { }
    ~cIncidentHandlerStub() {}

    int16_t m_nIncident;

    fep::Result AssociateStrategy(const int16_t nFEPIncident,
                                const fep::tIncidentStrategy eStrategyDelegate,
                                const fep::tStrategyAssociation eAssociation){return ERR_NOERROR;}
    fep::Result AssociateStrategy(
                    const int16_t nFEPIncident,
                    fep::IIncidentStrategy* pStrategyDelegate,
                    const char* strConfigurationPath = "",
                    const fep::tStrategyAssociation eAssociation = SA_REPLACE){return ERR_NOERROR;}
    fep::Result AssociateCatchAllStrategy(
            fep::IIncidentStrategy* pStrategyDelegate,
            const char* strConfigurationPath,
            const fep::tStrategyAssociation eAssociation = SA_APPEND){return ERR_NOERROR;}
    fep::Result DisassociateStrategy(
                    const int16_t nFEPIncident,
                    fep::IIncidentStrategy* pStrategyDelegate){return ERR_NOERROR;}
    fep::Result DisassociateStrategy(
                    const int16_t nFEPIncident,
                    const fep::tIncidentStrategy eStrategyDelegate){return ERR_NOERROR;}
    fep::Result DisassociateCatchAllStrategy(
                    fep::IIncidentStrategy* pStragetyDelegate){return ERR_NOERROR;}
    fep::Result GetLastIncident(
                    const fep::tIncidentEntry** ppIncidentEntry){return ERR_NOERROR;}
    fep::Result RetrieveIncidentHistory(
                    fep::tIncidentListConstIter& io_iterHistBegin,
                    fep::tIncidentListConstIter& io_iterHistEnd){return ERR_NOERROR;}
    fep::Result FreeIncidentHistory(){return ERR_NOERROR;}
    fep::Result InvokeIncident(int16_t nFEPIncident,
                           fep::tSeverityLevel eSeverity,
                           const char* strDescription,
                           const char *strOrigin,
                           int nLine,
                           const char *strFile)
    {
        m_nIncident = nFEPIncident;
        return ERR_NOERROR;
    }
};

fep::Result ResetSample(fep::IPreparationDataSample * poPrepSample)
{
    poPrepSample->SetFrameId(1);
    poPrepSample->SetSampleNumberInFrame(0);
    poPrepSample->SetSyncFlag(true);

    return ERR_NOERROR;
}

/**
 * @req_id "FEPSDK-1512"
 */
TEST(cTesterDataListenerAdapter, DataListenerAdapterIncidents)
{
    fep::IPreparationDataSample * poPrepSample = NULL;

    fep::cDataSampleFactory::CreateSample(&poPrepSample);
    poPrepSample->SetSignalHandle(NULL);
    poPrepSample->SetSize(sizeof(uint16_t));

    cUserDataListenerDummy * poUserDataListenerDummy = new cUserDataListenerDummy();

    cIncidentHandlerStub * poIncidentHandlerStub = new cIncidentHandlerStub();

    fep::cDataListenerAdapter * poDataListenerAdapter = new fep::cDataListenerAdapter(poUserDataListenerDummy,poIncidentHandlerStub,NULL);

    // Note: Resetting the frame ID to a lower number is not considered an error!

    // skip a frame
    ResetSample(poPrepSample);
    poDataListenerAdapter->Update(poPrepSample);
    ASSERT_EQ( 0, poIncidentHandlerStub->m_nIncident);

    poPrepSample->SetFrameId(3);
    poDataListenerAdapter->Update(poPrepSample);
    ASSERT_EQ( fep::FSI_TRANSM_RX_MISSING_DATASAMPLE, poIncidentHandlerStub->m_nIncident);

    // skip a sample in a frame
    ResetSample(poPrepSample);
    poPrepSample->SetSyncFlag(false);
    poDataListenerAdapter->Update(poPrepSample);
    poIncidentHandlerStub->m_nIncident = 0;

    poPrepSample->SetSampleNumberInFrame(2);
    poDataListenerAdapter->Update(poPrepSample);
    ASSERT_EQ( fep::FSI_TRANSM_RX_MISSING_DATASAMPLE, poIncidentHandlerStub->m_nIncident);

    // skip a sync flag
    ResetSample(poPrepSample);
    poPrepSample->SetSyncFlag(false);
    poDataListenerAdapter->Update(poPrepSample);
    poIncidentHandlerStub->m_nIncident = 0;

    poPrepSample->SetFrameId(2);
    poDataListenerAdapter->Update(poPrepSample);
    ASSERT_EQ( fep::FSI_TRANSM_RX_MISSING_DATASAMPLE, poIncidentHandlerStub->m_nIncident);

    delete poPrepSample;
    delete poDataListenerAdapter;
    delete poIncidentHandlerStub;
    delete poUserDataListenerDummy;
}
