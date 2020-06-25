/**
 * Implementation used as stimuli application for performance measurements.
 *
 * @file

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
 */

#include "stdafx.h"

#include "module_base.h"
#include "module_client.h"
#include "module_server.h"
#include "bus_check_helper.h"
#include "bus_check_base.h"
#include "bus_check_set_property_helper.h"
#include "bus_check_custom_command.h"
#include "bus_check_control_command.h"
#include "bus_check_set_property_command.h"
#include "bus_check_get_property_command.h"
#include "bus_check_delete_property_command.h"
#include "bus_check_reg_prop_listener_command.h"
#include "bus_check_unreg_prop_listener_command.h"
#include "bus_check_signal_transmission.h"
#include "bus_check_get_signal_info_command.h"
#include "bus_check_resolve_signal_type_command.h"
#include "bus_check_signal_description_command.h"
#include "bus_check_result_code_notification.h"
#include "bus_check_mapping_configuration_command.h"
#include "bus_check_name_change_command.h"
#include "bus_check_state_changes.h"
#include "bus_check_incidents.h"
#include "bus_check_property_mirror.h"
#include "bus_check_mute_signal_command.h"

#include <iostream>
#include <fstream>

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
#include "a_utils.h"
#else
#include "a_util/system.h"
#endif

using namespace fep;

struct cBusCheckOptions
{
    enum { UNDEFINED_MODE, SERVER_MODE, CLIENT_MODE} nRunMode;
 
    std::string strOutputFile;
    std::string strDomainId;
    std::string strServerName;
    std::string strClientName;
};

static const char* s_strDefaultServerName= "BusCompat" "Server";
static const char* s_strDefaultClientName= "BusCompat" "Client";

static int server_main(cBusCheckOptions& oProgramOptions)
{
    cModuleServer oModuleServer;

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
    if (!oProgramOptions.strDomainId.empty())
    {
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
        oModuleServer.SetDomainId(static_cast<uint16_t>(atoi(oProgramOptions.strDomainId.c_str())));
#else
        oModuleServer.SetDomainId(static_cast<uint16_t>(a_util::strings::to_int32(oProgramOptions.strDomainId)));
#endif
    }

    oModuleServer.Create(oProgramOptions.strServerName.c_str());
#else
    cModuleOptions oModuleOptions(oProgramOptions.strServerName.c_str());
    if (!oProgramOptions.strDomainId.empty())
    {
        oModuleOptions.SetDomainId(static_cast<uint16_t>(a_util::strings::toInt32(oProgramOptions.strDomainId)));
    }

    oModuleServer.Create(oModuleOptions);
#endif

    oModuleServer.WaitForState(FS_SHUTDOWN);

    oModuleServer.Destroy();

    return 0;
}



static int client_main(cBusCheckOptions& oProgramOptions)
{
    std::ofstream* pFileOutputStream= NULL;
    if (!oProgramOptions.strOutputFile.empty())
    {
        pFileOutputStream= new std::ofstream(oProgramOptions.strOutputFile.c_str(), std::ofstream::out);
    }
    std::ostream& oOutputStream= pFileOutputStream ? *pFileOutputStream : std::cerr;

 
    cModuleClient oModuleClient;
    oModuleClient.SetServerElementName(oProgramOptions.strServerName.c_str());

#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)
    if (!oProgramOptions.strDomainId.empty())
    {
        oModuleClient.SetDomainId(static_cast<uint16_t>(atoi(oProgramOptions.strDomainId.c_str())));
    }

    oModuleClient.Create(oProgramOptions.strClientName.c_str());
#else
    cModuleOptions oModuleOptions(oProgramOptions.strClientName.c_str());
    if (!oProgramOptions.strDomainId.empty())
    {
        oModuleOptions.SetDomainId(static_cast<uint16_t>(a_util::strings::toInt32(oProgramOptions.strDomainId)));
    }

    oModuleClient.Create(oModuleOptions);
#endif

    oModuleClient.WaitForState(FS_RUNNING);

    // Initialize the checks
    cBusCheckBase::SetClientModule(&oModuleClient);

    // Wait to settle ... state of client module
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)^M
    fep_util::cSystem::Sleep(1 * 1000 * 1000);
#else
    a_util::system::sleepMicroseconds(1 * 1000 * 1000);
#endif

    // Do Checks: 1. Control Command
