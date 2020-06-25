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
#pragma once

#include "dummy_participant.h"
#include <fep_participant_sdk.h>

class RemoteStarterParticipant : public fep::cModule
{
public:
    std::unique_ptr<fep::cModule> _participant_to_start;
    ~RemoteStarterParticipant()
    {
        if (_participant_to_start)
        {
            _participant_to_start->Destroy();
        }
    }

    fep::Result ProcessStartupEntry(const fep::tState) override
    {
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, 1.0);
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION,
            "RemoteStarter Dummy Participant");
        double fFepVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
            static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, 1.0);
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR, "Audi Electronics Venture GmbH");
        GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID, "0016a57b-8eef-4c98-b3b9-83872315d3a7");

        GetStateMachine()->StartupDoneEvent();

        return fep::ERR_NOERROR;
    }

    fep::Result ProcessIdleEntry(const fep::tState eOldState) override
    {
        return fep::ERR_NOERROR;
    }

    fep::Result ProcessInitializingEntry(const fep::tState) override
    {
        GetStateMachine()->InitDoneEvent();
        return fep::ERR_NOERROR;
    }

    fep::Result ProcessRunningEntry(const fep::tState) override
    {
        _participant_to_start.reset(new DummyParticipant());
        fep::cModuleOptions opts("Dummy");

        if (fep::isFailed(_participant_to_start->Create(opts)))
        {
            std::cerr << "Failed to create module!\n";
        }
        GetStateMachine()->StopEvent();
        return fep::ERR_NOERROR;
    }


};

static inline std::unique_ptr<fep::cModule> createRemoteStarterParticipant(const char* name)
{
    std::unique_ptr<fep::cModule> participant(new RemoteStarterParticipant());
    fep::cModuleOptions opts(name);

    if (fep::isFailed(participant->Create(opts)))
    {
        std::cerr << "Failed to create module!\n";
        return{};
    }

    return participant;
}


