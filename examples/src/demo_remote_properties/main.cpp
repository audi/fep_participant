/**
 * Implementation of a FEP Element Application that can read remote properties
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

#include "remote_properties.h"
#include <iostream>
#include <string>
#include <stdlib.h>

int main(int nArgc, const char* pArgv[])
{
	fep::cModuleOptions oModuleOptions("RemoteProperties");

    // Using cModuleOptions::SetAdditionalOption method to improve our command 
    // line arguments evaluation
	std::string strElement = std::string();
	oModuleOptions.SetAdditionalOption(strElement, "-r", "--remoteElement",
		"(Required Argument) set the name of the remote element.", "string");

	std::string strPath = std::string();
	oModuleOptions.SetAdditionalOption(strPath, "-p", "--propertyPath",
		"(Optional Argument) give the Path of the Property to read.", "string");

	std::string strTimeout = std::string();
	oModuleOptions.SetAdditionalOption(strTimeout, "-T", "--timeout",
		"(Optional Argument) define a timeout for the response.", "int");

	fep::Result nResult = oModuleOptions.ParseCommandLine(nArgc, pArgv);
	
	if (fep::isFailed(nResult))
	{
		std::cerr << ">>>> Failed to parse command line." << std::endl;
		std::cout << "Usage: " << "demo_remote_properties[.exe] -r/--remoteElement <Remote Element> [-p/propertyPath <Property Path>] [-T/--timeout <Timeout (s)>]\n";
		std::cout << "Default: Property Path = root path, Timeout = 3s\n";
		return 1;
	}

	timestamp_t tmTimeout = (timestamp_t)3e3;
	
	if (!strTimeout.empty())
	{
	    tmTimeout = (timestamp_t)(atof(strTimeout.c_str()) * 1000);
	}

	if (strElement.empty())
	{
	    std::cout << "Error: empty element name!\n";
	    return 1;
	}

	cRemoteProperties pElement(strElement.c_str(), strPath.c_str(), tmTimeout);

	// the element will drive itself through the state machine from this point
	if (fep::isFailed(pElement.Run(oModuleOptions)))
	{
	    std::cout << "Element could not initialize itself!\n";
	}

	return pElement.GetResultCode();

}
