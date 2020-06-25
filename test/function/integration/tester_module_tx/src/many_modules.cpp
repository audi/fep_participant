/**
 * Implementation of the tester for the integration of FEP Module with FEP Transmission Adapter
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
* Test Case:   TestManyModules
* Test ID:     1.3
* Test Title:  Test many modules in one process
* Description: This test tests the amount of modules that can be used in one process
* Strategy:   Try to create many modules and make everybody communicate with everybody
*              
* Passed If:   End of test reached.
*              
* Ticket:      -
* Requirement: FEPSDK-1574 FEPSDK-1575
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include <fep_test_common.h>
using namespace fep;
using namespace fep::component_config;

/**
 * This module will be receiving signals s_1 to s_n, while n being the amount of modules under test.
 * The module will be sending signal s_x while x being the index of this module.
 */
class cChattyModule : public cTestBaseModule, IUserDataListener
{
public:
    cChattyModule() : cTestBaseModule(), m_hOut(NULL)
    { }

    ~cChattyModule()
    {
        Destroy();
    }

    /**
     * The method \ref Create initializes the module.
     * 
     * @param [in] ui32TargetSignalCount  How man signals should be registered
     * @param [in] ui32TargetSampleCount  How many samples should be send and received
     * @param [in] ui32ModuleIdx  The index of this module
     * @param [in] tmCycleTime  The cycle time of sending samples
     * @returns  Standard result code.
     * @retval ERR_NOERROR  Everything went fine
     */
    fep::Result Create(uint32_t ui32TargetSignalCount, uint32_t ui32TargetSampleCount, uint32_t ui32ModuleIdx,
        timestamp_t tmCycleTime)
    {
        m_ui32TargetSampleCount = ui32TargetSampleCount;
        m_tmCycleTime = tmCycleTime;
        m_ui32ModuleIdx = ui32ModuleIdx;
        m_ui32TargetSignalCount = ui32TargetSignalCount;
        m_ui32AllSignalsReceived = 0;
        m_ui32SamplesSend = 0;
        return cTestBaseModule::Create(cModuleOptions(fep::TT_RTI_DDS,
            a_util::strings::format("module%d", ui32ModuleIdx).c_str()));
    }

    fep::Result ProcessStartupEntry(const fep::tState eOldState)
    {
        fep::Result nResult = cTestBaseModule::ProcessStartupEntry(eOldState);
        if(fep::isOk(nResult))
        {
            nResult |= GetPropertyTree()->SetPropertyValue(
                g_strTxAdapterPath_nNumberOfWorkerThreads, static_cast<int32_t>(1));
        }
        if(fep::isOk(nResult))
        {
            GetStateMachine()->StartupDoneEvent();
        }
        FillModuleHeader();
        return nResult;
    }

    fep::Result ProcessIdleEntry(const fep::tState eOldState)
    {
        fep::Result nResult = cTestBaseModule::ProcessIdleEntry(eOldState);

        // Do some cleanup
        m_oTimer.stop();
        // Not checking return value, in case we were not registered (lazy).
        if (NULL != m_hOut)
        {
            nResult = GetSignalRegistry()->UnregisterSignal(m_hOut);
            m_hOut = NULL;
        }
        for (std::list<handle_t>::iterator pIter =  m_lstHandlesIn.begin();
            m_lstHandlesIn.end() != pIter; pIter++)
        {
            if (fep::isOk(nResult))
            {
                nResult = GetUserDataAccess()->UnregisterDataListener(this, *pIter);
            }
            if (fep::isOk(nResult))
            {
                nResult = GetSignalRegistry()->UnregisterSignal(*pIter);
            }
        }
        return nResult;
    }

