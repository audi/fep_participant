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
//! [Transmitting participant]
#include "data_transmitter.h"

cDataTransmissionElement::cDataTransmissionElement():
    m_hSignalDDL(NULL),
    m_hSignalRAW(NULL)
{
}

fep::Result cDataTransmissionElement::ProcessStartupEntry(const fep::tState eOldState)
{
    fep::Result nResult = ERR_NOERROR;
    // First step: Register a signal description
    // (this is only needed for the DDL signal. it can be ommitted for raw signals)
    nResult = GetSignalRegistry()->RegisterSignalDescription("./Path/to/someSignalDescription.ddl",
        fep::ISignalRegistry::DF_DESCRIPTION_FILE);
    if (isFailed(nResult))
    {
        //If this fails we dont really know what to do other than go to state error
        GetStateMachine()->ErrorEvent();
    }
 
    return nResult;
}

fep::Result cDataTransmissionElement::ProcessIdleEntry(const fep::tState eOldState)
{
    fep::Result nResult = ERR_NOERROR;
    if (eOldState != fep::FS_STARTUP)
    {
        if (m_hSignalDDL != NULL)
        {
            GetSignalRegistry()->UnregisterSignal(m_hSignalDDL);
        }
        if (m_hSignalRAW != NULL)
        {
            GetSignalRegistry()->UnregisterSignal(m_hSignalRAW);
        }
    }
    return nResult;
}

fep::Result cDataTransmissionElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    fep::Result nResult = ERR_NOERROR;
    // Second step: Register the signals (if not already registered)

    // First a ddl serialized signal
    cUserSignalOptions oDDLSignalOptions("MyDDLSignal", fep::SD_Output, "someTypeDefinedInDDL");
    // Now the raw signal, this is simply done by using the RAW-Constructor e.g. the one without a type
    cUserSignalOptions oRAWSignalOptions("MyRAWSignal", fep::SD_Output);

    if (m_hSignalDDL == NULL)
    {
        // The actual Registration of the DDL Signal
        nResult = GetSignalRegistry()->RegisterSignal(oDDLSignalOptions, m_hSignalDDL);
    }
    if (isOk(nResult))
    {
        
        if (m_hSignalRAW == NULL)
        {
            // The actual Registration of the RAW Signal
            nResult = GetSignalRegistry()->RegisterSignal(oRAWSignalOptions, m_hSignalRAW);
        }
    }

    if (isFailed(nResult))
    {
        // If this fails we dont really know what to do other than go to state error
        GetStateMachine()->ErrorEvent();
    }
    else
    {
        // For this use case the initialization is done here
        GetStateMachine()->InitDoneEvent();
    }
    return nResult;
}

 fep::Result cDataTransmissionElement::CreateUserDataSample(IUserDataSample * &poSample)
{
    return GetUserDataAccess()->CreateUserDataSample(poSample);
}

fep::Result cDataTransmissionElement::TransmitData(IUserDataSample * poSample)
{
    // We set the sync flag to true (this is the standard way if you are not
    // using the DDB (see 'Special Use Cases' section))  |
    //                                                   V
    return GetUserDataAccess()->TransmitData(poSample, true);
}
//! [Transmitting participant]