/************************************************************************
 * Snippets hosting FEP Participant ... nothing else. :P
 *

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
 * @file
 *
 */
#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <fstream>
#include <ctime>

#define FEP_PREP_CMD_VERSION 1.0

#ifdef WIN32
    // only required for sleep()
    #include "Windows.h"
    #define sleep(x) Sleep(static_cast<tUInt32>(x) * 1000);
#endif

#include "snippet_ddb.h"

class cMyDescription
{
public:
    static const char* GetDescription(const char* strType)
    {
        strType = NULL;  // suppress a warning here - thats all
        return "Bla";
    }
};

cMyElement::cMyElement() : m_hMySignalHandle(NULL), m_pDDBAccess(NULL)
{
    // nothing to do here...
}

cMyElement::~cMyElement()
{
    // nothing to do here...
}

fep::Result cMyElement::ProcessStartupEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessStartupEntry(eOldState));

    return fep::ERR_NOERROR;
}

//! [ProcessDDBSyncInitialization]
fep::Result cMyElement::ProcessIdleEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessIdleEntry(eOldState));

    if (FS_STARTUP == eOldState)
    {
        std::cout << "Startup Done " << GetName() << std::endl;
    }
    else
    {
        if (m_pDDBAccess)
        {
            m_pDDBAccess->UnregisterSyncListener(this);
            cModule::DestroyDDBEntry(m_hMySignalHandle);
        }
    }

    return fep::ERR_NOERROR;
}

fep::Result cMyElement::ProcessInitializingEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessInitializingEntry(eOldState));
    std::cout << "Initializing " << GetName() << std::endl;

    RETURN_IF_FAILED(cModule::InitDDBEntry("MySignal",                          /* strSignalName */
                                           "tMySignal",                         /* strSignalType */
                                           50,                                  /* szMaxDepth */
                                           DDBDS_DeliverIncomplete,             /* DDB delivery strat */
                                           m_hMySignalHandle,
                                           &m_pDDBAccess));

    RETURN_IF_FAILED(m_pDDBAccess->RegisterSyncListener(this));

    GetStateMachine()->InitDoneEvent();

    return fep::ERR_NOERROR;
}
//! [ProcessDDBSyncInitialization]

fep::Result cMyElement::ProcessReadyEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessReadyEntry(eOldState));
    std::cout << "Ready " << GetName() << std::endl;
    return fep::ERR_NOERROR;
}

fep::Result cMyElement::ProcessRunningEntry(const fep::tState eOldState)
{
    //! [LockDataAppl]
    if (m_pDDBAccess)
    {
        const fep::IDDBFrame * pDDBFrame = NULL;

        // Since the DDB is *always* driven asynchronouly, LockData() locks access to the read buffer
        // This guarantees that data remain unaltered until the user unlocks the read buffer by
        // calling UnlockData()

        if (fep::isFailed(m_pDDBAccess->LockData(pDDBFrame)))
        {
            std::cerr << "Fatal: Buffer could not be locked or is empty!" << std::endl;
        }
        else
        {
            // note: IDDBFrame::GetFrameSize() is *not* necessarily equal to IDDBFrame::GetMaxSize()!
            // IDDBFrame::GetMaxSize() will always yield the max. buffer size as specified
            // in cModule::InitDDBEntry(), IDDBFrame::GetFrameSize() will yield the size of the current
            // frame as specified by the frame number of the last valid sample in the frame, and
            // IDDBFrame::GetValidCount() will yield the number of valid samples in the frame.

            // note: all methods above are 1-based;

            // warning: Due to the polling nature of this mechanism, GetRecentData() will
            // continue to return the same frame over and over again until a sync
            // signal has been encounterd by the DDB!
            // warning: This method does not actively notify the user of a new frame!
            const fep::IUserDataSample* pUserDataSample = NULL;
            for (size_t nSampleIdx = 0; nSampleIdx < pDDBFrame->GetFrameSize(); ++nSampleIdx)
            {
                // alternatively, you might check via
                // NULL != pDDBFrame->GetSample(nSampleIdx)
                if (pDDBFrame->IsValidSample(nSampleIdx))
                {
                    pUserDataSample = pDDBFrame->GetSample(nSampleIdx);
                    // [do something with the data here...]

                    std::stringstream ssLog;
                    ssLog << "Received FEP Sample no " << nSampleIdx << " with size ";
                    ssLog << pUserDataSample->GetSize() << std::endl;
                    std::cout << ssLog.str();
                }

            }

        }
        // NEVER forget to unlock the buffer; otherwise all subsequent samples
        // will be discarded by the DDB!
        RETURN_IF_FAILED(m_pDDBAccess->UnlockData());
    }
    //! [LockDataAppl]
    return fep::ERR_NOERROR;
}

