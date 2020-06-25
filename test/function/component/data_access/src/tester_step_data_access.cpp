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
#include <gtest/gtest.h>

#include <fep_participant_sdk.h>

#include "data_access/fep_step_data_access.h"

#include "tester_step_data_access.h"

using namespace fep;
using namespace fep::timing;

InputConfig makeInputConfig(handle_t hHandle, timestamp_t tmValidAge, timestamp_t tmDelay,
    InputViolationStrategy eStrat)
{
    InputConfig inputConfig;
    inputConfig.m_handle = hHandle;
    inputConfig.m_validAge_sim_us = tmValidAge;
    inputConfig.m_delay_sim_us = tmDelay;
    inputConfig.m_inputViolationStrategy = eStrat;
    return inputConfig;
}

OutputConfig makeOutputConfig(handle_t hHandle)
{
    OutputConfig outputConfig;
    outputConfig.m_handle = hHandle;
    return outputConfig;
}

/*
* Test Case:   cTesterStepDataAccess.IgnoreStrategy
* Test ID:     1.0
* Test Title:  Test Step Data Access IgnoreStrategy
* Description: Test ignore strategy for input validity violations
* Strategy:    1) Configure input and output
*              2) Cause input violation
*              3) Check that transmit is called
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id "FEPSDK-1771 FEPSDK-1778 FEPSDK-1783"
 */
TEST_F(cTesterStepDataAccess, IgnoreStrategy)
{
    InputConfig oInputConfig = makeInputConfig(&oInputConfig, 500 * 1000, 0, IS_IGNORE_INPUT_VALIDITY_VIOLATION);
    m_oDataAccess.CreateSampleBuffer(&oInputConfig, 64, 5);

    OutputConfig oOutputConfig = makeOutputConfig(&oOutputConfig);
    m_oDataAccess.StoreOutputSignalSize(&oOutputConfig, 64);

    // Shutdown semaphore ... not set for this test
    a_util::concurrency::semaphore thread_shutdown_semaphore;
    
    // some dummy for output configuration
    m_pStepAccess->ConfigureInput("test", oInputConfig);
    m_pStepAccess->ConfigureOutput("output", oOutputConfig);

    // try validating without valid sample and everything should be fine anyway
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->ValidateInputs(1000 * 1000, thread_shutdown_semaphore));
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->TransmitAllOutputs());

    // transmit was called
    ASSERT_TRUE(m_oDataAccess.m_bTransmit);
}

/*
* Test Case:   cTesterStepDataAccess.WarnStrategy
* Test ID:     1.1
* Test Title:  Test Step Data Access WarnStrategy
* Description: Test warning strategy for input validity violations
* Strategy:    1) Configure input and output
*              2) Cause input violation
*              3) Check that transmit is called and warning incident is triggered
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id "FEPSDK-1771 FEPSDK-1778 FEPSDK-1784"
 */
TEST_F(cTesterStepDataAccess, WarnStrategy)
{
    InputConfig oInputConfig = makeInputConfig(&oInputConfig, 500 * 1000, 0, IS_WARN_ABOUT_INPUT_VALIDITY_VIOLATION);
    m_oDataAccess.CreateSampleBuffer(&oInputConfig, 64, 5);

    OutputConfig oOutputConfig = makeOutputConfig(&oOutputConfig);
    m_oDataAccess.StoreOutputSignalSize(&oOutputConfig, 64);

    // some dummy for output configuration
    m_pStepAccess->ConfigureInput("test", oInputConfig);
    m_pStepAccess->ConfigureOutput("output", oOutputConfig);

    // Shutdown semaphore ... not set for this test
    a_util::concurrency::semaphore thread_shutdown_semaphore;

    // try validating without valid sample and warning should be posted
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->ValidateInputs(1000 * 1000, thread_shutdown_semaphore));
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->TransmitAllOutputs());
    ASSERT_EQ(fep::SL_Warning, m_oIncidentHandler.m_eSeverity);
    ASSERT_EQ(fep::FSI_STEP_LISTENER_INPUT_VALIDITY_VIOLATION, m_oIncidentHandler.m_nCode);
    ASSERT_EQ("Input test does not meet required valid age.", m_oIncidentHandler.m_strDesc);

    // transmit was called
    ASSERT_TRUE(m_oDataAccess.m_bTransmit);
}