#define CHECK(check_class,desc) cBusCheckHelper<check_class>(desc, oOutputStream)

    CHECK(cBusCheckControlCommand, "Control Command requesting Initialize").args(CE_Initialize);
    CHECK(cBusCheckControlCommand, "Control Command requesting Start").args(CE_Start);
    CHECK(cBusCheckControlCommand, "Control Command requesting Stop").args(CE_Stop);
    CHECK(cBusCheckControlCommand, "Control Command requesting Shutdown").args(CE_Shutdown);
    CHECK(cBusCheckControlCommand, "Control Command requesting ErrorFixed").args(CE_ErrorFixed);
    CHECK(cBusCheckControlCommand, "Control Command requesting Restart").args(CE_Restart);

    // Do Checks: 2. Custom Command
    CHECK(cBusCheckCustomCommand, "Custom Command without Parameters").args("CommandWithoutParameters", "{}");
    CHECK(cBusCheckCustomCommand, "Custom Command with Integer Parameter").args("CommandWithInteger", "{ \"IValue\" : \"12\" }");
    CHECK(cBusCheckCustomCommand, "Custom Command with Float Parameter").args("CommandWithFloatr", "{ \"FValue\" : \"12.34\" }");
    CHECK(cBusCheckCustomCommand, "Custom Command with String Parameter").args("CommandWithString", "{ \"SValue\" : \"Hallo\" }");
    CHECK(cBusCheckCustomCommand, "Custom Command with Multiple Parameters").args("CommandWithMisc", "{ \"IValue\" : \"12\", \"FValue\" : \"12.34\", \"SValue\" : \"Hallo\" }");

    // Do Checks: 3. SetProperty Command AND 4. GetProperty Command
    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with Boolean Value (true)").args("test.xBoolean", true);
    CHECK(cBusCheckGetPropertyCommand, "GetProperty Command with Boolean Value (true)").args("test.xBoolean", true);

    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with Boolean Value (false)").args("test.xBoolean", false);
    CHECK(cBusCheckGetPropertyCommand, "GetProperty Command with Boolean Value (false)").args("test.xBoolean", false);

    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with Integer Value (0)").args("test.xInteger", static_cast<int32_t>(0)); 
    CHECK(cBusCheckGetPropertyCommand, "GetProperty Command with Integer Value (0)").args("test.xInteger", static_cast<int32_t>(0)); 

    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with Integer Value (1)").args("test.xInteger", static_cast<int32_t>(1));
    CHECK(cBusCheckGetPropertyCommand, "GetProperty Command with Integer Value (1)").args("test.xInteger", static_cast<int32_t>(1)); 

    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with Integer Value (2)").args("test.xInteger", static_cast<int32_t>(2)); 
    CHECK(cBusCheckGetPropertyCommand, "GetProperty Command with Integer Value (2)").args("test.xInteger", static_cast<int32_t>(2)); 

    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with Integer Value (-1)").args("test.xInteger", static_cast<int32_t>(-1));
    CHECK(cBusCheckGetPropertyCommand, "GetProperty Command with Integer Value (-1)").args("test.xInteger", static_cast<int32_t>(-1)); 

    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with Float Value (0.0)").args("test.xFloat", 0.0);
    CHECK(cBusCheckGetPropertyCommand, "GetProperty Command with Float Value (0.0)").args("test.xFloat", 0.0);

    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with Float Value (12.34)").args("test.xFloat", 12.34);
    CHECK(cBusCheckGetPropertyCommand, "GetProperty Command with Float Value (12.34)").args("test.xFloat", 12.34);

    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with Float Value (-34.56)").args("test.xFloat", -34.56);
    CHECK(cBusCheckGetPropertyCommand, "GetProperty Command with Float Value (-34.56)").args("test.xFloat", -34.56);

    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with String Value (Simple)").args("test.xString", "Hello");
    CHECK(cBusCheckGetPropertyCommand, "GetProperty Command with String Value (Simple)").args("test.xString", "Hello");

    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with String Value (Multiple)").args("test.xString", "Hello World!");
    CHECK(cBusCheckGetPropertyCommand, "GetProperty Command with String Value (Multiple)").args("test.xString", "Hello World!");

    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with String Value (Special)").args("test.xString", "Hello \"World\" ... this is !\\�$%&/()=' Calling");
    CHECK(cBusCheckGetPropertyCommand, "GetProperty Command with String Value (Special)").args("test.xString", "Hello \"World\" ... this is !\\�$%&/()=' Calling");

    // Do Checks: 5. DeleteProperty Command
    CHECK(cBusCheckDeletePropertyCommand, "DeleteProperty Command on Boolean Value").args("test.xBoolean");
    CHECK(cBusCheckDeletePropertyCommand, "DeleteProperty Command on Integer Value").args("test.xInteger");
    CHECK(cBusCheckDeletePropertyCommand, "DeleteProperty Command on Float Value").args("test.xFloat");
    CHECK(cBusCheckDeletePropertyCommand, "DeleteProperty Command on String Value").args("test.xString");

    // Do Checks: 6. RegPropListener Command
    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with Boolean Value (true)").args("test.xBoolean", true);
    CHECK(cBusCheckRegPropListenerPropertyCommand, "RegPropListener Command on Boolean Value").args("test.xBoolean"); 

    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with Integer Value (0)").args("test.xInteger", static_cast<int32_t>(0)); 
    CHECK(cBusCheckRegPropListenerPropertyCommand, "RegPropListener Command on Integer Value").args("test.xInteger"); 

    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with Float Value (0.0)").args("test.xFloat", 0.0);
    CHECK(cBusCheckRegPropListenerPropertyCommand, "RegPropListener Command on Float Value").args("test.xFloat"); 

    CHECK(cBusCheckSetPropertyCommand, "SetProperty Command with String Value (Simple)").args("test.xString", "Hello");
    CHECK(cBusCheckRegPropListenerPropertyCommand, "RegPropListener Command on String Value").args("test.xString"); 

    // Do Checks: 7. UnregPropListener Command
    CHECK(cBusCheckUnregPropListenerPropertyCommand, "UnregPropListener Command on Boolean Value").args("test.xBoolean"); 
    CHECK(cBusCheckUnregPropListenerPropertyCommand, "UnregPropListener Command on Integer Value").args("test.xInteger"); 
    CHECK(cBusCheckUnregPropListenerPropertyCommand, "UnregPropListener Command on Float Value").args("test.xFloat"); 
    CHECK(cBusCheckUnregPropListenerPropertyCommand, "UnregPropListener Command on String Value").args("test.xString"); 

    // Do Checks: 8. Signal Transmission
    CHECK(cBusCheckSignalTransmission, "Signal Transmission with Signal BusCheckMixedSignal").args(); 

    // Do Checks: 9. GetSignalInfo Command
    CHECK(cBusCheckGetSignalInfoCommand, "GetSignalInfo Command on Signal BusCheckMixedSignal").args();

    // Do Checks: 10. ResolveSignalType Command
    CHECK(cBusCheckResolveSignalTypeCommand, "ResolveSignalType Command on Signal BusCheckMixedSignal").args("BusCheckMixedSignal");

    // Do Checks: 11. SignalDescription Command
    CHECK(cBusCheckSignalDescriptionCommand, "SignalDescription Command on Signal BusCheckMixedSignal").args("BusCheckMixedSignal");

    // Do Checks: 12. MappingConfiguration Command
    CHECK(cBusCheckMappingConfigurationCommand, "MappingConfiguration Command on Invalid Signal").args("XXXXX");
    
    // Do Checks: 13. NameChange Command
    CHECK(cBusCheckNameChangeCommand, "NameChange Command to New Name").args("BusCompat" "NoServer");
    oModuleClient.SetServerElementName("BusCompat" "NoServer");
    CHECK(cBusCheckNameChangeCommand, "NameChange Command to Old Name").args("BusCompat" "Server");
    oModuleClient.SetServerElementName("BusCompat" "Server");

    // Do Checks: 14. StateChanges 
    CHECK(cBusCheckSetPropertyHelper, "SetProperty Helper: Disable MirrorMode").args(cModuleServer::s_strMirrorModeEnabledConfig, false, 1 * 1000 * 1000);
    CHECK(cBusCheckSetPropertyHelper, "SetProperty Helper: Disable StandAloneMode").args(fep::component_config::g_strStateMachineStandAloneModePath_bEnable, false, 1 * 1000 * 1000);
    CHECK(cBusCheckStateChanges, "State Change to Idle (Stop Event)").args(CE_Stop, FS_IDLE, FS_RUNNING, 1 * 1000 * 1000);
    CHECK(cBusCheckSetPropertyHelper, "SetProperty Helper: Enable StandAloneMode").args(fep::component_config::g_strStateMachineStandAloneModePath_bEnable, true, 1 * 1000 * 1000);
    CHECK(cBusCheckSetPropertyHelper, "SetProperty Helper: Enable MirrorMode").args(cModuleServer::s_strMirrorModeEnabledConfig, true, 1 * 1000 * 1000);

    // Do Checks: 15. CheckIncidents
    CHECK(cBusCheckIncidents, "Incidents Check").args(static_cast<int16_t>(-1000), SL_Critical, "Hallo", 1000, "SourceFile.cpp");

    // Do Checks: 16. Mirror Properties 
    CHECK(cBusCheckSetPropertyHelper, "SetProperty Helper: Disable MirrorMode").args(cModuleServer::s_strMirrorModeEnabledConfig, false, 1 * 1000 * 1000);
    CHECK(cBusCheckPropertyMirror, "MirrorProperty Check").args("test.xRemote", "test.xLocal", static_cast<int32_t>(3)); 
    CHECK(cBusCheckSetPropertyHelper, "SetProperty Helper: Enable MirrorMode").args(cModuleServer::s_strMirrorModeEnabledConfig, true, 1 * 1000 * 1000);

    // Do Checks: 17. ResultCode Notification
    CHECK(cBusCheckResultCodeNotification, "ResultCode Notification with some value").args(42, 12345678);

    //Do Checks: 18. MuteSignal Command
    CHECK(cBusCheckMuteSignalCommand, "Mute Signal Command with muting flag true").args("SomeSignalName", fep::SD_Output, true);
    CHECK(cBusCheckMuteSignalCommand, "Mute Signal Command with muting flag false").args("SomeSignalName", fep::SD_Output, false);

    // Done ... Sleep for a while