    fep::Result ProcessInitializingEntry(const fep::tState eOldState)
    {
        fep::Result nResult = cTestBaseModule::ProcessInitializingEntry(eOldState);
        // template for the signal descriptions
        std::string strTemplateDescription =
            "<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>"
            "<adtf:ddl xmlns:adtf=\"adtf\">"
            "    <header>"
            "        <language_version>3.00</language_version>"
            "        <author>AUDI AG</author>"
            "        <date_creation>07#.04.2010</date_creation>"
            "        <date_change>07.04.2010</date_change>"
            "        <description>ADTF Common Description File</description>"
            "    </header>"
            "    <units>"
            "    </units>"
            "    <datatypes>"
            "        <datatype name=\"tBool\" size=\"8\" />"
            "        <datatype name=\"tChar\" size=\"8\" />"
            "        <datatype name=\"tUInt8\" size=\"8\" />"
            "        <datatype name=\"tInt8\" size=\"8\" />"
            "        <datatype name=\"tUInt16\" size=\"16\" />"
            "        <datatype name=\"tInt16\" size=\"16\" />"
            "        <datatype name=\"tUInt32\" size=\"32\" />"
            "        <datatype name=\"tInt32\" size=\"32\" />"
            "        <datatype name=\"tUInt64\" size=\"64\" />"
            "        <datatype name=\"tInt64\" size=\"64\" />"
            "        <datatype name=\"tFloat32\" size=\"32\" />"
            "        <datatype name=\"tFloat64\" size=\"64\" />"
            "    </datatypes>"
            "    <enums>"
            "    </enums>"
            "    <structs>"
            "      %s"
            "    </structs>"
            "    <streams>"
            "    </streams>"
            "</adtf:ddl>"
            ;
        // template for the signal names
        std::string strTemplateSignalName = "signal%d";
        //template for the signalt ypes
        std::string strTemplateSignalType = "tsignal%d";
        std::string strTemplateSignalStruct =
            "      <struct alignment=\"1\" name=\"tsignal%d\" version=\"1\">" \
            "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\""
            "                 name=\"s\" type=\"tFloat64\" />"
            "      </struct>";

        std::string strFullDescription;

        // Create full description
        for (uint32_t ui32Idx = 0; ui32Idx < m_ui32TargetSignalCount; ui32Idx++)
        {
            strFullDescription += a_util::strings::format(strTemplateSignalStruct.c_str(), ui32Idx);
        }
        strFullDescription = a_util::strings::format(strTemplateDescription.c_str(),
            strFullDescription.c_str());

        nResult |= GetSignalRegistry()->RegisterSignalDescription(strFullDescription.c_str(),
            fep::ISignalRegistry::DF_REPLACE);

        // Register the other module's signals and store their handle
        for (uint32_t ui32Idx = 0; ui32Idx < m_ui32TargetSignalCount; ui32Idx++)
        {
            std::string strSignalName = a_util::strings::format(strTemplateSignalName.c_str(), ui32Idx);
            std::string strSignalType = a_util::strings::format(strTemplateSignalType.c_str(), ui32Idx);
            handle_t hHandle = NULL;
            if (fep::isOk(nResult))
            {
                cUserSignalOptions oSigOptions(strSignalName.c_str(), SD_Input, strSignalType.c_str());
                nResult = GetSignalRegistry()->RegisterSignal(oSigOptions, hHandle);
                m_lstHandlesIn.push_back(hHandle);
                GTEST_PRINTF(
                    a_util::strings::format("Module %d: Reg %d", m_ui32ModuleIdx, ui32Idx).c_str());
            }
            if (fep::isOk(nResult))
            {
                nResult = GetUserDataAccess()->RegisterDataListener(this, hHandle);
            }
        }
        // Register this module's send signal
        std::string strSignalName = a_util::strings::format(strTemplateSignalName.c_str(), m_ui32ModuleIdx);
        std::string strSignalType = a_util::strings::format(strTemplateSignalType.c_str(), m_ui32ModuleIdx);
        if (fep::isOk(nResult))
        {
            cUserSignalOptions oSigOptions(strSignalName.c_str(), SD_Output, strSignalType.c_str());
            nResult = GetSignalRegistry()->RegisterSignal(oSigOptions, m_hOut);
        }

        // Register cyclic
        if (fep::isOk(nResult))
        {
            m_oTimer.setCallback(&cChattyModule::RunCyclic, *this);
            m_oTimer.setPeriod(m_tmCycleTime);
        }
        if (fep::isOk(nResult))
        {
            GetStateMachine()->InitDoneEvent();
        }
        return nResult;
    }

