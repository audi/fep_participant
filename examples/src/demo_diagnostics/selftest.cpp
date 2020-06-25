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
#include <iostream>
#include "common.h"
#include "element_client.h"
#include "element_server.h"
#include "selftest.h"
#include <fep_participant_sdk.h>
#include "a_util/strings.h"
#include "a_util/system.h"

int selftest::runClient(const fep::cModuleOptions& oModuleOptions)
{
    timestamp_t tmMinTimeout = 0;
    bool bSuccess = false;

    cElementClient* pClientFEP = new cElementClient();
    fep::cModuleOptions oOptions = oModuleOptions;
    oOptions.SetParticipantName(FEP_CLIENT_IDENTIFIER);
    if  (fep::isFailed(pClientFEP->Create(oOptions)))
    {
        std::cerr << "Could not create client FEP element. This is fatal" << std::endl;
        return 2;
    }

    fep::AutomationInterface AI;

    timestamp_t tmTimeouts[6] = {100, 250, 500, 1000, 2500, MAX_TIMEOUT};
    for (int8_t i=0; i < 6; i++)
    {
        fep::tState eState;
        fep::Result resGetElementState = AI.GetParticipantState(eState,
            FEP_SERVER_IDENTIFIER, tmTimeouts[i]); 
        if (resGetElementState == fep::ERR_TIMEOUT)
        {
            std::cerr<<"Could not retrieve server element state within a timeout of " << tmTimeouts[i] <<" ms."<<std::endl;
        }
        else if (fep::isOk(resGetElementState))
        {
            tmMinTimeout = tmTimeouts[i];
            std::cout << "Retrieved server element state within less than " << tmTimeouts[i] << " ms."<< std::endl;
            bSuccess = true;
            break;
        } else
        {
            std::cerr << "Error-Code: " << resGetElementState.getErrorCode() << std::endl;
            std::cerr << "Fatal error occcured. Not due to the timeout." << std::endl;
            return 2;
        }
    }
    if (!bSuccess)
    {
        std::cerr << "Could not retrieve server element state within a maximum timeout of " << MAX_TIMEOUT << " ms." << std::endl;
        tmMinTimeout = -1;
    }

    pClientFEP->SetSignaltest(true);
    while (!pClientFEP->GetSignaltestDone())
    {
        a_util::system::sleepMilliseconds(1000);
    }

    pClientFEP->SetPropertytest(true, 2 * tmMinTimeout);
    while (!pClientFEP->GetPropertytestDone())
    {
        a_util::system::sleepMilliseconds(1000);
    }

    // Evaluating test results and formatting their output
    bool remotePropRet = pClientFEP->GetRemotePropertyReceived();
    timestamp_t roundtriptime = pClientFEP->GetRoundTripTime();
    std::cout << "Measurements finished." << std::endl;
    std::cout << std::endl << std::endl << "_______________________________________________" << std::endl;
    std::cout << "____________________SUMMARY____________________" << std::endl;
    std::ostringstream ss;
    ss << tmMinTimeout;
    std::cout << "Minimal timeout: " << (tmMinTimeout <= 0 ? "-" : ss.str()) << " ms" << std::endl;
    ss.str(std::string());
    ss << roundtriptime/1e6 << " s";
    std::cout << "Signal roundtrip time: " << (roundtriptime <= 0 ? "failed" : ss.str()) << std::endl;
    std::cout << "Retrieval of remote property: " << (remotePropRet ? "successful" : "failed") << std::endl;
    std::cout << "_______________________________________________" << std::endl << std::endl;
    std::cout << "Exiting ..." << std::endl;

    pClientFEP->TerminateClient();
    pClientFEP->WaitForState(fep::FS_IDLE, MAX_TIMEOUT);
    pClientFEP->Destroy();
    return 0;
}

int selftest::main(const fep::cModuleOptions & oModuleOptions, std::string strProgName,
    bool bServerFlag, bool bClientFlag, bool bAutoFlag)
{
    int nResult = 0;
    bool bMixedModus = (!bServerFlag & !bClientFlag) || (bServerFlag & bClientFlag);
    
    if (bMixedModus)
    {
        std::cout << "Running in mixed-mode (Server & Client started by same process)!" << std::endl;
        cElementServer* pServerFEP = new cElementServer();

        fep::cModuleOptions oMyModuleOptions(oModuleOptions);
        oMyModuleOptions.SetParticipantName(FEP_SERVER_IDENTIFIER);
        if (fep::isFailed(pServerFEP->Create(oMyModuleOptions)))
        {
            std::cerr << "Could not create server FEP element. This is fatal" << std::endl;
            exit(2);
        }
        std::cout << "ServerFEP created!" << std::endl;
        std::cout << "Enter Ctrl-C to abort selftest" << std::endl;
        nResult = runClient(oModuleOptions);
        if (nResult == 2)
        {
            exit(2);
        }
        while (!pServerFEP->IsDone())
        {
            a_util::system::sleepMilliseconds(1000);
        }
        pServerFEP->TerminateServer();
        pServerFEP->WaitForState(fep::FS_IDLE, MAX_TIMEOUT);
        pServerFEP->Destroy();
    }
    else if (bClientFlag)
    {
        std::cout << "Running in client mode" << std::endl;
        if (!bAutoFlag)
        {
            while (1)
            {
                std::cout << "Is server ready for measurement?[y|quit]" << std::endl;
                std::string strInput;
                std::cin >> strInput;
                if (0 == strInput.compare("y"))
                {
                    nResult = runClient(oModuleOptions);
                    break;
                }
                else if (0 == strInput.compare("quit"))
                {
                    nResult = 0;
                    break;
                }
            }
        }
        else
        {
            nResult = runClient(oModuleOptions);
        }
    }
    else if (bServerFlag)
    {
        std::cout << "Running in Server-Mode!" << std::endl;
        cElementServer* pServerFEP = new cElementServer();

        fep::cModuleOptions oMyModuleOptions(oModuleOptions);
        oMyModuleOptions.SetParticipantName(FEP_SERVER_IDENTIFIER);
        if (fep::isFailed(pServerFEP->Create(oMyModuleOptions)))
        {
            std::cerr << "Could not create server FEP element. This is fatal" << std::endl;
            exit(2);
        }
        if (!bAutoFlag)
        {
            while (1)
            {
                std::cout << "Enter \"quit\" to shutdown server element. [quit]" << std::endl;
                std::string strInput;
                std::cin >> strInput;

                if (0 == strInput.compare("quit"))
                {
                    break;
                }
            }
        }
        else
        {
            while (!pServerFEP->IsDone())
            {
                a_util::system::sleepMilliseconds(1000);
            }
            a_util::system::sleepMilliseconds(1000);
        }
        pServerFEP->TerminateServer();
        pServerFEP->WaitForState(fep::FS_IDLE, MAX_TIMEOUT);
        pServerFEP->Destroy();
        nResult = 0;
    }
    return nResult;
}
