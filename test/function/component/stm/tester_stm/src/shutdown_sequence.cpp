/**
* Implementation of the tester for the FEP State Machine (shutdown sequence)
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

/*
* Test Case:   TestShutdownSequence
* Test ID:     1.11
* Test Title:  Test Shutdown Sequence
* Description: Tests if the STM performs a full shutdown sequence to the shutdown state during destruction
* Strategy:    Starting from all possible states, check if the correct shutdown sequence is performed
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEPSDK-1423
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>

#include "statemachine/fep_statemachine.h"

using namespace fep;

#include "tester_fep_stm.h"

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

class cShutdownSequenceTestListener : public IStateEntryListener
{
public:
    //we need to use the begin state becuse the WaitForState within the tests will return immediatelly
    //after the state has been reached, but the notifier is called AFTER the State was already set
    explicit cShutdownSequenceTestListener(tState begin_state) :
      m_strSequenceTaken(), m_begin_state(begin_state){ }

      void AddToSequence(const char * strState)
      {
          std::lock_guard<std::mutex> lock(_lock_string); 
          if (!m_strSequenceTaken.empty())
          {
              m_strSequenceTaken.push_back('-');
          }

          m_strSequenceTaken.append(strState);
      }
      std::string GetSequence()
      {
          std::lock_guard<std::mutex> lock(_lock_string);
          return m_strSequenceTaken;
      }

public: // overrides IStateEntryListener
    fep::Result ProcessStartupEntry(const tState eOldState)
    {
        if (m_begin_state != FS_STARTUP)
        {
            AddToSequence("Startup");
        }
        return ERR_NOERROR;
    }

    fep::Result ProcessIdleEntry(const tState eOldState)
    {
        if (m_begin_state != FS_IDLE)
        {
            AddToSequence("Idle");
        }
        return ERR_NOERROR;
    }

    fep::Result ProcessInitializingEntry(const tState eOldState)
    {
        if (m_begin_state != FS_INITIALIZING)
        {
            AddToSequence("Initializing");
        }
        return ERR_NOERROR;
    }

    fep::Result ProcessReadyEntry(const tState eOldState)
    {
        if (m_begin_state != FS_READY)
        {
            AddToSequence("Ready");
        }
        return ERR_NOERROR;
    }

    fep::Result ProcessRunningEntry(const tState eOldState)
    {
        if (m_begin_state != FS_RUNNING)
        {
            AddToSequence("Running");
        }
        return ERR_NOERROR;
    }

    fep::Result ProcessErrorEntry(const tState eOldState)
    {
        if (m_begin_state != FS_ERROR)
        {
            AddToSequence("Error");
        }
        return ERR_NOERROR;
    }

    fep::Result ProcessShutdownEntry(const tState eOldState)
    {
        if (m_begin_state != FS_SHUTDOWN)
        {
            AddToSequence("Shutdown");
        }
        return ERR_NOERROR;
    }

    fep::Result CleanUp(const fep::tState eOldState)
    {
        AddToSequence("Cleanup");
        return ERR_NOERROR;
    }

private:
    std::mutex  _lock_string;
    std::string m_strSequenceTaken;
    tState      m_begin_state;
};


/**
 * @req_id "FEPSDK-1423"
 */
TEST_F(cTesterStatemachine, TestShutdownSequenceStartup)
{
    static const timestamp_t tmWaitTime = -1;
    // Listener
    cShutdownSequenceTestListener oShutdownSequenceTestListener(FS_STARTUP);

    oSTM.FireupStateMachine();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_STARTUP, tmWaitTime));

    oSTM.RegisterStateEntryListener(&oShutdownSequenceTestListener);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
    EXPECT_EQ(oShutdownSequenceTestListener.GetSequence(), "Shutdown");
    EXPECT_EQ(oSTM.GetState(), FS_SHUTDOWN);
    oSTM.UnregisterStateEntryListener(&oShutdownSequenceTestListener);
}

/**
 * @req_id "FEPSDK-1423"
 */
TEST_F(cTesterStatemachine, TestShutdownSequenceIdle)
{
    static const timestamp_t tmWaitTime = -1;
    // Listener
    cShutdownSequenceTestListener oShutdownSequenceTestListener(FS_IDLE);

    oSTM.FireupStateMachine();
    ASSERT_TRUE(isOk(GoToState(FS_IDLE)));

    oSTM.RegisterStateEntryListener(&oShutdownSequenceTestListener);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
    EXPECT_EQ(oShutdownSequenceTestListener.GetSequence(), "Cleanup-Shutdown");
    EXPECT_EQ(oSTM.GetState(), FS_SHUTDOWN);
    oSTM.UnregisterStateEntryListener(&oShutdownSequenceTestListener);
}

