/**
 * Implementation of an snippet hosting FEP Participant :P
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
#include <fep_participant_sdk.h>

#include <stdlib.h>
#include <memory>
#include <iostream>

#define main main_v1
//! [ModuleConfigAPI]
int main(int nArgc, const char* pArgv[])
{
    cModuleOptions oModuleOptions;
    oModuleOptions.SetParticipantName("AnElementName");
    oModuleOptions.SetDomainId(42);
    oModuleOptions.SetTransmissionType(fep::TT_RTI_DDS);
    oModuleOptions.AppendToNetworkInterfaceList("192.168.3.*");
    
    cModule oModule;
    oModule.Create(oModuleOptions);

    // ... do work

    oModule.Destroy();

    return 0;
}
//! [ModuleConfigAPI]
#undef main

#define main main_v2
//! [ModuleConfigCMD]
int main(int nArgc, const char* pArgv[])
{
    cModuleOptions oModuleOptions;
    oModuleOptions.ParseCommandLine(nArgc, pArgv);

    cModule oModule;
    oModule.Create(oModuleOptions);

    // ... do work

    oModule.Destroy();

    return 0;
}
//! [ModuleConfigCMD]
#if 0
//! [ModuleConfigCMD_add]
int main(int nArgc, const char* pArgv[])
{
    cModuleOptions oModuleOptions;
    std::string strMonitoredValue = "";
    oModuleOptions.SetAdditionalOption(strMonitoredValue,
		"-s",
		"--search",
		"(help text) Search for FEP Participants with the argument name",
		"FEP Participant name"
	) 	
    oModuleOptions.ParseCommandLine(nArgc, pArgv);

    
    cModule oModule;
    oModule.Create(oModuleOptions);

    // ... do work

    oModule.Destroy();

    return 0;
}
//! [ModuleConfigCMD_add]
#endif 
#if 0
//! [ModuleConfigCMD_all]
(-n, --name, -d, --domain, -t, --transmission, -i, --interface, -h, --help, -v, --version)
//! [ModuleConfigCMD_all]
#endif 
#if 0
//! [ModuleConfigCMD_use]
<fep_executable> --name AnElementName --domain 42 --transmission RTI_DDS --interface 192.168.3.*
//! [ModuleConfigCMD_use]
#endif 
#if 0
//! [ModuleConfigCMD_interface_ip_address]
<fep_executable> --interface 192.168.3.1
//! [ModuleConfigCMD_interface_ip_address]
//! [ModuleConfigCMD_interface_ip_range]
<fep_executable> --interface 192.168.3.0/24
<fep_executable> --interface 192.168.3/24
<fep_executable> --interface 10/8
//! [ModuleConfigCMD_interface_ip_range]
//! [ModuleConfigCMD_interface_ip_wildcard]
<fep_executable> --interface 192.168.3.*
//! [ModuleConfigCMD_interface_ip_wildcard]
//! [ModuleConfigCMD_interface_hostname]
<fep_executable> --interface AUDIINLXXXXXX
//! [ModuleConfigCMD_interface_hostname]
//! [ModuleConfigCMD_interface_interfacename]
<fep_executable> --interface LAN-Verbindung
//! [ModuleConfigCMD_interface_interfacename]
//! [ModuleConfigCMD_interface_multiple]
<fep_executable> --interface 10/8,192.168.3/24
//! [ModuleConfigCMD_interface_multiple]
//! [ModuleConfigAPI_interface_multiple]
oModuleOptions.AppendToNetworkInterfaceList("10/8");
oModuleOptions.AppendToNetworkInterfaceList("192.168.3/24");
//! [ModuleConfigAPI_interface_multiple]
#endif 
#undef main
#define main main_v3
//! [ModuleConfig_Recommended]
int main(int nArgc, const char* pArgv[])
{
    cModuleOptions oModuleOptions;
    oModuleOptions.SetParticipantName("TheDefaultElementName");
    oModuleOptions.ParseCommandLine(nArgc, pArgv);

    cModule oModule;
    oModule.Create(oModuleOptions);

    // ... do work

    oModule.Destroy();

    return 0;
}
//! [ModuleConfig_Recommended]
#undef main

#define main main_v4
//! [ModuleConfig_TwoModules]
int main(int nArgc, const char* pArgv[])
{
    cModuleOptions oDefaultModuleOptions;
    oDefaultModuleOptions.ParseCommandLine(nArgc, pArgv);

    cModuleOptions oModuleOptions1(oDefaultModuleOptions);
    oModuleOptions1.SetParticipantName("FirstElementName");

    cModule oModule1;
    oModule1.Create(oModuleOptions1);

    cModuleOptions oModuleOptions2(oDefaultModuleOptions);
    oModuleOptions2.SetParticipantName("SecondElementName");

    cModule oModule2;
    oModule2.Create(oModuleOptions2);

    // ... do work

    oModule1.Destroy();
    oModule2.Destroy();

    return 0;
}
//! [ModuleConfig_TwoModules]
#undef main


/**
 *
 * Main function of the FEP application
 *
 * This merely serves as a "script" to drive several use-cases involving
 * module configuration.
 *
 * @param [in] nArgc Argument count
 * @param [in] pArgv Argument pointer
 *
 * @return Returns a standard system result code.
 */
int main(int nArgc, const char* pArgv[])
{
    main_v1(nArgc, pArgv);
    main_v2(nArgc, pArgv);
    main_v3(nArgc, pArgv);
    main_v4(nArgc, pArgv);

    return 0;
}