/*
* Test Case:   cTesterStepDataAccess.SkipStrategy
* Test ID:     1.2
* Test Title:  Test Step Data Access SkipStrategy
* Description: Test skip strategy for input validity violations
* Strategy:    1) Configure input and output
*              2) Cause input violation
*              3) Check that no transmit was called and critical global incident is triggered
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id "FEPSDK-1771 FEPSDK-1778 FEPSDK-1785"
 */
TEST_F(cTesterStepDataAccess, SkipStrategy)
{
    InputConfig oConfig = makeInputConfig(&oConfig, 500 * 1000, 0, IS_SKIP_OUTPUT_PUBLISH);
    m_oDataAccess.CreateSampleBuffer(&oConfig, 64, 5);

    OutputConfig oOutputConfig = makeOutputConfig(&oOutputConfig);
    m_oDataAccess.StoreOutputSignalSize(&oOutputConfig, 64);

    // some dummy for output configuration
    m_pStepAccess->ConfigureInput("test", oConfig);
    m_pStepAccess->ConfigureOutput("output", oOutputConfig);

    // Shutdown semaphore ... not set for this test
    a_util::concurrency::semaphore thread_shutdown_semaphore;

    // try validating without valid sample and warning should be posted
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->ValidateInputs(1000 * 1000, thread_shutdown_semaphore));
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->TransmitAllOutputs());

    // critical global incident was thrown
    ASSERT_EQ(fep::SL_Critical_Global, m_oIncidentHandler.m_eSeverity);
    ASSERT_EQ(fep::FSI_STEP_LISTENER_INPUT_VALIDITY_VIOLATION, m_oIncidentHandler.m_nCode);
    ASSERT_EQ("Input test does not meet required valid age. CAUTION: defined outputs will not be published!", m_oIncidentHandler.m_strDesc);

    // no transmit was called
    ASSERT_FALSE(m_oDataAccess.m_bTransmit);
}

/*
* Test Case:   cTesterStepDataAccess.ErrorStrategy
* Test ID:     1.3
* Test Title:  Test Step Data Access ErrorStrategy
* Description: Test error strategy for input validity violations
* Strategy:    1) Configure input and output
*              2) Cause input violation
*              3) Check that state machine error event and critical global incident is triggered
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id "FEPSDK-1771 FEPSDK-1778 FEPSDK-1786"
 */
TEST_F(cTesterStepDataAccess, ErrorStrategy)
{
    InputConfig oInputConfig = makeInputConfig(&oInputConfig, 500 * 1000, 0, IS_SET_STM_TO_ERROR);
    m_oDataAccess.CreateSampleBuffer(&oInputConfig, 64, 5);
    m_pStepAccess->ConfigureInput("test", oInputConfig);

    // Shutdown semaphore ... not set for this test
    a_util::concurrency::semaphore thread_shutdown_semaphore;

    // try validating without valid sample and warning should be posted
    ASSERT_NE(a_util::result::SUCCESS, m_pStepAccess->ValidateInputs(1000 * 1000, thread_shutdown_semaphore));

    // critical global incident was thrown
    ASSERT_EQ(fep::SL_Critical_Global, m_oIncidentHandler.m_eSeverity);
    ASSERT_EQ(fep::FSI_STEP_LISTENER_INPUT_VALIDITY_VIOLATION, m_oIncidentHandler.m_nCode);
    ASSERT_EQ("Input test does not meet required valid age. FATAL: changing state to FS_ERROR - continuation not possible!", m_oIncidentHandler.m_strDesc);

    // stm got error event
    ASSERT_TRUE(m_oStateMachine.m_bErrorEventReceived);
}

/*
* Test Case:   cTesterStepDataAccess.TestOutputs
* Test ID:     1.4
* Test Title:  Test Step Data Access Output Configuration
* Description: Test whether outputs are transmitted correctly
* Strategy:    1) Configure one output
*              2) Transmit an configured output and non configured one
*              3) Check output sample timestamps
*              4) Check whether configured output sample is only transmitted after extra call
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id "FEPSDK-1772 FEPSDK-1787"
 */
