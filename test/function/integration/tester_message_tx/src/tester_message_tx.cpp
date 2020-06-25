/**
 * Implementation of the tester for the integration of Messages and Transmission
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
/**
* Test Case:   TestRemoteMessageStateChanges
* Test ID:     1.0
* Test Title:  Test if all state changes are done
* Description: Test if state change commands or state notifications got lost
* Strategy:    Initiates 2 fep elements two passive client modules waiting for state change 
*              commands and executiting state changes. A command or controller fep element
*              is sending state change requests and observing state changes by listening
*              to notifications. The controller checks if the client elements have done
*              the required state changes. The test succeeds if all required state changes
*              can be observed.
*              
* Passed If:   no errors occur
*              
* Ticket:      -
* Requirement: FEPSDK-1738
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>


class cControllerModule : public fep::cModule, public fep::cNotificationListener
{
    class cSingleModuleObservation
    {
    public:
        void appendState(const fep::tState nState)
        {
            m_saveStates.push_back(nState);
        }

        void ClearSequence()
        {
            m_saveStates.clear();
        }

        fep::Result CompareSequence(const std::vector<fep::tState>& expected_sequence)
        {
            bool matches= true;
            {
                if (m_saveStates.size() != expected_sequence.size())
                {
                    matches= false;
                }
                else
                {
                    for (std::size_t i= 0; i < m_saveStates.size(); ++i)
                    {
                        if (m_saveStates[i] != expected_sequence[i])
                        {
                            matches= false;
                        }
                    }
                }
            }

            if (matches)
            {
                std::cerr << "OK";
            }
            else
            {
                std::cerr << "FAIL [";

                for (std::size_t i= 0; i < m_saveStates.size(); ++i)
                {
                    if (i > 0)
                    {
                        std::cerr << ",";
                    }
                    std::cerr << fep::cState::ToString(m_saveStates[i]);
                }

                std::cerr << "] != [";

                for (std::size_t i= 0; i < expected_sequence.size(); ++i)
                {
                    if (i > 0)
                    {
                        std::cerr << ",";
                    }
                    std::cerr << fep::cState::ToString(expected_sequence[i]);
                }

                std::cerr << "]";
            }

            if (!matches)
            {
                return ERR_INVALID_STATE;
            }
            
            return ERR_NOERROR;
        }

    public:
        std::vector<fep::tState> m_saveStates;
    };

    class cAllModuleObservation
    {
    public:
        void appendState(const std::string& strModuleName, const fep::tState nState)
        {
            a_util::concurrency::unique_lock<a_util::concurrency::mutex> locker(m_lock);

            std::map<std::string, cSingleModuleObservation*>::iterator it= m_allModules.find(strModuleName);

            if (it == m_allModules.end())
            {
                m_allModules.insert(std::make_pair(strModuleName, new cSingleModuleObservation()));
                it= m_allModules.find(strModuleName);
            }
            it->second->appendState(nState);
        }

        void ClearSequence()
        {
           a_util::concurrency::unique_lock<a_util::concurrency::mutex> locker(m_lock);
        
            for (std::map<std::string, cSingleModuleObservation*>::iterator it= m_allModules.begin(); 
                 it != m_allModules.end(); ++it)
            {
                it->second->ClearSequence();
            }
        }

        fep::Result CompareSequence(const std::vector<fep::tState>& expected_sequence)
        {
            fep::Result nResult= ERR_NOERROR;
            a_util::concurrency::unique_lock<a_util::concurrency::mutex> locker(m_lock);
        
            for (std::map<std::string, cSingleModuleObservation*>::iterator it= m_allModules.begin(); 
                 it != m_allModules.end(); ++it)
            {
                std::cerr << it->first << ": ";
                nResult|= it->second->CompareSequence(expected_sequence);
        
                std::cerr << std::endl;
            }

            return nResult;
        }


    private:
        std::map<std::string, cSingleModuleObservation*> m_allModules;
        a_util::concurrency::mutex m_lock;
    };

public: // override cNotificationListener
	virtual fep::Result Update(fep::IStateNotification const * pStateNotification)
	{
        //std::cerr << pStateNotification->GetSender() << ": " << fep::cState::ToString(pStateNotification->GetState()) << std::endl;
        m_oAllModuleObservations.appendState(pStateNotification->GetSender(), pStateNotification->GetState());
		return ERR_NOERROR;
	}

public: // override cModule/IStateEntryListener
	virtual fep::Result ProcessStartupEntry(const fep::tState eOldState)
	{
		RETURN_IF_FAILED(cModule::ProcessStartupEntry(eOldState));

        fep::Result nResult = ERR_NOERROR;
       
	    nResult |= GetPropertyTree()->SetPropertyValue(
            FEP_STM_STANDALONE_PATH , true);

		nResult = GetNotificationAccess()->RegisterNotificationListener(this);

        GetStateMachine()->StartupDoneEvent();

		return nResult;
	}

public:
    void ClearSequence()
    {
        m_oAllModuleObservations.ClearSequence();
    }
    fep::Result CompareSequence(const std::vector<fep::tState>& expected_sequence)
    {
        return m_oAllModuleObservations.CompareSequence(expected_sequence);
    }

private:
    cAllModuleObservation m_oAllModuleObservations;
};

class cClientModule : public fep::cModule, public fep::cCommandListener
{
public:
	cClientModule()
	{ }

public: // override cCommandListener
	virtual fep::Result Update(fep::IControlCommand const * poCommand)
	{
		return ERR_NOERROR;
	}

public: // override cModule/IStateEntryListener
	virtual fep::Result ProcessStartupEntry(const fep::tState eOldState)
	{
		RETURN_IF_FAILED(cModule::ProcessStartupEntry(eOldState));

		//std::cerr << GetName() << ": " << fep::cState::ToString(fep::FS_STARTUP) << std::endl;

        fep::Result nResult = GetCommandAccess()->RegisterCommandListener(this);
		nResult = (ERR_UNEXPECTED == nResult) ? (ERR_NOERROR) : (nResult);

        GetStateMachine()->StartupDoneEvent();

		return nResult;
	}

	virtual fep::Result ProcessIdleEntry(const fep::tState eOldState)
	{
		RETURN_IF_FAILED(cModule::ProcessIdleEntry(eOldState));

		//std::cerr << GetName() << ": " << fep::cState::ToString(fep::FS_IDLE) << std::endl;

		return ERR_NOERROR;
	}

	virtual fep::Result ProcessInitializingEntry(const fep::tState eOldState)
	{
		RETURN_IF_FAILED(cModule::ProcessInitializingEntry(eOldState));

		//std::cerr << GetName() << ": " << fep::cState::ToString(fep::FS_INITIALIZING) << std::endl;

        GetStateMachine()->InitDoneEvent();

        return ERR_NOERROR;
	}

	virtual fep::Result ProcessReadyEntry(const fep::tState eOldState)
	{
		RETURN_IF_FAILED(cModule::ProcessReadyEntry(eOldState));

		//std::cerr << GetName() << ": " << fep::cState::ToString(fep::FS_READY) << std::endl;

        return ERR_NOERROR;
	}

	virtual fep::Result ProcessRunningEntry(const fep::tState eOldState)
	{
		RETURN_IF_FAILED(cModule::ProcessRunningEntry(eOldState));

		//std::cerr << GetName() << ": " << fep::cState::ToString(fep::FS_RUNNING) << std::endl;

        return ERR_NOERROR;
	}

	virtual fep::Result ProcessShutdownEntry(const fep::tState eOldState)
	{
		RETURN_IF_FAILED(cModule::ProcessShutdownEntry(eOldState));

		//std::cerr << GetName() << ": " << fep::cState::ToString(fep::FS_SHUTDOWN) << std::endl;

        return ERR_NOERROR;
	}

	virtual fep::Result ProcessErrorEntry(const fep::tState eOldState)
	{
		RETURN_IF_FAILED(cModule::ProcessErrorEntry(eOldState));

		//std::cerr << GetName() << ": " << fep::cState::ToString(fep::FS_ERROR) << std::endl;

        return ERR_NOERROR;
	}
};

/// FIXME
/**
 * @req_id "FEPSDK-1738"
 */
