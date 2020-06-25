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
//! [Receiving participant]
#include "data_receiver.h"

cDataReceptionElement::cDataReceptionElement() :
    m_hSignalDDL(NULL),
    m_hSignalRAW(NULL),
    m_oDDLDataProcessor(),
    m_oRawDataProcessor()
{
}

fep::Result cDataReceptionElement::ProcessStartupEntry(const fep::tState eOldState)
{
    fep::Result nResult = ERR_NOERROR;

    // First step: Register a signal description 
    // (this is only needed for the DDL signal. it can be ommitted for raw signals)
    nResult = GetSignalRegistry()->RegisterSignalDescription("./Path/to/someSignalDescription.ddl",
        fep::ISignalRegistry::DF_DESCRIPTION_FILE);
    if (isFailed(nResult))
    {
        // If this fails we dont really know what to do other than go to state error
        GetStateMachine()->ErrorEvent();
    }

    return nResult;
}

fep::Result cDataReceptionElement::ProcessIdleEntry(const fep::tState eOldState)
{
    fep::Result nResult = ERR_NOERROR;
    if (eOldState != fep::FS_STARTUP)
    {
        // First we unregister our DataListeners (this always needs to be done before
        // destroying the elements!)
        GetUserDataAccess()->UnregisterDataListener(&m_oDDLDataProcessor, m_hSignalDDL);
        GetUserDataAccess()->UnregisterDataListener(&m_oRawDataProcessor, m_hSignalRAW);


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

fep::Result cDataReceptionElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    fep::Result nResult = ERR_NOERROR;
    // Second step: Register the signals (if not already registered)
    // first a ddl serialized signal
    cUserSignalOptions oDDLSignalOptions("MyDDLSignal", fep::SD_Input, "someTypeDefinedInDDL");
    // Now the raw signal, this is simply done by using the RAW-Constructor 
    // e.g. the one without a type
    cUserSignalOptions oRAWSignalOptions("MyRAWSignal", fep::SD_Input);

    if (m_hSignalDDL == NULL)
    {
        nResult = GetSignalRegistry()->RegisterSignal(oDDLSignalOptions, m_hSignalDDL);
    }
    if (isOk(nResult))
    {
        if (m_hSignalRAW == NULL)
        {
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
        // In case of an input signal we need to register an UserDataListener that will
        // actually receive the data (e.g. the update calls)
        nResult = GetUserDataAccess()->RegisterDataListener(&m_oDDLDataProcessor, m_hSignalDDL);
        nResult = GetUserDataAccess()->RegisterDataListener(&m_oRawDataProcessor, m_hSignalRAW);
        if (isOk(nResult))
        {
            // For this use case the initialization is done here
            GetStateMachine()->InitDoneEvent();
        }
        else
        {
            // If this fails we dont really know what to do other than go to state error
            GetStateMachine()->ErrorEvent();
        }
    }
   
    return nResult;
}
//! [Receiving participant]