#if (FEP_SDK_PARTICIPANT_VERSION_MAJOR == 1) && (FEP_SDK_PARTICIPANT_VERSION_MINOR < 1)^M
    fep_util::cSystem::Sleep(1 * 1000 * 1000);
#else
    a_util::system::sleepMicroseconds(1 * 1000 * 1000);
#endif

    // Destroy
    oModuleClient.Destroy();

    if (pFileOutputStream)
    {
        pFileOutputStream->close();
        delete pFileOutputStream;
    }

    return 0;
}

/* Some macros to help parsing arguments
 */
#define XARG_INIT() std::string strProgname(pArgv[0]); int nArgPos = 1
#define XARG_AVAILABLE() (nArgPos < nArgc)
#define XARG_GET_NEXT() std::string(pArgv[nArgPos++])
#define XARG_PEEK_NEXT() std::string(pArgv[nArgPos])

static void PrintUsage(std::ostream& os, const std::string& strProgname)
{
    os << "Usage:" << " " << strProgname.c_str() << " [options] (server|client)" << std::endl;
    os << "Available options are:" << std::endl;
    os << "    " << "-h,--help,-?           " << "Print help/usage information." << std::endl;
    os << "    " << "-d,--domain <id>       " << "Set domain id" << std::endl;
    os << "    " << "-o,--output <file>     " << "Set output file (client only)" << std::endl;
}