TEST_F(cTesterStepDataAccess, TestOutputs)
{
    // two dummies for outputs
    handle_t hConfigHandle = &hConfigHandle;
    handle_t hNormalHandle = &hNormalHandle;

    OutputConfig oOutputConfig = makeOutputConfig(&oOutputConfig);
    m_pStepAccess->ConfigureOutput("output", oOutputConfig);

    // Shutdown semaphore ... not set for this test
    a_util::concurrency::semaphore thread_shutdown_semaphore;

    cDataSample oNormalSample;
    oNormalSample.SetSignalHandle(hNormalHandle);
    oNormalSample.SetTime(1000);
    cDataSample oConfigSample;
    oConfigSample.SetSignalHandle(hConfigHandle);
    oConfigSample.SetTime(5000);
    oConfigSample.SetSize(64);
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->ValidateInputs(1000 * 1000, thread_shutdown_semaphore));
    ASSERT_EQ(a_util::result::SUCCESS, dynamic_cast<IStepDataAccess*>(m_pStepAccess)->TransmitData(&oNormalSample));
    ASSERT_EQ(a_util::result::SUCCESS, dynamic_cast<IStepDataAccess*>(m_pStepAccess)->TransmitData(&oConfigSample));
    // only not configured output should be forwarded instantly
    ASSERT_TRUE(m_oDataAccess.m_bTransmit);
    ASSERT_EQ(hNormalHandle, m_oDataAccess.m_vecTransmits[0].hSampleHandle);
    // timestamp should have been changed to current sim time
    ASSERT_EQ(1000 * 1000, m_oDataAccess.m_vecTransmits[0].tmSampleTime);
    // now configured sample should be transmitted
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->TransmitAllOutputs());
    ASSERT_EQ(hConfigHandle, m_oDataAccess.m_vecTransmits[1].hSampleHandle);
    // sample time should have been changed to current sim time
    ASSERT_EQ(1000 * 1000, m_oDataAccess.m_vecTransmits[1].tmSampleTime);
}

/*
* Test Case:   cTesterStepDataAccess.TestValidationOrder
* Test ID:     1.5
* Test Title:  Test Step Data Access Input Validation Order
* Description: Test that inputs are validated in order of strategy severity
* Strategy:    1) Configure one input per strategy
*              2) Successively cause 2 input violations
*              3) Check effects of strategy order (i.e. error > skip etc.)
* Passed If:   no errors occur
* Ticket:      -
* Requirement: FEP_SDK_???
*/

/**
 * @req_id "FEPSDK-1788"
 */