    fep::Result ProcessRunningEntry(const fep::tState eOldState)
    {
        m_oTimer.start();
        return fep::ERR_NOERROR;
    }

    fep::Result Update(const IUserDataSample* poSample)
    {
        // Increase counter
        if (m_mapReceiverCounts.find(poSample->GetSignalHandle()) == m_mapReceiverCounts.end())
        {
            m_mapReceiverCounts[poSample->GetSignalHandle()] = 1;
        }
        else
        {
            m_mapReceiverCounts[poSample->GetSignalHandle()]++;
        }
        m_ui32AllSignalsReceived++;
        return ERR_NOERROR;
    }

    void RunCyclic()
    {
        fep::Result nResult;
        // Create a sample on the fly, no performance needed
        IUserDataSample * poSendSample = NULL;
        if (fep::isOk(nResult))
        {
            nResult = GetUserDataAccess()->CreateUserDataSample(poSendSample);
        }
        double f64Value = 0;
        if (fep::isOk(nResult))
        {
            poSendSample->Attach(static_cast<void*>(&f64Value), sizeof(f64Value));
            poSendSample->SetSignalHandle(m_hOut);
        }
        // Send the sample, if not already all samples have been sent.
        if (fep::isOk(nResult) && m_ui32SamplesSend < m_ui32TargetSampleCount)
        {
            nResult = GetUserDataAccess()->TransmitData(poSendSample, true);
            m_ui32SamplesSend++;
        }
        if (NULL != poSendSample)
        {
            delete poSendSample;
        }
    }

    /**
     * The method \ref AnalyzeResult checks if all signals and all samples have been received.
     * Warning: This modifies the results.
     * 
     * @returns  Standard result code.
     * @retval ERR_NOERROR  Everything went fine
     * @retval ERR_FAILED   Something did not go fine
     */
    fep::Result AnalyzeResult()
    {
        fep::Result nResult = ERR_NOERROR;
        std::list<handle_t> lstReceivedHandles;
        for (std::map<handle_t, uint32_t>::iterator pIter = m_mapReceiverCounts.begin();
            m_mapReceiverCounts.end() != pIter; pIter++)
        {
            lstReceivedHandles.push_back(pIter->first);
            if (pIter->second != m_ui32TargetSampleCount)
            {
                nResult = ERR_FAILED;
                LOG_INFO("Not all samples have been received.");
            }
        }
        lstReceivedHandles.sort();
        m_lstHandlesIn.sort();
        if (m_lstHandlesIn != lstReceivedHandles)
        {
            nResult = ERR_FAILED;
            LOG_INFO("Not all signals have been received");
        }
        m_lstHandlesIn.clear();
        m_mapReceiverCounts.clear();
        return nResult;
    }

    /**
     * The method \ref IsDone checks if all samples have been sent and all samples probably have
     * been received (lazy version of AnalyzeResult() without modifiing the results).
     * 
     * @returns  True, if the module is done, false otherwise.
     */
    bool IsDone()
    {
        return m_ui32TargetSampleCount * m_ui32TargetSignalCount == m_ui32AllSignalsReceived
            && m_ui32SamplesSend == m_ui32TargetSampleCount;
    }
    
private:
    /// all handles of subscribed signals.
    std::list<handle_t> m_lstHandlesIn;
    /// the handle of the published signal
    handle_t m_hOut;
    /// the amount signals to be registered
    uint32_t m_ui32TargetSignalCount;
    /// the index of this module
    uint32_t m_ui32ModuleIdx;
    /// the cycle time for sending samples
    timestamp_t m_tmCycleTime;
    /// the amount of samples to send
    uint32_t m_ui32TargetSampleCount;
    /// the amount of samples sent
    uint32_t m_ui32SamplesSend;
    /// map of handles and counts of received samples
    std::map<handle_t, uint32_t> m_mapReceiverCounts;
    /// amount of all received signals
    uint32_t m_ui32AllSignalsReceived;
    // timer
    a_util::system::Timer m_oTimer;
};

