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

static void SerializeStringIntoSample(fep::IUserDataSample& sample, const std::string& str)
{
    if (fep::isOk(sample.SetSize(str.size())))
    {
        sample.CopyFrom(str.c_str(), sample.GetSize());
    }
}

class DynamicDataSender : public fep::cModule
{
public:
    DynamicDataSender() : m_hSignalRAW(NULL)
    {
    }

    DynamicDataSender(bool bAutoFlag) : m_hSignalRAW(NULL), m_bAutoFlag(bAutoFlag)
    {
    }

    ~DynamicDataSender()
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
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION, "Example:FEP Dynamic Data sender");
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fVersion);
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, fVersion);
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID,
            "c8112bab-4e3b-4d64-b959-08fc17a708e4");

        GetStateMachine()->StartupDoneEvent();
        return {};
    }

    fep::Result ProcessIdleEntry(const fep::tState eOldState) override
    {
        if (fep::FS_STARTUP != eOldState)
        {
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

        fep::cUserSignalOptions options("MyRAWSignal", fep::tSignalDirection::SD_Output);
        nResult = GetSignalRegistry()->RegisterSignal(options, m_hSignalRAW);

        if (fep::isFailed(nResult))
        {
            GetStateMachine()->ErrorEvent();
        }
        else
        {
            GetStateMachine()->InitDoneEvent();
        }

        return {};
    }

    fep::Result ProcessReadyEntry(const fep::tState eOldState) override
    {
        std::cout << GetName() << " Ready" << std::endl;
        return {};
    }

    void SendDataSample(std::string strData)
    {
        fep::IUserDataSample* pSample = nullptr;
        if (fep::isOk(GetUserDataAccess()->CreateUserDataSample(pSample, m_hSignalRAW)))
        {
            SerializeStringIntoSample(*pSample, strData);
            GetUserDataAccess()->TransmitData(pSample, true);
            delete pSample;
        }
    }

    fep::Result ProcessRunningEntry(const fep::tState eOldState) override
    {
        std::cout << GetName() << " Running (send some strings)" << std::endl;

        std::string input;
        if (!m_bAutoFlag)
        {
            while (true)
            {
                std::cout << GetName() << "> ";
                std::getline(std::cin, input);

                SendDataSample(input);

                if (input.empty())
                {
                    break;
                }
            }
        }
        else
        {
            SendDataSample("Hi");
            SendDataSample("I am test string");
            SendDataSample("");
        }

        GetStateMachine()->StopEvent();
        return {};
    }

public:
    handle_t m_hSignalRAW;
    bool m_bAutoFlag = false;
};

int main(int argc, const char* argv[])
{
    fep::cModuleOptions oModuleOptions("DynamicDataSender");

    bool bAutoFlag = false;
    oModuleOptions.SetAdditionalOption(bAutoFlag, "-a", "--auto",
        "(optional) Runs the sender participant without user input.\n");

    if (fep::isFailed(oModuleOptions.ParseCommandLine(argc, argv)))
    {
        return 1;
    }

    DynamicDataSender* sender = new DynamicDataSender(bAutoFlag);
    if (fep::isFailed(sender->Create(oModuleOptions)))
    {
        return 1;
    }

    sender->WaitForState(fep::FS_IDLE);
    sender->GetStateMachine()->InitializeEvent();
    sender->WaitForState(fep::FS_READY);
    sender->GetStateMachine()->InitializeEvent();
    sender->WaitForState(fep::FS_READY);
    sender->GetStateMachine()->StartEvent();
    sender->WaitForState(fep::FS_RUNNING);

    // wait until it is idle again
    sender->WaitForState(fep::FS_IDLE);
    sender->GetStateMachine()->ShutdownEvent();
    sender->WaitForShutdown();

    sender->Destroy();
}