/**
 * @req_id "FEPSDK-1423"
 */
TEST_F(cTesterStatemachine, TestShutdownSequenceInitializing)
{
    static const timestamp_t tmWaitTime = -1;
    // Listener
    cShutdownSequenceTestListener oShutdownSequenceTestListener(FS_INITIALIZING);

    oSTM.FireupStateMachine();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_STARTUP, tmWaitTime));
    ASSERT_TRUE(isOk(GoToState(FS_INITIALIZING)));

    oSTM.RegisterStateEntryListener(&oShutdownSequenceTestListener);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
    EXPECT_EQ(oSTM.GetState(), FS_SHUTDOWN);
    EXPECT_EQ(oShutdownSequenceTestListener.GetSequence(), "Idle-Cleanup-Shutdown");
    oSTM.UnregisterStateEntryListener(&oShutdownSequenceTestListener);
}

/**
 * @req_id "FEPSDK-1423"
 */
TEST_F(cTesterStatemachine, TestShutdownSequenceReady)
{
    static const timestamp_t tmWaitTime = -1;
    // Listener
    cShutdownSequenceTestListener oShutdownSequenceTestListener(FS_READY);

    oSTM.FireupStateMachine();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_STARTUP, tmWaitTime));
    ASSERT_TRUE(isOk(GoToState(FS_READY)));

    oSTM.RegisterStateEntryListener(&oShutdownSequenceTestListener);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
    EXPECT_EQ(oSTM.GetState(), FS_SHUTDOWN);
    EXPECT_EQ(oShutdownSequenceTestListener.GetSequence(), "Idle-Cleanup-Shutdown");
    oSTM.UnregisterStateEntryListener(&oShutdownSequenceTestListener);
}

/**
 * @req_id "FEPSDK-1423"
 */
TEST_F(cTesterStatemachine, TestShutdownSequenceRunning)
{
    static const timestamp_t tmWaitTime = -1;
    // Listener
    cShutdownSequenceTestListener oShutdownSequenceTestListener(FS_RUNNING);

    oSTM.FireupStateMachine();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_STARTUP, tmWaitTime));
    ASSERT_TRUE(isOk(GoToState(FS_RUNNING)));

    oSTM.RegisterStateEntryListener(&oShutdownSequenceTestListener);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
    EXPECT_EQ(oShutdownSequenceTestListener.GetSequence(), "Idle-Cleanup-Shutdown");
    EXPECT_EQ(oSTM.GetState(), FS_SHUTDOWN);
    oSTM.UnregisterStateEntryListener(&oShutdownSequenceTestListener);
}

/**
 * @req_id "FEPSDK-1423"
 */
TEST_F(cTesterStatemachine, TestShutdownSequenceError)
{
    static const timestamp_t tmWaitTime = -1;
    // Listener
    cShutdownSequenceTestListener oShutdownSequenceTestListener(FS_ERROR);

    oSTM.FireupStateMachine();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_STARTUP, tmWaitTime));
    ASSERT_TRUE(isOk(GoToState(FS_ERROR)));

    oSTM.RegisterStateEntryListener(&oShutdownSequenceTestListener);
    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
    EXPECT_EQ(oShutdownSequenceTestListener.GetSequence(), "Cleanup-Shutdown");
    EXPECT_EQ(oSTM.GetState(), FS_SHUTDOWN);
    oSTM.UnregisterStateEntryListener(&oShutdownSequenceTestListener);
}

/**
 * @req_id "FEPSDK-1423"
 */
TEST_F(cTesterStatemachine, TestShutdownSequenceShutdown)
{
    static const timestamp_t tmWaitTime = 100;

    oSTM.FireupStateMachine();
    ASSERT_EQ(a_util::result::SUCCESS, WaitForState(&oSTM, FS_STARTUP, tmWaitTime));
    ASSERT_TRUE(isOk(GoToState(FS_SHUTDOWN)));

    ASSERT_EQ(a_util::result::SUCCESS, oSTM.PerformShutdown());
    EXPECT_EQ(oSTM.GetState(), FS_SHUTDOWN);
}