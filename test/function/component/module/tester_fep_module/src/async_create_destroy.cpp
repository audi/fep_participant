/**
 * Implementation of the tester for the FEP Module
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
 * Test Case:   TestAsyncCreateDestroy
 * Test ID:     1.4
 * Test Title:  Test the async creation and destruction
 * Description: This test verifies, that Destroy can be successfully called while
 *              Create is running and vice versa
 * Strategy:    Call both Create and Destroy async
 * Passed If:   Destroy can be called while Create is called and vice versa
 * Ticket:      
 * Requirement: FEPSDK-1542
 */

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;

#include "transmission_adapter/fep_transmission.h"

// This is the same adapter as the original one, but it will block its destruction
// and therefore the Destroy() call of the Module this class is attached to.
class cBlockingDDSAdapter : public fep::cTransmissionAdapter
{
public:
    volatile bool m_bPleaseBlock; 

    cBlockingDDSAdapter() : cTransmissionAdapter(), m_bPleaseBlock(false)
    { }

    // This is currently (see commit date) a call that is made in the cModule::Destroy() method. If
    // this class wont block anymore and therefore the test fails, use your brain and find some
    // other call that you can block so that Destroy() will be blocked
    fep::Result UnregisterCommandListener(ICommandListener* const poCommandListener)
    {
        while (m_bPleaseBlock)
        {
            a_util::system::sleepMilliseconds(10);
        }
        return cTransmissionAdapter::UnregisterCommandListener(poCommandListener);
    }

    fep::Result GetRecentSample(handle_t hSignalHandle, fep::IPreparationDataSample* pSample) const
    {
        return ERR_NOERROR;
    }

    fep::Result RegisterDataListener(fep::IPreparationDataListener* poDataListener,
        handle_t hSignalHandle)
    {
        return ERR_NOERROR;
    }

    fep::Result RegisterSignal(const char* strSignalName, const char* strSignalType,
                           const char* strSignalDescription, size_t szSignal,
                           fep::tSignalDirection eDirection, handle_t& hSignalHandle,
                           fep::tSignalSerialization eSerialization)
    {
        return ERR_NOERROR;
    }

    fep::Result TransmitData(fep::IPreparationDataSample* poPreparationSample)
    {
        return ERR_NOERROR;
    }

    fep::Result UnregisterDataListener(fep::IPreparationDataListener* poDataListener,
        const handle_t hSignalHandle)
    {
        return ERR_NOERROR;
    }

    fep::Result UnregisterSignal(handle_t hSignalHandle)
    {
        return ERR_NOERROR;
    }

    fep::Result TransmitCommand(
        fep::ICommand *)
    {
        return ERR_NOERROR;
    }

    fep::Result TransmitNotification(
        const fep::INotification *)
    {
        return ERR_NOERROR;
    }
    size_t GetMaxTransmitSize()
    {
        return 0;
    }
};

// This class can asynchronously call Create and Destroy
class cCreateDestroyTester : public cModule
{
public:

    bool m_bPleaseBlock;
    volatile bool m_bIsBlocking;
    cBlockingDDSAdapter* m_pAdapter;

    cCreateDestroyTester() : cModule(), m_bPleaseBlock(false),
        m_bIsBlocking(false), m_pAdapter(NULL)
    {
        m_pAdapter = new cBlockingDDSAdapter();
    }

    ~cCreateDestroyTester()
    {
        if (NULL != m_pAdapter)
        {
            delete m_pAdapter;
        }
    }

    fep::Result ProcessStartupEntry(const fep::tState eOldState)
    {
        while (m_bPleaseBlock)
        {
            m_bIsBlocking = true;
            a_util::system::sleepMilliseconds(10);
        }
        m_bIsBlocking = false;
        return ERR_NOERROR;
    }

    fep::Result CheckExistence()
    {
        fep::Result nResult = ERR_NOERROR;
        if (GetCommandAccess() == NULL || GetStateMachine() == NULL)
        {
            nResult = ERR_FAILED;
        }

        // if those two survived, the rest should be fine
        return nResult;
    }
};

