/**
 * Implementation of an exemplary FEP Element Application
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
#include <a_util/system.h>

#include <stdlib.h>
#include <iostream>

// Auxiliary macro evaluating an expression and exiting main in case of failure.
// This is used here for brevity. In your actual application you should 
// apply propper error handling!
#define EXIT_IF_FAILED(expr) \
    if (fep::isFailed(expr)) \
    { \
        std::cerr << "Expression \""#expr"\" failed [Line " << __LINE__ << ": "; \
        std::cerr << __FILE__ << "]" << std::endl; \
        exit(1); \
    }


#include "demo_master_element.h"
#include "demo_incident_element.h"

/*
 * Main function of this example
 *
 * This merely serves as a "script" to drive several use-cases involving
 * incidents.
 */
int main(int nArgc, const char* pArgv[])
{
    fep::cModuleOptions oModuleOptions("MasterElement");
    fep::Result nResult= oModuleOptions.ParseCommandLine(nArgc, pArgv);
    if (fep::isFailed(nResult))
    {
        std::cerr << ">>>> Failed to parse command line." << std::endl;
        return 1;
    }

    cMasterElement oMasterElement;
    EXIT_IF_FAILED(oMasterElement.Create(oModuleOptions));


    // Configuring the Master / and the Incident Strategy:
    EXIT_IF_FAILED(oMasterElement.GetPropertyTree()->SetPropertyValue(
                       MASTER_STRAT_PROP_BE_FUSSY, true));
    EXIT_IF_FAILED(oMasterElement.GetPropertyTree()->SetPropertyValue(
                       MASTER_STRAT_CRITICAL_ELEMENT, "BadlyCodedElement"));

    // Enabling stand-alone mode for the master Instance to prevent interference with
    // other elements issuing commands (for any reason)
    EXIT_IF_FAILED(oMasterElement.SetStandAloneModeEnabled(true));

    // attempt to start up the Master - this wont work since we expect it to have a properly
    // filled in participant header.
    oMasterElement.FireUpMaster();

    std::cerr << ">>>> The master refused to start. Apparently the participant header was " \
                 "not filled and its incident strategy did complain." << std::endl;

    // Option a) Disable fussiness
    EXIT_IF_FAILED(oMasterElement.GetPropertyTree()->SetPropertyValue(
                       MASTER_STRAT_PROP_BE_FUSSY, false));

    if (fep::isOk(oMasterElement.FireUpMaster()))
    {
        std::cout << ">>>> Disabled fussiness; now the master accepts to enter running state.";
        std::cout << std::endl;
    }

    std::cout << ">>>> Halting the master to correct the otherwise persistent issue of an ";
    std::cout << "invalid FEP Element Header and starting over afterwards." << std::endl;

    EXIT_IF_FAILED(oMasterElement.FillInElementHeader());
    EXIT_IF_FAILED(oMasterElement.HaltMaster());
    EXIT_IF_FAILED(oMasterElement.GetPropertyTree()->SetPropertyValue(
                       MASTER_STRAT_PROP_BE_FUSSY, true));

    // starting through should be working flawlessly now...
    EXIT_IF_FAILED(oMasterElement.FireUpMaster());

    std::cout << ">>>> Now the Element Header has been filled in properly and the Master "\
                 "reached running state." << std::endl;

    // Dealing with the slave Element which is being monitored by the Master..
    cBadlyCodedElement oBadlyCodedElement;
    EXIT_IF_FAILED(oBadlyCodedElement.Create("BadlyCodedElement"));
    a_util::system::sleepMilliseconds(2*1000);

    std::cout << ">>>> First of all, the slave element doesn't have its Element Header set ";
    std::cout << "up either. This will make the fussy master fail. So, filling the ";
    std::cout << "property tree for a second attempt and starting over." << std::endl;

    EXIT_IF_FAILED(oBadlyCodedElement.FillInElementHeader());
    EXIT_IF_FAILED(oMasterElement.FireUpMaster()); // <- this also resolves the Masters error state.

    std::cout << ">>>> Resolving the induced error state of the FEP Element." << std::endl;
    EXIT_IF_FAILED(oBadlyCodedElement.GetStateMachine()->ErrorFixedEvent());

    std::cout << ">>>> Constructing a secondary incident by setting the BadlyCoded Element ";
    std::cout << "into StandAloneMode - which generally is a pretty bad idea. ";
    std::cout << "For this demonstration, the Master Element is set up to resolve this issue ";
    std::cout << "remotely" << std::endl;

    EXIT_IF_FAILED(oBadlyCodedElement.SetStandAloneModeEnabled(true));
    a_util::system::sleepMilliseconds(2*1000);

    std::cout << ">>>> With all pre-conditions met, the system is able to reach running mode";
    std::cout << std::endl;

    EXIT_IF_FAILED(oMasterElement.InitializeSystem());
    EXIT_IF_FAILED(oMasterElement.StartSystem());
    a_util::system::sleepMilliseconds(5*1000);

    std::cout << ">>>> Shutting down the system to exit the demonstration" << std::endl;

    EXIT_IF_FAILED(oMasterElement.HaltSystem());
    EXIT_IF_FAILED(oMasterElement.HaltMaster());
    a_util::system::sleepMilliseconds(5*1000);

    oMasterElement.Destroy();
    oBadlyCodedElement.Destroy();

    std::cout << ">>>> MAIN::Example run complete. Exiting." << std::endl;
    return 0;
}