int main(int nArgc, const char* pArgv[])
{
    XARG_INIT();

    cBusCheckOptions oProgramOptions;
    oProgramOptions.nRunMode = cBusCheckOptions::UNDEFINED_MODE;
    oProgramOptions.strServerName= s_strDefaultServerName;
    oProgramOptions.strClientName= s_strDefaultClientName;

    while (XARG_AVAILABLE())
    {
        std::string strCurrentArg = XARG_GET_NEXT();

        if (strCurrentArg == "-?" || strCurrentArg == "-h" || strCurrentArg == "--help")
        {
            // Help requested: Print help and exit
            PrintUsage(std::cout, strProgname);
            return 0;
        }
        else if (strCurrentArg == "--domain" || strCurrentArg == "-d")
        {
            if (XARG_AVAILABLE())
            {
                oProgramOptions.strDomainId = XARG_GET_NEXT();
            }
            else
            {
                std::cerr << strProgname.c_str() << ":" << " required argument expected for option '" << strCurrentArg << "'" << std::endl;
                PrintUsage(std::cerr, strProgname);
                return 1;
            }
        }
        else if (strCurrentArg == "--output" || strCurrentArg == "-o")
        {
            if (XARG_AVAILABLE())
            {
                oProgramOptions.strOutputFile = XARG_GET_NEXT();
            }
            else
            {
                std::cerr << strProgname.c_str() << ":" << " required argument expected for option '" << strCurrentArg << "'" << std::endl;
                PrintUsage(std::cerr, strProgname);
                return 1;
            }
        }
        else if (strCurrentArg == "client")
        {
            oProgramOptions.nRunMode = cBusCheckOptions::CLIENT_MODE;
        }
        else if (strCurrentArg == "server")
        {
            oProgramOptions.nRunMode = cBusCheckOptions::SERVER_MODE;
        }
    }

    int res= 0;
    switch (oProgramOptions.nRunMode)
    {
    case cBusCheckOptions::UNDEFINED_MODE:
        std::cerr << strProgname.c_str() << ":" << " required argument for run mode: 'client' or 'server'" << std::endl;
        PrintUsage(std::cerr, strProgname);
        res= 1;
        break;
    case cBusCheckOptions::SERVER_MODE:
        res= server_main(oProgramOptions);
        break;
    case cBusCheckOptions::CLIENT_MODE:
        res= client_main(oProgramOptions);
        break;
    }
    
    return res;
}