/**
 * @req_id "FEPSDK-1574 FEPSDK-1575"
 */
TEST(cTesterModuleTransmissionAdapter, TestManyModules)
{
    // Test parameters
    // - amount of modules to spawn
    uint32_t ui32AmountOfModules = 5;

    // - cycle time for sending samples
    timestamp_t tmCycleTime = 1000000;
    // - amount of samples to send
    uint32_t ui32TargetSampleCount = 10;
    // - timeout for waiting until sending should be complete

    std::list<cChattyModule*> lstModules;
    std::vector<std::string> vecParticipants;

    const timestamp_t tTimeOut = 2000;

    // Create modules
    for (uint32_t ui32Idx = 0 ; ui32Idx < ui32AmountOfModules; ui32Idx++)
    {
        cChattyModule * pModule = new cChattyModule();
        ASSERT_EQ(a_util::result::SUCCESS, pModule->Create(ui32AmountOfModules, ui32TargetSampleCount, ui32Idx,
            tmCycleTime));
        ASSERT_TRUE(fep::isOk(pModule->WaitForState(FS_IDLE, tTimeOut)));
        lstModules.push_back(pModule);
        vecParticipants.push_back(pModule->GetName());
    }

    // Start modules
    AutomationInterface oAI;
    // wait for AI to be ready
    a_util::system::sleepMilliseconds(5 * 1000);
    ASSERT_EQ(a_util::result::SUCCESS, oAI.WaitForSystemState(FS_IDLE, vecParticipants, tTimeOut*4));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEvent(CE_Initialize, "module*"));

    // This produces very heavy CPU load (10 modules with each 10 signals. therefore we just let the
    // system run for some time before we start interacting again
    a_util::system::sleepMilliseconds(10 * 1000); // Wait 10 seconds
    ASSERT_EQ(a_util::result::SUCCESS, oAI.WaitForSystemState(FS_READY, vecParticipants, tTimeOut));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEvent(CE_Start, "module*"));
    // Wait for all modules to start and finish sending signals
    timestamp_t tmWaitTime = 0;
    bool bIsDone = false;
    timestamp_t tmTimeOut = tmCycleTime * ui32TargetSampleCount * 2;
    while(tmWaitTime < tmTimeOut && !bIsDone)
    {
        bIsDone = true;
        for (std::list<cChattyModule*>::iterator pIter = lstModules.begin();
            lstModules.end() != pIter; pIter++)
        {
            if (!(*pIter)->IsDone())
            {
                bIsDone = false;
                break;
            }
        }
        tmWaitTime += tmCycleTime;
        a_util::system::sleepMilliseconds(static_cast<uint32_t>(tmCycleTime / 1000));
    }
    
    // Check result
    for (std::list<cChattyModule*>::iterator pIter = lstModules.begin();
        lstModules.end() != pIter; pIter++)
    {
        ASSERT_EQ(a_util::result::SUCCESS, (*pIter)->AnalyzeResult());
    }
    
    ASSERT_EQ(a_util::result::SUCCESS, oAI.TriggerEvent(CE_Stop, "module*"));
    ASSERT_EQ(a_util::result::SUCCESS, oAI.WaitForSystemState(FS_IDLE, vecParticipants, tTimeOut));

    // Clean up
    for (std::list<cChattyModule*>::iterator pIter = lstModules.begin();
        lstModules.end() != pIter; pIter++)
    {
        (*pIter)->Destroy();
        delete *pIter;
    }
}
