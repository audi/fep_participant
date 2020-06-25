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
#include <fep_participant_sdk.h>
#include <a_util/system/uuid.h>

#ifdef _MSC_VER
// disable harmless warning originating from generated interface code
#pragma warning(disable : 4290)
#endif

// Interface class holding meta information about the Interface
class IServiceInterface
{
public:
    // Publish meta information
    FEP_RPC_IID("iid.example_service_interface", "Example Service Interface");
};

inline std::unique_ptr<fep::cModule> CreateParticipant(const char* strName = NULL)
{
    auto oParticipant = std::unique_ptr<fep::cModule>(new fep::cModule());

    fep::cModuleOptions oOptions;
    if (strName)
    {
        oOptions.SetParticipantName(strName);
    }
    else
    {
        // generate it
        oOptions.SetParticipantName(a_util::system::generateUUIDv4().c_str());
    }

    if (fep::isFailed(oParticipant->Create(oOptions)))
    {
        return nullptr;
    }
    double fFepVersion = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) +
        static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    oParticipant->GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VERSION, fFepVersion);
    oParticipant->GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_DESCRIPTION, "Example");
    oParticipant->GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_FEP_VERSION, fFepVersion);
    oParticipant->GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT, "FEP SDK");
    oParticipant->GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_CONTEXT_VERSION, fFepVersion);
    oParticipant->GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_VENDOR,
        "Audi Electronics Venture GmbH");
    if (strName)
    {
        // use different typeID for different participants!!
        oParticipant->GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID,
            "ad1852f2-c17c-4cb6-8e36-f1ea4b9685c0");
    }
    else
    {
        // use different typeID for different participants!!
        oParticipant->GetPropertyTree()->SetPropertyValue(FEP_PARTICIPANT_HEADER_TYPE_ID,
            "24b168d1-48e5-49c1-8423-cdd882d2a921");
    }

    oParticipant->GetStateMachine()->StartupDoneEvent();
    oParticipant->WaitForState(fep::FS_IDLE);

    return oParticipant;
}
