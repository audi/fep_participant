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

#include "fep3/components/legacy/timing/global_scheduler_configuration.h"
#include <cstddef>
#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include <utility>
#include <a_util/base/types.h>
#include <a_util/filesystem/path.h>
#include <a_util/result/result_type.h>
#include <a_util/strings/strings_convert_decl.h>
#include <a_util/strings/strings_functions.h>
#include <a_util/xml/dom.h>

#include "fep3/components/legacy/timing/common_timing.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep_errors.h"

using namespace fep;
using namespace fep::timing;

#define ERROR_DESCRIPTION "Error in Timing Configuration: "
#define MAKE_ERROR(x) fep::Result(ERR_UNEXPECTED, ERROR_DESCRIPTION x, 0, filename.c_str(), __FUNCTION__);


static TimeViolationStrategy TimeViolationStrategy_fromString(const char* strategy_string)
{
//#define COMPARE_STRATEGY(x) if (a_util::strings::isEqualNoCase(strategy_string, #x) || a_util::strings::isEqualNoCase(strategy_string, "TS_" #x)) { return TS_##x; }
#define COMPARE_STRATEGY(x) if (a_util::strings::isEqual(strategy_string, "TS_" #x)) { return TS_##x; }

    COMPARE_STRATEGY(UNKNOWN);
    COMPARE_STRATEGY(IGNORE_RUNTIME_VIOLATION);
    COMPARE_STRATEGY(WARN_ABOUT_RUNTIME_VIOLATION);
    COMPARE_STRATEGY(SKIP_OUTPUT_PUBLISH);
    COMPARE_STRATEGY(SET_STM_TO_ERROR);

#undef COMPARE_STRATEGY

    return TS_UNKNOWN;
}

static const char* TimeViolationStrategy_toString(const TimeViolationStrategy strategy_enum)
{
#define COMPARE_STRATEGY(x) case TS_##x: return "TS_" #x;

    switch (strategy_enum)
    {
        COMPARE_STRATEGY(UNKNOWN);
        COMPARE_STRATEGY(IGNORE_RUNTIME_VIOLATION);
        COMPARE_STRATEGY(WARN_ABOUT_RUNTIME_VIOLATION);
        COMPARE_STRATEGY(SKIP_OUTPUT_PUBLISH);
        COMPARE_STRATEGY(SET_STM_TO_ERROR);
    }

#undef COMPARE_STRATEGY

    return "TS_UNKNOWN";
}

static InputViolationStrategy InputViolationStrategy_fromString(const char* strategy_string)
{
    //#define COMPARE_STRATEGY(x) if (a_util::strings::isEqualNoCase(strategy_string, #x) || a_util::strings::isEqualNoCase(strategy_string, "IS_" #x)) { return IS_##x; }
#define COMPARE_STRATEGY(x) if (a_util::strings::isEqual(strategy_string, "IS_" #x)) { return IS_##x; }

    COMPARE_STRATEGY(UNKNOWN);
    COMPARE_STRATEGY(IGNORE_INPUT_VALIDITY_VIOLATION);
    COMPARE_STRATEGY(WARN_ABOUT_INPUT_VALIDITY_VIOLATION);
    COMPARE_STRATEGY(SKIP_OUTPUT_PUBLISH);
    COMPARE_STRATEGY(SET_STM_TO_ERROR);

#undef COMPARE_STRATEGY

    return IS_UNKNOWN;
}

static const char* InputViolationStrategy_toString(const InputViolationStrategy strategy_enum)
{
#define COMPARE_STRATEGY(x) case IS_##x: return "IS_" #x;

    switch (strategy_enum)
    {
        COMPARE_STRATEGY(UNKNOWN);
        COMPARE_STRATEGY(IGNORE_INPUT_VALIDITY_VIOLATION);
        COMPARE_STRATEGY(WARN_ABOUT_INPUT_VALIDITY_VIOLATION);
        COMPARE_STRATEGY(SKIP_OUTPUT_PUBLISH);
        COMPARE_STRATEGY(SET_STM_TO_ERROR);
    }

#undef COMPARE_STRATEGY

    return "IS_UNKNOWN";
}