fep::Result cMyElement::ProcessErrorEntry(const fep::tState eOldState)
{
    RETURN_IF_FAILED(cModule::ProcessErrorEntry(eOldState));
    std::cout << GetName() << " threw an error! Something went wrong!" << std::endl;
    return fep::ERR_NOERROR;
}

//! [ProcessDDBSyncImpl]
fep::Result cMyElement::ProcessDDBSync(
        handle_t const hSignal,
        const fep::IDDBFrame& oDDBFrame)
{
    // note: By choosing an appropriate delivery strategy, the DDB will only deliver complete frames,
    // i.e. all IUserDataSamples are present (identified by their sample number in the frame), and
    // the last IUserDataSample has the sync flag set.
    //
    // In any case, the DDB will throw incidents when frames are dropped. Depending on the sensitivity
    // of the use-case, incidents have to be monitored through the respective handling mechnisms
    // provided by FEP.

    if (hSignal == m_hMySignalHandle)
    {
        // guard against incomplete frames!
        if (!oDDBFrame.IsComplete())
        {
            // Note: Logging is already performed by the the FEP Incident Handler.

            // [do something to address missing data samples...]

            // Note: Incidents are *always* invoked synchronously and in advance of calling
            // ProcessDDBSync()! => Simply skipping this frame.
            m_bDDBFrameInconsistent = false;
            return fep::ERR_NOERROR;
        }


        for (uint16_t nCnt = 0; nCnt < oDDBFrame.GetFrameSize(); ++nCnt)
        {
            if (oDDBFrame.IsValidSample(nCnt))
            {
                const IUserDataSample* pDataSample = oDDBFrame.GetSample(nCnt);

                // [do something with the data here...]

                std::stringstream ssLog;

                ssLog << "Received FEP Sample no " << nCnt << " with size ";
                ssLog << pDataSample->GetSize() << std::endl;
                std::cout << ssLog.str();
            }
        }
    }

    // The error returned here will issue a FEP incident FSI_GENERAL_WARNING of severity SL_Warning
    return fep::ERR_NOERROR;
}
//! [ProcessDDBSyncImpl]

//! [DDBErrorHandling]
fep::Result cMyElement::HandleLocalIncident(const int16_t nIncident,
                                        const fep::tSeverityLevel eSeverity,
                                        const char *strOrigin,
                                        int nLine,
                                        const char *strFile,
                                        const timestamp_t tmSimTime,
                                        const char* strDescription /* = NULL*/)
{
    switch(nIncident)
    {
    case FSI_DDB_RX_ABORT_SYNC:   //< The frame is incomplete and has been dropped (only for DDBDS_DumpIncomplete)
                                  //< or the stock buffer is discarded because a new frame has been completed
    case FSI_DDB_RX_OVERRUN:      //< Unable to store the currently recevied data sample. It has been dropped.
        m_bDDBFrameInconsistent = true;
    default:
        break;
    }

    //Note: Logging of incident and descrption is already covered by the FEP Inciden Handler

    return fep::ERR_NOERROR;
}
//! [DDBErrorHandling]
