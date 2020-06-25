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
//! [The main program]
#include "data_processor.h"
#include "data_receiver.h"
#include "data_transmitter.h"
#include <fep_participant_sdk.h>
using namespace fep;

fep::Result SimulateSomethingSpectacular(IUserDataSample* &poRawSample, IUserDataSample* &poDdlSample)
{
    // Here we do our spectacular simulation work
    return ERR_NOERROR;
}

int main(int argc, char** argv)
{
    cDataReceptionElement oReceptionElement;
    cDataTransmissionElement oTransmissionElement;

    cModuleOptions oReceptionElementOptions;
    cModuleOptions oTransmissionElementOptions;

    oReceptionElementOptions.SetElementName("ReceiverElement");
    oTransmissionElementOptions.SetElementName("TransmissionElement");

    oReceptionElement.Create(oReceptionElementOptions);
    oTransmissionElement.Create(oTransmissionElementOptions);

    oReceptionElement.GetStateMachine()->StartupDoneEvent();
    oTransmissionElement.GetStateMachine()->StartupDoneEvent();

    oReceptionElement.GetStateMachine()->InitializeEvent();
    oTransmissionElement.GetStateMachine()->InitializeEvent();

    oReceptionElement.WaitForState(FS_READY);
    oTransmissionElement.WaitForState(FS_READY);

    IUserDataSample* poSampleDDL;
    IUserDataSample* poSampleRAW;
    // Now we create our user data samples
    oTransmissionElement.CreateUserDataSample(poSampleDDL);
    oTransmissionElement.CreateUserDataSample(poSampleRAW);
    //  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  IMPORTANT!: The handle of a signal needs to be set so that !
    //    FEP knows on which signal the data should be transmitted !
    //  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    poSampleDDL->SetSignalHandle(oTransmissionElement.m_hSignalDDL);
    poSampleRAW->SetSignalHandle(oTransmissionElement.m_hSignalRAW);

    // Bring the elements into state FS_RUNNING (this is where data
    //   [as we previously defined it] transmission happens)
    oReceptionElement.GetStateMachine()->StartEvent();
    oTransmissionElement.GetStateMachine()->StartEvent();

    // Now lets simulate something:
    SimulateSomethingSpectacular(poSampleRAW, poSampleDDL);
    // After we finished our simulation we transmit the data:
    oTransmissionElement.TransmitData(poSampleDDL);
    oTransmissionElement.TransmitData(poSampleRAW);

    // We are done here so lets stop the elements
    oReceptionElement.GetStateMachine()->StopEvent();
    oTransmissionElement.GetStateMachine()->StopEvent();

    // Now destroy the created UserDataSamples
    delete poSampleDDL;
    delete poSampleRAW;

    oTransmissionElement.Destroy();
    oReceptionElement.Destroy();

    return 0;
}
//! [The main program]