static fep::Result internal_readTimingConfigFromDOM(a_util::xml::DOM& dom, const std::string& filename, TimingConfiguration& timing_configuration)
{
    using namespace a_util::xml;

    // Clear configuration
    timing_configuration = TimingConfiguration();

    fep::Result result = ERR_NOERROR;

    {
        // Read Header
        DOMElement headerElement;
        if (dom.findNode("/timing/header", headerElement))
        {
            DOMElement domElement;

            // Element "author" is optional
            domElement = headerElement.getChild("author");
            if (!domElement.isNull())
            {
                timing_configuration.m_header.m_author = domElement.getData();
            }

            // Element "date_creation" is optional
            domElement = headerElement.getChild("date_creation");
            if (!domElement.isNull())
            {
                timing_configuration.m_header.m_date_creation = domElement.getData();
            }

            // Element "date_change" is optional
            domElement = headerElement.getChild("date_change");
            if (!domElement.isNull())
            {
                timing_configuration.m_header.m_date_change = domElement.getData();
            }

            // Element "description" is optional
            domElement = headerElement.getChild("description");
            if (!domElement.isNull())
            {
                timing_configuration.m_header.m_description = domElement.getData();
            }
        }
    }

    {
        // Read Participants
        DOMElementList participantElementList;
        if (dom.findNodes("/timing/participants/participant", participantElementList))
        {
            for (DOMElementList::iterator it1 = participantElementList.begin(); it1 != participantElementList.end(); ++it1)
            {
                DOMElement& participantElement = *it1;
                std::string participant_name;
                Participant participant;
                  

                // Attribute "name" is required and must be not empty
                if (participantElement.hasAttribute("name"))
                {
                    participant_name = participantElement.getAttribute("name");
                    if (participant_name.empty())
                    {
                        result = MAKE_ERROR("Empty attribute \"name\"");
                    }
                }
                else
                {
                    if (fep::isOk(result))
                    {
                        result = MAKE_ERROR("Missing attribute \"name\"");
                    }
                }

                // Parse system timeout
                if (participantElement.hasAttribute("systemTimeout_s"))
                {
                    std::string system_timeout_string = participantElement.getAttribute("systemTimeout_s");
                    if (system_timeout_string.empty())
                    {
                        result = MAKE_ERROR("Empty attribute \"systemTimeout_s\"");
                    }
                    else
                    {
                        if (system_timeout_string == "0" || system_timeout_string == "NONE" || system_timeout_string == "OFF")
                        {
                            participant.m_systemTimeout_s = 0;
                        }
                        else
                        {
                            timestamp_t systemTimeout_s = a_util::strings::toInt64(system_timeout_string);
                            if (systemTimeout_s <= 0)
                            {
                                result = MAKE_ERROR("Invalid value for \"systemTimeout_s\"");
                            }
                            else
                            {
                                participant.m_systemTimeout_s = systemTimeout_s;
                            }
                        }
                    }
                }

                // Parse Steps
                DOMElementList stepElementList;
                if (participantElement.findNodes("steps/step", stepElementList))
                {
                    for (DOMElementList::iterator it2 = stepElementList.begin(); it2 != stepElementList.end(); ++it2)
                    {
                        DOMElement& stepElement = *it2;
                        StepConfig step_config(100 * 1000);
                        std::string step_name;

                        // Attribute "name" is required
                        if (stepElement.hasAttribute("name"))
                        {
                            step_name = stepElement.getAttribute("name");
                            if (step_name.empty())
                            {
                                if (fep::isOk(result))
                                {
                                    result = MAKE_ERROR("Empty attribute \"name\"");
                                }
                            }
                        }
                        else
                        {
                            if (fep::isOk(result))
                            {
                                result = MAKE_ERROR("Missing attribute \"name\"");
                            }
                        }

                        // Read stepListenerConfig: cycleTime_sim_us
                        if (stepElement.hasAttribute("cycleTime_sim_us"))
                        {
                            std::string str = stepElement.getAttribute("cycleTime_sim_us");
                            step_config.m_cycleTime_sim_us = a_util::strings::toInt64(str);
                            if (step_config.m_cycleTime_sim_us == 0)
                            {
                                result = MAKE_ERROR("Invalid attribute \"cycleTime_sim_us\"");
                            }
                        }
                        else
                        {
                            if (fep::isOk(result))
                            {
                                result = MAKE_ERROR("Missing attribute \"cycleTime_sim_us\"");
                            }
                        }

                        // Read stepListenerConfig: cycleTime_sim_us
                        if (stepElement.hasAttribute("maxRuntime_us"))
                        {
                            std::string str = stepElement.getAttribute("maxRuntime_us");
                            step_config.m_maxRuntime_us = a_util::strings::toInt64(str);
                            if (step_config.m_maxRuntime_us < 0)
                            {
                                result = MAKE_ERROR("Invalid attribute \"maxRuntime_us\"");
                            }
                        }

                        // Read stepListenerConfig: cycleTime_sim_us
                        if (stepElement.hasAttribute("maxInputWaittime_us"))
                        {
                            std::string str = stepElement.getAttribute("maxInputWaittime_us");
                            step_config.m_maxInputWaittime_us = a_util::strings::toInt64(str);
                            if (step_config.m_maxInputWaittime_us < 0)
                            {
                                result = MAKE_ERROR("Invalid attribute \"maxInputWaittime_us\"");
                            }
                        }
                        else
                        {
                            if (fep::isOk(result))
                            {
                                result = MAKE_ERROR("Missing attribute \"maxInputWaittime_us\"");
                            }
                        }

                        // Read stepListenerConfig: runtimeViolationStrategy
                        if (stepElement.hasAttribute("runtimeViolationStrategy"))
                        {
                            std::string str = stepElement.getAttribute("runtimeViolationStrategy");
                            step_config.m_runtimeViolationStrategy = TimeViolationStrategy_fromString(str.c_str());

                            if (step_config.m_runtimeViolationStrategy == TS_UNKNOWN)
                            {
                                if (fep::isOk(result))
                                {
                                    result = MAKE_ERROR("Invalid attribute \"runtimeViolationStrategy\"");
                                }
                            }
                        }
                        else
                        {
                            if (fep::isOk(result))
                            {
                                result = MAKE_ERROR("Missing attribute \"runtimeViolationStrategy\"");
                            }
                        }

                        DOMElementList inputElementList;
                        if (stepElement.findNodes("inputs/input", inputElementList))
                        {
                            for (DOMElementList::iterator it3 = inputElementList.begin(); it3 != inputElementList.end(); ++it3)
                            {
                                DOMElement& inputElement = *it3;
                                std::string input_name;
                                InputConfig input_config;
                                input_config.m_handle = NULL; // Invalid handle

                                // Attribute "name" is required
                                if (inputElement.hasAttribute("name"))
                                {
                                    input_name = inputElement.getAttribute("name");
                                    if (input_name.empty())
                                    {
                                        if (fep::isOk(result))
                                        {
                                            result = MAKE_ERROR("Empty attribute \"name\"");
                                        }
                                    }
                                }
                                else
                                {
                                    if (fep::isOk(result))
                                    {
                                        result = MAKE_ERROR("Missing attribute \"name\"");
                                    }
                                }

                                // Attribute "name" is required
                                if (inputElement.hasAttribute("validAge_sim_us"))
                                {
                                    std::string str = inputElement.getAttribute("validAge_sim_us");
                                    input_config.m_validAge_sim_us = a_util::strings::toInt64(str);
                                    // Fixme: Might check value !!!
                                }
                                else
                                {
                                    if (fep::isOk(result))
                                    {
                                        result = MAKE_ERROR("Missing attribute \"validAge_sim_us\"");
                                    }
                                }

                                // Attribute "name" is required
                                if (inputElement.hasAttribute("delay_sim_us"))
                                {
                                    std::string str = inputElement.getAttribute("delay_sim_us");
                                    input_config.m_delay_sim_us = a_util::strings::toInt64(str);
                                    if (input_config.m_delay_sim_us < 0)
                                    {
                                        result = MAKE_ERROR("Invalid attribute \"delay_sim_us\"");
                                    }
                                }

                                // Read stepListenerConfig: inputViolationStrategy
                                if (inputElement.hasAttribute("inputViolationStrategy"))
                                {
                                    std::string str = inputElement.getAttribute("inputViolationStrategy");
                                    input_config.m_inputViolationStrategy = InputViolationStrategy_fromString(str.c_str());

                                    if (input_config.m_inputViolationStrategy == IS_UNKNOWN)
                                    {
                                        if (fep::isOk(result))
                                        {
                                            result = MAKE_ERROR("Invalid attribute \"inputViolationStrategy\"");
                                        }
                                    }
                                }
                                else
                                {
                                    if (fep::isOk(result))
                                    {
                                        result = MAKE_ERROR("Missing attribute \"inputViolationStrategy\"");
                                    }
                                }

                                if (fep::isOk(result))
                                {
                                    step_config.m_inputs.insert(std::make_pair(input_name, input_config));
                                }
                            }
                        }

                        DOMElementList outputElementList;
                        if (stepElement.findNodes("outputs/output", outputElementList))
                        {
                            for (DOMElementList::iterator it3 = outputElementList.begin(); it3 != outputElementList.end(); ++it3)
                            {
                                DOMElement& outputElement = *it3;
                                std::string output_name;
                                OutputConfig output_config;
                                output_config.m_handle = NULL; // Invalid handle

                                // Attribute "name" is required
                                if (outputElement.hasAttribute("name"))
                                {
                                    output_name = outputElement.getAttribute("name");
                                    if (output_name.empty())
                                    {
                                        if (fep::isOk(result))
                                        {
                                            result = MAKE_ERROR("Empty attribute \"name\"");
                                        }
                                    }
                                }
                                else
                                {
                                    if (fep::isOk(result))
                                    {
                                        result = MAKE_ERROR("Missing attribute \"name\"");
                                    }
                                }

                                if (fep::isOk(result))
                                {
                                    step_config.m_outputs.insert(std::make_pair(output_name, output_config));
                                }
                            }
                        }

                        participant.m_tasks.insert(std::make_pair(step_name, step_config));
                    }
                }

                {
                    // Read Inputs
                    DOMElementList inputElementList;
                    if (participantElement.findNodes("inputs/input", inputElementList))
                    {
                        for (DOMElementList::iterator it2 = inputElementList.begin(); it2 != inputElementList.end(); ++it2)
                        {
                            DOMElement& inputElement = *it2;
                            std::string input_name;
                            GlobalInputConfig globalInputConfig;

                            // Attribute "name" is required
                            if (inputElement.hasAttribute("name"))
                            {
                                input_name = inputElement.getAttribute("name");
                                if (input_name.empty())
                                {
                                    if (fep::isOk(result))
                                    {
                                        result = MAKE_ERROR("Empty attribute \"name\"");
                                    }
                                }
                            }
                            else
                            {
                                if (fep::isOk(result))
                                {
                                    result = MAKE_ERROR("Missing attribute \"name\"");
                                }
                            }

                            // Read inputConfig: backLogSize
                            if (inputElement.hasAttribute("backLogSize"))
                            {
                                std::string str = inputElement.getAttribute("backLogSize");
                                globalInputConfig.m_backLogSize = a_util::strings::toInt32(str);
                                if (globalInputConfig.m_backLogSize == 0)
                                {
                                    result = MAKE_ERROR("Invalid attribute \"backLogSize\"");
                                }
                            }
                            else
                            {
                                if (fep::isOk(result))
                                {
                                    result = MAKE_ERROR("Missing attribute \"backLogSize\"");
                                }
                            }

                            if (fep::isOk(result))
                            {
                                participant.m_input_configs.insert(std::make_pair(input_name, globalInputConfig));
                            }
                        }
                    }
                }

                if (fep::isOk(result))
                {
                    timing_configuration.m_participants.insert(std::make_pair(participant_name, participant));
                }
            }
        }

        // Element "participant" is required
        if (timing_configuration.m_participants.size() == 0)
        {
            if (fep::isOk(result))
            {
                result = MAKE_ERROR("Missing element \"participant\"");
            }
        }

    }


    return result;
}