class cCreateDestroyThread
{
public:
    fep::Result m_nCreateResult;
    fep::Result m_nDestroyResult;
    bool m_bDoCreate;
    bool m_bDoDestroy;
    cCreateDestroyTester* m_pModule;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;

public:
    cCreateDestroyThread() : m_nCreateResult(ERR_NOERROR), m_nDestroyResult(ERR_NOERROR),
        m_bDoCreate(false), m_bDoDestroy(false)
    {

    }

    fep::Result Create()
    {
        m_pThread.reset(new a_util::concurrency::thread(&cCreateDestroyThread::ThreadFunc, this));
        return ERR_NOERROR;
    }

    void Join()
    {
        m_pThread->join();
    }

public:
    // This will call asynchronously either Create or Destroy
    void ThreadFunc()
    {
        if (m_bDoCreate)
        {
            m_nCreateResult = ERR_NOERROR;
            m_nCreateResult = m_pModule->Create("TestModule");
            m_bDoCreate = false;
        }
        if (m_bDoDestroy)
        {
            m_nDestroyResult = ERR_NOERROR;
            m_nDestroyResult = m_pModule->Destroy();
            m_bDoDestroy = false;
        }
    }
};

// This class will asynchronously destroy a module
class cMurderer
{
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;
public:
    cCreateDestroyTester* m_pModule;
    void ThreadFunc()
    {
        if (NULL != m_pModule)
        {
            delete m_pModule;
            m_pModule = NULL;
        }
    }

    fep::Result Create()
    {
        m_pThread.reset(new a_util::concurrency::thread(&cMurderer::ThreadFunc, this));
        return ERR_NOERROR;
    }

    void Join()
    {
        m_pThread->join();
    }
};

#define WAIT_UNTIL_ITS_TRUE(_expr) \
    tmRestSleepTime = tmMaxSleepTime; \
    while (!_expr && 0 != tmRestSleepTime) \
    { \
        a_util::system::sleepMilliseconds(static_cast<uint32_t>(tmSleepTime / 1000)); \
        tmRestSleepTime =- tmSleepTime; \
    } \
    ASSERT_TRUE(_expr)

/**
 * @req_id "FEPSDK-1542"
 */
