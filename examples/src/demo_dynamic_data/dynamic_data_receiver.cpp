/************************************************************************
 * Implementation of the dynamic data sender
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
 */
#include <fep_participant_sdk.h>
#include <iostream>

static std::string DeserializeStringFromSample(const fep::IUserDataSample& sample)
{
    std::string value;
    if (sample.GetSize() > 0)
    {
        value.assign(static_cast<const char*>(sample.GetPtr()), sample.GetSize());
    }
    return value;
}

class DynamicDataReceiver : public fep::cModule, private fep::IUserDataListener
{
public:
    DynamicDataReceiver() : m_hSignalRAW(NULL)
    {
    }

    ~DynamicDataReceiver()
    {
        Destroy();
    }

public:
    fep::Result ProcessStartupEntry(const fep::tState eOldState) override
    {
        // Filling the Element header properly
        double fVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
            static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, fVersion);
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION, "Example:FEP Dynamic Data receiver");
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fVersion);
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, fVersion);
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID,
            "6687de50-8aa3-49e8-a0cb-e66618f6e0b1");

        GetStateMachine()->StartupDoneEvent();
        return {};
    }

    fep::Result ProcessIdleEntry(const fep::tState eOldState) override
    {
        if (fep::FS_STARTUP != eOldState)
        {
            GetUserDataAccess()->UnregisterDataListener(this, m_hSignalRAW);

            if (NULL != m_hSignalRAW)
            {
                GetSignalRegistry()->UnregisterSignal(m_hSignalRAW);
            }
        }

        std::cout << GetName() << " Idle" << std::endl;
        return {};
    }

    fep::Result ProcessInitializingEntry(const fep::tState eOldState) override
    {
        fep::Result nResult = fep::ERR_NOERROR;

        fep::cUserSignalOptions options("MyRAWSignal", fep::tSignalDirection::SD_Input);
        nResult = GetSignalRegistry()->RegisterSignal(options, m_hSignalRAW);

        if (fep::isFailed(nResult))
        {
            GetStateMachine()->ErrorEvent();
        }
        else
        {
            GetUserDataAccess()->RegisterDataListener(this, m_hSignalRAW);
            GetStateMachine()->InitDoneEvent();
        }

        return {};
    }

    fep::Result ProcessReadyEntry(const fep::tState eOldState) override
    {
        std::cout << GetName() << " Ready" << std::endl;
        return {};
    }


    fep::Result ProcessRunningEntry(const fep::tState eOldState) override
    {
        std::cout << GetName() << " Running and listening for strings" << std::endl;
        return {};
    }

    fep::Result Update(const fep::IUserDataSample* poSample) override
    {
        if (poSample->GetSignalHandle() == m_hSignalRAW)
        {
            std::string received = DeserializeStringFromSample(*poSample);
            if (received.empty())
            {
                GetStateMachine()->StopEvent();
            }

            std::cout << "Received: " << received << std::endl;
        }
        return {};
    }

public:
    handle_t m_hSignalRAW;
};

int main(int argc, const char* argv[])
{
    fep::cModuleOptions oModuleOptions("DynamicDataReceiver");
    if (fep::isFailed(oModuleOptions.ParseCommandLine(argc, argv)))
    {
        return 1;
    }

    DynamicDataReceiver receiver;
    if (fep::isFailed(receiver.Create(oModuleOptions)))
    {
        return 1;
    }

    receiver.WaitForState(fep::FS_IDLE);
    receiver.GetStateMachine()->InitializeEvent();
    receiver.WaitForState(fep::FS_READY);
    receiver.GetStateMachine()->InitializeEvent();
    receiver.WaitForState(fep::FS_READY);
    receiver.GetStateMachine()->StartEvent();
    receiver.WaitForState(fep::FS_RUNNING);

    // wait until it is idle again
    receiver.WaitForState(fep::FS_IDLE);
    receiver.GetStateMachine()->ShutdownEvent();
    receiver.WaitForShutdown();
}
