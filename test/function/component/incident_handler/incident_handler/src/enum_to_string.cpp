/**
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
* Test Case:   TestEnumToStringAndStringToEnum
* Test ID:     1.7
* Test Title:  cFEPIncident helper class
* Description: Testing functionality of the cFEPIncident helper class.
* Strategy:   All FEP core incident code enums are correctly transformed into string representation
*              and all string representations are correctly transformed into enum.
* Passed If:   see strategy
*              
* Ticket:      #33515
* Requirement: FEPSDK-1569 FEPSDK-1570
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"

#include <fep_test_common.h>
#include <fep_ih_test_common.h>
#include "test_fixture.h"

using namespace fep;

#ifdef WIN32
#pragma push_macro("GetObject")
#undef GetObject
#endif

/**
 * @req_id "FEPSDK-1569 FEPSDK-1570"
 */
TEST_F(TestFixture, TestEnumToStringAndStringToEnum)
{
    //testing enum to string

    std::string strEnumString = "FSI_RESERVED";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_RESERVED));
    strEnumString = "FSI_GENERAL_CRITICAL";
    ASSERT_EQ(strEnumString, fep::cFEPIncident::ToString(fep::FSI_GENERAL_CRITICAL_FAILURE));
    strEnumString = "FSI_GENERAL_CRITICAL";
    ASSERT_EQ(strEnumString, fep::cFEPIncident::ToString(fep::FSI_GENERAL_CRITICAL_GLOBAL_FAILURE));
    strEnumString = "FSI_GENERAL_CRITICAL";
    ASSERT_EQ(strEnumString, fep::cFEPIncident::ToString(fep::FSI_GENERAL_CRITICAL));
    strEnumString = "FSI_GENERAL_WARNING";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_GENERAL_WARNING));
    strEnumString = "FSI_GENERAL_INFORMATION";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_GENERAL_INFORMATION));
    strEnumString = "FSI_DDB_RX_OVERRUN";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_DDB_RX_OVERRUN));
    strEnumString = "FSI_DDB_NOT_INITIALIZED";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_DDB_NOT_INITIALIZED));
    strEnumString = "FSI_DDB_RX_ABORT_SYNC";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_DDB_RX_ABORT_SYNC));
    strEnumString = "FSI_DDB_RX_ABORT_MANUAL";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_DDB_RX_ABORT_MANUAL));
    strEnumString = "FSI_STM_STATE_RQ_FAILED";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_STM_STATE_RQ_FAILED));
    strEnumString = "FSI_STM_STAND_ALONE_MODE";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_STM_STAND_ALONE_MODE));
    strEnumString = "FSI_INCIDENT_CONFIG_FAILED";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_INCIDENT_CONFIG_FAILED));
    strEnumString = "FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID));
    strEnumString = "FSI_TRANSM_MSG_TX_FAILED";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_TRANSM_MSG_TX_FAILED));
    strEnumString = "FSI_TRANSM_DATA_TX_FAILED";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_TRANSM_DATA_TX_FAILED));
    strEnumString = "FSI_TRANSM_RX_MISSING_DATASAMPLE";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_TRANSM_RX_MISSING_DATASAMPLE));
    strEnumString = "FSI_TRANSM_RX_INVALID_STATE";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_TRANSM_RX_INVALID_STATE));
    strEnumString = "FSI_TRANSM_TX_WRONG_SAMPLE_SIZE";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_TRANSM_TX_WRONG_SAMPLE_SIZE));
    strEnumString = "FSI_TRANSM_SAMPLE_VERSION_FAILED";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_TRANSM_SAMPLE_VERSION_FAILED));
    strEnumString = "FSI_TRANSM_FEP_PROTO_WRONG_SER_MODE";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_TRANSM_FEP_PROTO_WRONG_SER_MODE));
    strEnumString = "FSI_TRANSM_FEP_PROTO_CORRUPT_HEADER";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_TRANSM_FEP_PROTO_CORRUPT_HEADER));
    strEnumString = "FSI_MODULE_CREATED_AGAIN";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_MODULE_CREATED_AGAIN));
    strEnumString = "FSI_MAPPING_CONFIG_INVALID";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_MAPPING_CONFIG_INVALID));
    strEnumString = "FSI_SIGNAL_DESCRIPTION_INVALID";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_SIGNAL_DESCRIPTION_INVALID));
    strEnumString = "FSI_SAMPLE_DROPPED_FROM_BACKLOG";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_SAMPLE_DROPPED_FROM_BACKLOG));
    strEnumString = "FSI_SAMPLE_STILL_LOCKED";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_SAMPLE_STILL_LOCKED));
    strEnumString = "FSI_TIMING_CLIENT_CONFIGURATION_FAIL";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_TIMING_CLIENT_CONFIGURATION_FAIL));
    strEnumString = "FSI_TIMING_MASTER_CONFIGURATION_FAIL";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FSI_TIMING_MASTER_CONFIGURATION_FAIL));
    strEnumString = "FSI_TIMING_CLIENT_TRIGGER_SKIP";
    ASSERT_EQ(strEnumString, fep::cFEPIncident::ToString(fep::FSI_TIMING_CLIENT_TRIGGER_SKIP));
    strEnumString = "FSI_TIMING_CLIENT_NOTIF_FAIL";
    ASSERT_EQ(strEnumString, fep::cFEPIncident::ToString(fep::FSI_TIMING_CLIENT_NOTIF_FAIL));
    strEnumString = "FSI_STEP_LISTENER_RUNTIME_VIOLATION";
    ASSERT_EQ(strEnumString, fep::cFEPIncident::ToString(fep::FSI_STEP_LISTENER_RUNTIME_VIOLATION));
    strEnumString = "FSI_STEP_LISTENER_INPUT_VALIDITY_VIOLATION";
    ASSERT_EQ(strEnumString, fep::cFEPIncident::ToString(fep::FSI_STEP_LISTENER_INPUT_VALIDITY_VIOLATION));
    strEnumString = "FUI_RESERVED";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FUI_RESERVED));
    strEnumString = "FUI_READY";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FUI_READY));
    strEnumString = "FUI_SUCCESS";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FUI_SUCCESS));
    strEnumString = "FUI_BYE_BYE";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FUI_BYE_BYE));
    strEnumString = "FUI_AUTO_CONFIG";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FUI_AUTO_CONFIG));
    strEnumString = "FUI_PROPERTY_RECOVERED";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FUI_PROPERTY_RECOVERED));
    strEnumString = "FUI_INVALID_COMMAND_OR_SETTING";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FUI_INVALID_COMMAND_OR_SETTING));
    strEnumString = "FUI_CORRUPTED";
    ASSERT_EQ(strEnumString , fep::cFEPIncident::ToString(fep::FUI_CORRUPTED));

    //testing string to enum

    std::string strEnum = "FSI_RESERVED";
    int32_t eIncident;
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_RESERVED , eIncident);
    strEnum = "FSI_GENERAL_CRITICAL";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_GENERAL_CRITICAL , eIncident);
    strEnum = "FSI_GENERAL_WARNING";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_GENERAL_WARNING , eIncident);
    strEnum = "FSI_GENERAL_INFORMATION";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_GENERAL_INFORMATION , eIncident);
    strEnum = "FSI_DDB_RX_OVERRUN";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_DDB_RX_OVERRUN , eIncident);
    strEnum = "FSI_DDB_NOT_INITIALIZED";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_DDB_NOT_INITIALIZED , eIncident);
    strEnum = "FSI_DDB_RX_ABORT_SYNC";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_DDB_RX_ABORT_SYNC , eIncident);
    strEnum = "FSI_STM_STATE_RQ_FAILED";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_STM_STATE_RQ_FAILED , eIncident);
    strEnum = "FSI_STM_STAND_ALONE_MODE";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_STM_STAND_ALONE_MODE , eIncident);
    strEnum = "FSI_INCIDENT_CONFIG_FAILED";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_INCIDENT_CONFIG_FAILED , eIncident);
    strEnum = "FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID, eIncident);
    strEnum = "FSI_TRANSM_MSG_TX_FAILED";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_TRANSM_MSG_TX_FAILED , eIncident);
    strEnum = "FSI_TRANSM_DATA_TX_FAILED";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_TRANSM_DATA_TX_FAILED , eIncident);
    strEnum = "FSI_TRANSM_RX_MISSING_DATASAMPLE";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_TRANSM_RX_MISSING_DATASAMPLE , eIncident);
    strEnum = "FSI_TRANSM_RX_INVALID_STATE";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_TRANSM_RX_INVALID_STATE , eIncident);
    strEnum = "FSI_TRANSM_TX_WRONG_SAMPLE_SIZE";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_TRANSM_TX_WRONG_SAMPLE_SIZE , eIncident);
    strEnum = "FSI_TRANSM_SAMPLE_VERSION_FAILED";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_TRANSM_SAMPLE_VERSION_FAILED , eIncident);
    strEnum = "FSI_TRANSM_FEP_PROTO_WRONG_SER_MODE";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_TRANSM_FEP_PROTO_WRONG_SER_MODE , eIncident);
    strEnum = "FSI_TRANSM_CONNLIB_STARTUP_FAILED";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    strEnum = "FSI_MODULE_CREATED_AGAIN";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_MODULE_CREATED_AGAIN , eIncident);
    strEnum = "FSI_MAPPING_CONFIG_INVALID";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_MAPPING_CONFIG_INVALID , eIncident);
    strEnum = "FSI_SIGNAL_DESCRIPTION_INVALID";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_SIGNAL_DESCRIPTION_INVALID , eIncident);
    strEnum = "FSI_SAMPLE_DROPPED_FROM_BACKLOG";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_SAMPLE_DROPPED_FROM_BACKLOG , eIncident);
    strEnum = "FSI_SAMPLE_STILL_LOCKED";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_SAMPLE_STILL_LOCKED , eIncident);
    strEnum = "FSI_SERIALIZATION_CHANGE_WITH_REGISTERED_SIGNALS";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_SERIALIZATION_CHANGE_WITH_REGISTERED_SIGNALS, eIncident);
    strEnum = "FSI_MAPPED_SIGNAL_INCONSISTENCY_FAIL";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_MAPPED_SIGNAL_INCONSISTENCY_FAIL, eIncident);
    strEnum = "FSI_TIMING_CLIENT_CONFIGURATION_FAIL";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_TIMING_CLIENT_CONFIGURATION_FAIL, eIncident);
    strEnum = "FSI_TIMING_MASTER_CONFIGURATION_FAIL";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_TIMING_MASTER_CONFIGURATION_FAIL, eIncident);
    strEnum = "FSI_TIMING_CLIENT_TRIGGER_SKIP";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_TIMING_CLIENT_TRIGGER_SKIP, eIncident);
    strEnum = "FSI_TIMING_CLIENT_NOTIF_FAIL";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_TIMING_CLIENT_NOTIF_FAIL, eIncident);
    strEnum = "FSI_STEP_LISTENER_RUNTIME_VIOLATION";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_STEP_LISTENER_RUNTIME_VIOLATION, eIncident);
    strEnum = "FSI_STEP_LISTENER_INPUT_VALIDITY_VIOLATION";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_STEP_LISTENER_INPUT_VALIDITY_VIOLATION, eIncident);
    strEnum = "FSI_DRIVER_ISSUE";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FSI_DRIVER_ISSUE, eIncident);
    strEnum = "FUI_RESERVED";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FUI_RESERVED , eIncident);
    strEnum = "FUI_READY";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FUI_READY , eIncident);
    strEnum = "FUI_SUCCESS";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FUI_SUCCESS , eIncident);
    strEnum = "FUI_BYE_BYE";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FUI_BYE_BYE , eIncident);
    strEnum = "FUI_AUTO_CONFIG";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FUI_AUTO_CONFIG , eIncident);
    strEnum = "FUI_PROPERTY_RECOVERED";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FUI_PROPERTY_RECOVERED , eIncident);
    strEnum = "FUI_INVALID_COMMAND_OR_SETTING";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FUI_INVALID_COMMAND_OR_SETTING , eIncident);
    strEnum = "FUI_CORRUPTED";
    fep::cFEPIncident::FromString(strEnum.c_str(), eIncident);
    ASSERT_EQ(fep::FUI_CORRUPTED , eIncident);
}