TEST(TesterFepModule, TestAsyncCreateDestroy)
{
    // When debugging, set a higher sleep time number, otherwise this test will act strange
    timestamp_t tmMaxSleepTime = static_cast<timestamp_t>(50e6);
    timestamp_t tmRestSleepTime = 0;
    timestamp_t tmSleepTime = static_cast<timestamp_t>(100000);
    {
        // Check Async Create / Destroy
        cCreateDestroyTester oCDTester;
        cCreateDestroyThread oThreadCreate;
        cCreateDestroyThread oThreadDestroy;
        // Now first call Create
        oThreadCreate.m_bDoCreate = true;
        oThreadCreate.m_bDoDestroy = false;
        // ...and block it
        oCDTester.m_bPleaseBlock = true;
        oThreadCreate.m_pModule = &oCDTester;
        oThreadDestroy.m_pModule = &oCDTester;
        oThreadCreate.Create();
        // Give it some time to create and block, then check if its really blocking
        WAIT_UNTIL_ITS_TRUE(oCDTester.m_bIsBlocking);
        // Okay, now lets call Destroy. This must fail.
        oThreadDestroy.m_bDoCreate = false;
        oThreadDestroy.m_bDoDestroy = true;
        oThreadDestroy.Create();
        // Give it some time to not Destroy and fail, then check if the module really is not
        // destroyed and still alive and kicking
        WAIT_UNTIL_ITS_TRUE((ERR_NOT_INITIALISED == oThreadDestroy.m_nDestroyResult));
        oThreadDestroy.Join();
        // Lets wrap it up and finish Creation, see if everything is still okay.
        oCDTester.m_bPleaseBlock = false;
        oThreadCreate.Join();
        ASSERT_TRUE(!oCDTester.m_bIsBlocking);
        ASSERT_TRUE(ERR_NOERROR == oThreadCreate.m_nCreateResult);
        ASSERT_TRUE(ERR_NOERROR == oCDTester.CheckExistence());
        // Lets Destroy, but also block this call
        oCDTester.m_pAdapter->m_bPleaseBlock = true;
        oThreadDestroy.m_bDoCreate = false;
        oThreadDestroy.m_bDoDestroy = true;
        oThreadDestroy.Create();
        a_util::system::sleepMilliseconds(1000);
        // Lets Create, see if that fails
        oThreadCreate.m_bDoCreate = true;
        oThreadCreate.m_bDoDestroy = false;
        oThreadCreate.Create();
        WAIT_UNTIL_ITS_TRUE((ERR_RESOURCE_IN_USE == oThreadCreate.m_nCreateResult));
        oThreadCreate.Join();
        // Finish Destroy.
        oCDTester.m_pAdapter->m_bPleaseBlock = false;
        oThreadDestroy.Join();
        ASSERT_TRUE(ERR_NOERROR == oThreadDestroy.m_nDestroyResult);
        // Done.
    }
    /*{
        // Now we will test, if the DTOR will block until Create or Destroy is done.
        cCreateDestroyThread oThreadMain;
        cCreateDestroyTester * pCreateTester = new cCreateDestroyTester();
        oThreadMain.m_bDoCreate = true;
        oThreadMain.m_bDoDestroy = false;
        oThreadMain.m_pModule = pCreateTester;
        pCreateTester->m_bPleaseBlock = true;
        // First block Create
        oThreadMain.Create();
        WAIT_UNTIL_ITS_TRUE(pCreateTester->m_bIsBlocking);
        
        // Now try to delete the module, but first secure the cBlockingDDSAdapter so
        // that it wont be deleted in the DTOR of cCreateDestroyTester
        // (that would be before the DTOR of cModule)
        cBlockingDDSAdapter* pSecuredAdapter = pCreateTester->m_pAdapter;
        pCreateTester->m_pAdapter = NULL;
        cMurderer oKiller;
        oKiller.m_pModule = pCreateTester;
        oKiller.Create();
        // Is it deleted?
        WAIT_UNTIL_ITS_TRUE((NULL != oKiller.m_pModule));
        // Now unblock and see if its deleted
        pCreateTester->m_bPleaseBlock = false;
        WAIT_UNTIL_ITS_TRUE((NULL == oKiller.m_pModule));
        oThreadMain.Join();
        oKiller.Join();
        // Clean up previously secured adapter
        delete pSecuredAdapter;

        // Same now for Destroy
        cCreateDestroyTester * pDestroyTester = new cCreateDestroyTester();
        // First Create the adapter, so that it will be destroyable
        oThreadMain.m_bDoCreate = true;
        oThreadMain.m_bDoDestroy = false;
        oThreadMain.m_pModule = pDestroyTester;
        oThreadMain.Create();
        WAIT_UNTIL_ITS_TRUE(!oThreadMain.m_bDoCreate);
        oThreadMain.Join();
        // And now call Destroy and block it
        oThreadMain.m_bDoCreate = false;
        oThreadMain.m_bDoDestroy = true;
        pDestroyTester->m_pAdapter->m_bPleaseBlock = true;
        oThreadMain.Create();
        a_util::system::sleepMilliseconds(1000);
        // Now try to delete the module, secure adapter first (see above)
        pSecuredAdapter = pDestroyTester->m_pAdapter;
        pDestroyTester->m_pAdapter = NULL;
        oKiller.m_pModule = pDestroyTester;
        oKiller.Create();
        // Is it deleted?
        WAIT_UNTIL_ITS_TRUE((NULL != oKiller.m_pModule));
        // Now unblock and see if its deleted
        pSecuredAdapter->m_bPleaseBlock = false;
        WAIT_UNTIL_ITS_TRUE((NULL == oKiller.m_pModule));
        oKiller.Join();
        oThreadMain.Join();
        delete pSecuredAdapter;
    }*/
}
