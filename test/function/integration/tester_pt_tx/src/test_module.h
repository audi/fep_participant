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

/// Test module that self-drives itself to the running state and provides
/// access to the command access and property tree.
class cTestModule : public fep::cModule
{
    friend class cTesterCommandInterpretation;
private:
    a_util::concurrency::semaphore m_oEvent;

public:
    fep::Result WaitForRunning(timestamp_t nTimeout)
    {
        if (!m_oEvent.wait_for(a_util::chrono::microseconds(nTimeout)))
        {
            return ERR_TIMEOUT;
        }
        return ERR_NOERROR;
    }

    fep::Result ProcessInitializingEntry(const fep::tState eOldState)
    {
        RETURN_IF_FAILED(fep::cModule::ProcessInitializingEntry(eOldState));
        GetStateMachine()->InitDoneEvent();
        return ERR_NOERROR;
    }

    fep::Result ProcessIdleEntry(const fep::tState eOldState)
    {
        RETURN_IF_FAILED(fep::cModule::ProcessInitializingEntry(eOldState));
        if (fep::FS_STARTUP != eOldState)
        {
            GetStateMachine()->ShutdownEvent();
        }
        else
        {
            GetStateMachine()->InitializeEvent();
        }
        return ERR_NOERROR;
    }

    fep::Result ProcessReadyEntry(const fep::tState eOldState)
    {
        RETURN_IF_FAILED(fep::cModule::ProcessReadyEntry(eOldState));
        GetStateMachine()->StartEvent();
        return ERR_NOERROR;
    }

    fep::Result ProcessStartupEntry(const fep::tState eOldState)
    {
        RETURN_IF_FAILED(fep::cModule::ProcessStartupEntry(eOldState));
        GetStateMachine()->StartupDoneEvent();
        return ERR_NOERROR;
    }

    fep::Result ProcessRunningEntry(const fep::tState eOldState)
    {
        RETURN_IF_FAILED(fep::cModule::ProcessRunningEntry(eOldState));
        m_oEvent.notify();
        return ERR_NOERROR;
    }

    IPropertyTree * GetPropTree()
    {
        return cModule::GetPropertyTree();
    }

    fep::ICommandAccess * GetCmdAccess()
    {
        return cModule::GetCommandAccess();
    }
};