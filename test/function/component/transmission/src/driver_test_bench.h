/**
* Implementation of the tester for the FEP Transmission Adapter
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
#include "fep_participant_sdk.h"
#include "transmission_adapter/fep_transmission.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include "fep_test_common.h"


#ifndef _DRIVER_TESTER_COMMON_H_
#define _DRIVER_TESTER_COMMON_H_
class cDriverTester
{
public:
    static cTestBaseModule* m_pFEPModule;
    static fep::cTransmissionAdapter* m_pTransmissionAdapter;
    static fep::ITransmissionDriver* m_pDriver;
    static fep::IStateMachine* m_pStateMachine;

public:
    static void TestData(eFEPTransmissionType gs_eDUT);
    static void TestRxSampleSizeMismatch(eFEPTransmissionType gs_eDUT);
    static void TestMessageAfterCreate(eFEPTransmissionType gs_eDUT);
    static void TestVariableSignalSize(eFEPTransmissionType gs_eDUT);
};
#endif //_DRIVER_TESTER_COMMON_H_