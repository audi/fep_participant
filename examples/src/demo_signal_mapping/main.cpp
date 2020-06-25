/************************************************************************
 * Implementation of the signal mapping demo
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

#include "signal_producer.h"
#include "signal_consumer.h"
#include <a_util/system/system.h>
#include <fep_participant_sdk.h>

int main(int nArgc, const char* pArgv[])
{
    fep::cModuleOptions oModuleOptions;
    if (fep::isFailed(oModuleOptions.ParseCommandLine(nArgc, pArgv)))
    {
        return 1;
    }

    cSignalProducer oProducer;
    fep::cModuleOptions oProducerModuleOptions(oModuleOptions);
    oProducerModuleOptions.SetParticipantName("Producer");
    if (fep::isFailed(oProducer.Create(oProducerModuleOptions)))
    {
        return 1;
    }

    cSignalConsumer oConsumer;
    fep::cModuleOptions oConsumerModuleOptions(oModuleOptions);
    oConsumerModuleOptions.SetParticipantName("Consumer");
    if (fep::isFailed(oConsumer.Create(oConsumerModuleOptions)))
    {
        return 1;
    }

    oProducer.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, oProducer.GetName());
    oConsumer.GetPropertyTree()->SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, oProducer.GetName());

    // init timing client first
    oConsumer.GetStateMachine()->InitializeEvent();
    oConsumer.WaitForState(fep::FS_READY);

    oProducer.GetStateMachine()->InitializeEvent();
    oProducer.WaitForState(fep::FS_READY);

    // start timing client first
    oConsumer.GetStateMachine()->StartEvent();
    oConsumer.WaitForState(fep::FS_RUNNING);

    oProducer.GetStateMachine()->StartEvent();
    oProducer.WaitForState(fep::FS_RUNNING);

    // let them run 10s
    a_util::system::sleepMilliseconds(10 * 1000);

    oConsumer.GetStateMachine()->StopEvent();
    oProducer.GetStateMachine()->StopEvent();
    oConsumer.WaitForState(fep::FS_IDLE);
    oProducer.WaitForState(fep::FS_IDLE);

    oProducer.Destroy();
    oConsumer.Destroy();
}