TEST_F(cTesterStepDataAccess, TestValidationOrder)
{
    InputConfig oIgnoreConfig = makeInputConfig(&oIgnoreConfig, 500 * 1000, 0, IS_IGNORE_INPUT_VALIDITY_VIOLATION);
    InputConfig oWarnConfig = makeInputConfig(&oWarnConfig, 500 * 1000, 0, IS_WARN_ABOUT_INPUT_VALIDITY_VIOLATION);
    InputConfig oSkipConfig = makeInputConfig(&oSkipConfig, 500 * 1000, 0, IS_SKIP_OUTPUT_PUBLISH);
    InputConfig oErrorConfig = makeInputConfig(&oErrorConfig, 500 * 1000, 0, IS_SET_STM_TO_ERROR);
    m_oDataAccess.CreateSampleBuffer(&oIgnoreConfig, 64, 5);
    m_oDataAccess.CreateSampleBuffer(&oWarnConfig, 64, 5);
    m_oDataAccess.CreateSampleBuffer(&oSkipConfig, 64, 5);
    m_oDataAccess.CreateSampleBuffer(&oErrorConfig, 64, 5);
    cDataSampleBuffer* pIgnoreBuffer;
    ASSERT_EQ(a_util::result::SUCCESS, m_oDataAccess.GetSampleBuffer(&oIgnoreConfig, pIgnoreBuffer));
    cDataSampleBuffer* pWarnBuffer;
    ASSERT_EQ(a_util::result::SUCCESS, m_oDataAccess.GetSampleBuffer(&oWarnConfig, pWarnBuffer));
    cDataSampleBuffer* pSkipBuffer;
    ASSERT_EQ(a_util::result::SUCCESS, m_oDataAccess.GetSampleBuffer(&oSkipConfig, pSkipBuffer));
    cDataSampleBuffer* pErrorBuffer;
    ASSERT_EQ(a_util::result::SUCCESS, m_oDataAccess.GetSampleBuffer(&oErrorConfig, pErrorBuffer));
    cDataSample oIgnoreSample;
    oIgnoreSample.SetSignalHandle(&oIgnoreConfig);
    oIgnoreSample.SetSize(64);
    cDataSample oWarnSample;
    oWarnSample.SetSignalHandle(&oWarnConfig);
    oWarnSample.SetSize(64);
    cDataSample oSkipSample;
    oSkipSample.SetSignalHandle(&oSkipConfig);
    oSkipSample.SetSize(64);
    cDataSample oErrorSample;
    oErrorSample.SetSignalHandle(&oErrorConfig);
    oErrorSample.SetSize(64);
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->ConfigureInput("Ignore", oIgnoreConfig));
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->ConfigureInput("Warn", oWarnConfig));
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->ConfigureInput("Skip", oSkipConfig));
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->ConfigureInput("Error", oErrorConfig));
    
    // output sample for skip strategy test
    handle_t hOutputHandle = &hOutputHandle;
    OutputConfig oOutputConfig = makeOutputConfig(hOutputHandle);
    m_oDataAccess.StoreOutputSignalSize(hOutputHandle, 64);
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->ConfigureOutput("output", oOutputConfig));
    IUserDataSample* pOutputSample;
    ASSERT_EQ(a_util::result::SUCCESS, m_oDataAccess.CreateUserDataSample(pOutputSample, hOutputHandle));

    timestamp_t tmCurrSimTime = 1000 * 1000;

    // first check good case
    oIgnoreSample.SetTime(500 * 1000);
    oWarnSample.SetTime(500 * 1000);
    oSkipSample.SetTime(500 * 1000);
    oErrorSample.SetTime(500 * 1000);

    pIgnoreBuffer->Update(&oIgnoreSample);
    pWarnBuffer->Update(&oWarnSample);
    pSkipBuffer->Update(&oSkipSample);
    pErrorBuffer->Update(&oErrorSample);

    // Shutdown semaphore ... not set for this test
    a_util::concurrency::semaphore thread_shutdown_semaphore;

    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->ValidateInputs(tmCurrSimTime, thread_shutdown_semaphore));
    // no incident (SL_Info is the default value) and no stm error
    ASSERT_EQ(fep::SL_Info, m_oIncidentHandler.m_eSeverity);
    ASSERT_FALSE(m_oStateMachine.m_bErrorEventReceived);

    // now let skip and error input fail and check whether error strategy is dominant
    tmCurrSimTime = 1500 * 1000;
    oIgnoreSample.SetTime(1000 * 1000);
    oWarnSample.SetTime(1000 * 1000);
    pIgnoreBuffer->Update(&oIgnoreSample);
    pWarnBuffer->Update(&oWarnSample);

    ASSERT_NE(a_util::result::SUCCESS, m_pStepAccess->ValidateInputs(tmCurrSimTime, thread_shutdown_semaphore));
    // incident and stm error event check
    ASSERT_EQ("Input Error does not meet required valid age. FATAL: changing state to FS_ERROR - continuation not possible!",
        m_oIncidentHandler.m_strDesc);
    ASSERT_TRUE(m_oStateMachine.m_bErrorEventReceived);
    // tranmission gets cancelled
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->TransmitAllOutputs());
    ASSERT_FALSE(m_oDataAccess.m_bTransmit);

    // reset
    m_oStateMachine.m_bErrorEventReceived = false;
    m_oIncidentHandler.Reset();

    // now let warn and skip input fail and check whether skip strategy is dominant
    tmCurrSimTime = 2500 * 1000;
    oIgnoreSample.SetTime(2000 * 1000);
    oErrorSample.SetTime(2000 * 1000);
    pIgnoreBuffer->Update(&oIgnoreSample);
    pErrorBuffer->Update(&oErrorSample);

    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->ValidateInputs(tmCurrSimTime, thread_shutdown_semaphore));
    // since skip strategy does not stop computation we still check subsequent inputs and thus receive the warn strategy incident as last one
    ASSERT_EQ("Input Warn does not meet required valid age.",
        m_oIncidentHandler.m_strDesc);
    // but transmission got cancelled anyway
    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->TransmitAllOutputs());
    ASSERT_FALSE(m_oDataAccess.m_bTransmit);

    // reset
    m_oIncidentHandler.Reset();

    // now let ignore and warn input fail and check whether warn strategy is dominant
    tmCurrSimTime = 3500 * 1000;
    oSkipSample.SetTime(3000 * 1000);
    oErrorSample.SetTime(3000 * 1000);
    pSkipBuffer->Update(&oSkipSample);
    pErrorBuffer->Update(&oErrorSample);

    ASSERT_EQ(a_util::result::SUCCESS, m_pStepAccess->ValidateInputs(tmCurrSimTime, thread_shutdown_semaphore));
    // warn incident received
    ASSERT_EQ("Input Warn does not meet required valid age.", m_oIncidentHandler.m_strDesc);


    // delete sample
    delete pOutputSample;
}