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
#include <cassert>
#include <iostream>
#include <memory>
#include <string>

#include "transmitter_participant.h"
#include "utils.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"

namespace
{

constexpr uint8_t number_of_allowed_transmitters = 10;

auto createNameAndID(const std::string& transmitter_id_input) -> std::pair<std::string,
                                                                           fep_examples::ComponentID>
{
    fep_examples::ComponentID transmitter_id{0};
    try
    {
        transmitter_id = a_util::strings::toNumeric<fep_examples::ComponentID>(transmitter_id_input);
    }
    catch (const std::exception&)
    {
        std::cerr << "Internal error at argument of parameter --id: Conversion to int failed"
                  << std::endl;
        exit(1);
    }
    if (transmitter_id == 0 || transmitter_id > number_of_allowed_transmitters)
    {
        std::cerr << "Internal error at argument of parameter --id: Out of bounce, only "
                     "values from 1 to "
                  << number_of_allowed_transmitters << " allowed" << std::endl;
        exit(1);
    }
    assert(transmitter_id >= 1);
    assert(transmitter_id <= number_of_allowed_transmitters);
    return std::make_pair(std::string("Transmitter30") + '_' + transmitter_id_input,
                          transmitter_id);
}
} // namespace

int main(int nArgc, const char* pArgv[])
{
    // FEP Transmitter participant
    fep::ParticipantFEP2 transmitter_participant;

    // Set FEP Timing 30 as default timing
    fep::cModuleOptions module_options{"Transmitter30", fep::eTimingSupportDefault::timing_FEP_30};

    // Enable console logging
    setProperty(transmitter_participant, FEP_CONSOLE_LOG_ENABLE, true);
    setProperty(transmitter_participant, FEP_CONSOLE_LOG_ENABLE_CATCHALL, true);

    // Add an additional configuration parameter regarding component id
    auto transmitter_id_input = std::string();
    module_options.SetAdditionalOption(
        transmitter_id_input,
        "-c",
        "--component",
        "(Required Argument) set component id of transmitter participant for cascade.\n"
        "Only int-values from 1 to 100 are valid.",
        "int");

    // Parse command line
    if (fep::isFailed(module_options.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }

    // Create and set participant name, get Transmitter ID from commandLine
    auto name_and_ID = createNameAndID(transmitter_id_input);
    module_options.SetParticipantName(name_and_ID.first.c_str());

    // set header information
    fep_examples::utils::setParticipantHeaderInformation(
        transmitter_participant, "Realtime Cascade 30 Demo Element Transmitter");

    // Create the DataJob
    std::shared_ptr<fep::DataJob> transmitter_element =
        std::make_shared<Transmitter>(name_and_ID.second);

    // Create the FEP Participant
    fep::Result result = transmitter_participant.Create(module_options, transmitter_element);
    if (fep::isFailed(result))
    {
        std::cerr << "Internal Error: Failed to create Elements." << std::endl;
        std::cerr << result.getErrorLabel() << ": " << result.getDescription() << std::endl;
        return 1;
    }

    return fep::isFailed(transmitter_participant.waitForShutdown()) ? 1 : 0;
}
