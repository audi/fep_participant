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

#include <string>
#include <iostream>
#include <vector>

#include "a_util/concurrency.h"
#include "a_util/system.h"

#include <fep_participant_sdk.h>

#include "run_elements.h"
#include "FepElement.h"

using namespace fep;

// Default FEP Wait timeout
#define FEP_WAIT_TIMEOUT 2 * 1000

static std::vector<FepElement*> pFepElements;

extern "C" int start_fep_elements(const int domain_id, const int nElements, const char* pElementNameArgs[])
{
    std::vector<std::string> oElementNames;
    for (int i = 0; i < nElements; ++i)
    {
        oElementNames.push_back(pElementNameArgs[i]);
    }

    pFepElements.resize(oElementNames.size());

    for (std::size_t i = 0; i < oElementNames.size(); ++i)
    {
        std::string& oElementName = oElementNames[i];
        FepElement*& pFepElement = pFepElements[i];

        pFepElement = new FepElement();

        {
            fep::Result result;

            cModuleOptions oModuleOptions;
            oModuleOptions.SetDomainId(static_cast<const uint16_t>(domain_id));
            oModuleOptions.SetParticipantName(oElementName.c_str());

            pFepElement->Create(oModuleOptions);
            result = pFepElement->WaitForState(fep::FS_IDLE, FEP_WAIT_TIMEOUT);
            if (result != ERR_NOERROR)
            {
                std::cerr << "Error" << ":" << " failed to reach state FS_IDLE '" << result.getErrorLabel() << "' : " << result.getDescription() << std::endl;
                return 1;
            }
        }
    }

    return 0;
}

extern "C" int stop_fep_elements()
{
    for (std::size_t i = 0; i < pFepElements.size(); ++i)
    {
        FepElement*& pFepElement = pFepElements[i];

        pFepElement->GetStateMachine()->ShutdownEvent();
    }

    for (std::size_t i = 0; i < pFepElements.size(); ++i)
    {
        FepElement*& pFepElement = pFepElements[i];

        pFepElement->WaitForShutdown();
        pFepElement->Destroy();

        delete pFepElement;

        pFepElement = nullptr;
    }

    return 0;
}