fep::Result TimingConfig::readTimingConfigFromFile(const std::string& filename, TimingConfiguration& timing_configuration)
{
    using namespace a_util::xml;
    using namespace a_util::filesystem;

    fep::Result result = ERR_NOERROR;

    // Read dom 
    DOM dom;
    if (!dom.load(Path(filename).toString()))
    {
        result = MAKE_ERROR("Failed to parse file");
    }

    if (fep::isOk(result))
    {
        result = internal_readTimingConfigFromDOM(dom, filename, timing_configuration);
    }

    return result;
}

fep::Result TimingConfig::readTimingConfigFromString(const std::string& xml_string, TimingConfiguration& timing_configuration)
{
    using namespace a_util::xml;

    fep::Result result = ERR_NOERROR;

    // Dummy filename used for error reporting
    static const std::string filename= "<internal>";

    // Read dom 
    DOM dom;
    if (!dom.fromString(xml_string))
    {
        result = MAKE_ERROR("Failed to parse file");
    }

    if (fep::isOk(result))
    {
        result = internal_readTimingConfigFromDOM(dom, "<internal>", timing_configuration);
    }

    return result;
}

static fep::Result writeTimingConfigToStream(std::ostream& os, const TimingConfiguration& timing_configuration)
{
    os << "<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>" << std::endl;
    os << "<timing xmlns:timing=\"timing\">" << std::endl;

    os << " <header>" << std::endl;
    os << "  <author>" << timing_configuration.m_header.m_author << "</author>" << std::endl;
    os << "  <date_creation>" << timing_configuration.m_header.m_date_creation << "</date_creation>" << std::endl;
    os << "  <date_change>" << timing_configuration.m_header.m_date_change << "</date_change>" << std::endl;
    os << "  <description>" << timing_configuration.m_header.m_description << "</description>" << std::endl;
    os << " </header>" << std::endl;

    os << " <participants>" << std::endl;

    for (std::map<std::string,Participant>::const_iterator it1 = timing_configuration.m_participants.begin(); it1 != timing_configuration.m_participants.end(); ++it1)
    {
        const std::string& participant_name = it1->first;
        const Participant& participant = it1->second;
        os << "    <participant name=\"" << participant_name << "\">" << std::endl;
        os << "        <steps>" << std::endl;

        for (std::map<std::string,StepConfig>::const_iterator it2 = participant.m_tasks.begin(); it2 != participant.m_tasks.end(); ++it2)
        {
            const std::string& step_name = it2->first;
            const StepConfig& step_config = it2->second;

            os << "            <step"
                << " name=\"" << step_name << "\""
                << " cycleTime_sim_us=\"" << step_config.m_cycleTime_sim_us << "\""
                << " maxRuntime_us=\"" << step_config.m_maxRuntime_us << "\""
                << " maxInputWaittime_us=\"" << step_config.m_maxInputWaittime_us << "\""
                << " runtimeViolationStrategy=\"" << TimeViolationStrategy_toString(step_config.m_runtimeViolationStrategy) << "\""
                << ">" << std::endl;
            os <<  "                <inputs>" << std::endl;

            for (std::map<std::string, InputConfig>::const_iterator it3 = step_config.m_inputs.begin(); it3 != step_config.m_inputs.end(); ++it3)
            {
                const std::string& input_name= it3->first;
                const InputConfig& input_config = it3->second;
                os << "                    <input"
                    << " name=\"" << input_name << "\""
                    << " validAge_sim_us=\"" << input_config.m_validAge_sim_us << "\""
                    << " delay_sim_us=\"" << input_config.m_delay_sim_us << "\""
                    << " inputViolationStrategy=\"" << InputViolationStrategy_toString(input_config.m_inputViolationStrategy) << "\""
                    << " />" << std::endl;
            }
            os << "                </inputs>" << std::endl;
            os << "            </step>" << std::endl;
        }
        os << "        </steps>" << std::endl;

        os << "        <inputs>" << std::endl;
        for (std::map<std::string, GlobalInputConfig>::const_iterator it2 = participant.m_input_configs.begin(); it2 != participant.m_input_configs.end(); ++it2)
        {
            const std::string& input_name = it2->first;
            const GlobalInputConfig& global_input_config = it2->second;
            os << "            <input name=\"" << input_name << "\" backLogSize=\"" << global_input_config.m_backLogSize << "\" />" << std::endl;
        }
        os << "        </inputs>" << std::endl;

        os << "    </participant>" << std::endl;
    }
    os << " </participants>" << std::endl;
    os << "</timing>" << std::endl;
    return ERR_NOERROR;
}


fep::Result TimingConfig::writeTimingConfigToFile(const std::string& filename, const TimingConfiguration& timing_configuration)
{
    using namespace a_util::filesystem;

    fep::Result result = ERR_NOERROR;
    
    std::ofstream os;
    os.open(Path(filename).toString().c_str(), std::ios_base::out);
    if (!os.is_open())
    {
        result = fep::ERR_INVALID_FILE;
    }

    if (fep::isOk(result))
    {
        result = writeTimingConfigToStream(os, timing_configuration);
    }

    return result;
}

fep::Result TimingConfig::writeTimingConfigToString(std::string& resString, const TimingConfiguration& timing_configuration)
{
    std::ostringstream os;

    fep::Result result = writeTimingConfigToStream(os, timing_configuration);

    if (fep::isOk(result))
    {
        resString = os.str();
    }

    return result;
}