TEST(cTesterMessageTx, TestRemoteMessageStateChanges)
{
    static const timestamp_t s_tmSleepTime= 500 * 1000;
    static const size_t s_nLoops= 20;

    cClientModule oClientModuleA;
    cClientModule oClientModuleB;
	cControllerModule oControllerModule;

    // Create modules
	ASSERT_EQ(oClientModuleA.Create("TestStateChanges" "_" "A"), ERR_NOERROR);
	ASSERT_EQ(oClientModuleB.Create("TestStateChanges" "_" "B"), ERR_NOERROR);
	ASSERT_EQ(oControllerModule.Create("TestStateChanges" "_" "CTX"), ERR_NOERROR);

    // Expected sequence 
    std::vector<fep::tState> expected_sequence;
    expected_sequence.push_back(fep::FS_INITIALIZING);
    expected_sequence.push_back(fep::FS_READY);
    expected_sequence.push_back(fep::FS_RUNNING);
    expected_sequence.push_back(fep::FS_IDLE);

    // Initial sleep ... wait for modules to settle
    a_util::system::sleepMicroseconds(s_tmSleepTime);

    // Mark first run ... the first loop is allowed to fail
    bool first_run= true;
    for (size_t i= 0; i< s_nLoops; ++i)
	{
        oControllerModule.ClearSequence();
        ASSERT_EQ(oControllerModule.GetStateMachine()->TriggerRemoteEvent(fep::CE_Initialize, "*"), ERR_NOERROR);
		a_util::system::sleepMicroseconds(s_tmSleepTime);
        ASSERT_EQ(oControllerModule.GetStateMachine()->TriggerRemoteEvent(fep::CE_Start, "*"), ERR_NOERROR);
		a_util::system::sleepMicroseconds(s_tmSleepTime);
        ASSERT_EQ(oControllerModule.GetStateMachine()->TriggerRemoteEvent(fep::CE_Stop, "*"), ERR_NOERROR);
		a_util::system::sleepMicroseconds(s_tmSleepTime);

        if (first_run)
        {
            // Ignore the first run ... it is used to get all elements in sync and into
            // a commmon state. 
            first_run= false;
        }
        else 
        {
            ASSERT_EQ(oControllerModule.CompareSequence(expected_sequence), ERR_NOERROR);
	    }
    }

	ASSERT_EQ(oControllerModule.Destroy(), ERR_NOERROR);
	ASSERT_EQ(oClientModuleA.Destroy(), ERR_NOERROR);
	ASSERT_EQ(oClientModuleB.Destroy(), ERR_NOERROR);
}
