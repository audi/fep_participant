/**
*
* Bus Compat Stimuli: Bus Check for Mute Signal Command
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
#include "bus_check_mute_signal_command.h"
#include "messages/fep_command_mute_signal.h"
#include "module_client.h"

cBusCheckMuteSignalCommand::cBusCheckMuteSignalCommand(std::string strSigName, fep::tSignalDirection eDirection, bool bMuteStatus):
    m_bMuteStatus(bMuteStatus),
    m_strSignalName(strSigName),
    m_eDirection(eDirection),
    m_bMuteStatusReceived(false),
    m_eDirectionReceived(fep::SD_Undefined)
{
}

cBusCheckMuteSignalCommand::~cBusCheckMuteSignalCommand()
{
}

fep::Result cBusCheckMuteSignalCommand::Update(fep::IMuteSignalCommand const * poCommand)
{
    m_strSignalNameReceived = poCommand->GetSignalName();
    m_eDirectionReceived = poCommand->GetSignalDirection();
    m_bMuteStatusReceived = poCommand->GetMutingStatus();
    m_semUpdate.notify();
    return fep::ERR_NOERROR;
}

fep::Result cBusCheckMuteSignalCommand::Update(fep::IResultCodeNotification const * poCommand)
{

    return fep::ERR_NOERROR;
}

fep::Result cBusCheckMuteSignalCommand::DoSend()
{

    timestamp_t ti = GetClientModule()->GetTimingInterface()->GetTime();
    fep::cMuteSignalCommand oCmd(m_strSignalName.c_str(), m_eDirection, m_bMuteStatus,
        GetClientModule()->GetName(),                     // Sender
        GetClientModule()->GetServerElementName(),        // Receiver
        ti,
        ti + 1
    );

    fep::Result nResult = GetClientModule()->GetCommandAccess()->TransmitCommand(&oCmd);
    return nResult;
}

fep::Result cBusCheckMuteSignalCommand::DoReceive()
{
    fep::Result nResult = fep::ERR_NOERROR;
    if (!m_semUpdate.wait_for(a_util::chrono::milliseconds(10*10*1000)))
    {
        nResult = fep::ERR_TIMEOUT;
        m_result_log << "** Fatal Error: MuteSignalCommand - No Update Received." << std::endl;
    }

    if (0 != m_strSignalName.compare(m_strSignalNameReceived))
    {
        m_result_log << "** MuteSignalCommand: Signal name mismatch." << std::endl;
        nResult = fep::ERR_FAILED;
    }
    if (m_eDirection != m_eDirectionReceived)
    {
        m_result_log << "** MuteSignalCommand: Signal direction mismatch." << std::endl;
        nResult = fep::ERR_FAILED;
    }
    if (m_bMuteStatus != m_bMuteStatusReceived)
    {
        m_result_log << "** MuteSignalCommand: Muting flag mismatch." << std::endl;
        nResult = fep::ERR_FAILED;
    }
    return nResult;
}
