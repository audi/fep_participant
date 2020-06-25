/**
 * Implementation of adapted tx adapter mockup used by this test
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
#ifndef _TESTER_FEP_DATA_ACCESS_H_INC_
#define _TESTER_FEP_DATA_ACCESS_H_INC_

#include "function/_common/fep_mock_incident_handler.h"
#include "function/_common/fep_mock_property_tree.h"

#include "signal_registry/fep_signal_struct.h"
#include "transmission_adapter/fep_serialization_helpers.h"

class cTesterFepDataAccess : public ::testing::Test {
public:
    static const char* GetSignallName() { return "InSignal"; }
    static const char* GetSignallType() { return "MinimalStruct"; }
    const char* GetSignalDescription() const { return m_strSignalDescription.c_str(); }
    size_t GetSignallSize() const { return m_szSignalSize; }

protected:
    virtual void SetUp()
    {
        m_oTransmissionAdapterMock.MockReset();
        m_oSignalRegistryPrivateMock.MockReset();

        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.Initialize(
            &m_oTransmissionAdapterMock,
            &m_oSignalRegistryPrivateMock,
            &m_oSignalMappingPrivateMock,
            &m_oIncidentHandlerMock,
            &m_oPropertyTreeBaseMock));

        // 0. Set up signal
        {
            cSignalRegistry oSignalRegistry;
            const char* strSignalDescription;
            // Using a real signal registry to get needed values
            ASSERT_EQ(a_util::result::SUCCESS, oSignalRegistry.RegisterSignalDescription(
                "./files/test.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));
            ASSERT_EQ(a_util::result::SUCCESS, oSignalRegistry.ResolveSignalType(GetSignallType(), strSignalDescription));
            m_strSignalDescription = strSignalDescription;
            ASSERT_EQ(a_util::result::SUCCESS, fep::helpers::CalculateSignalSizeFromDescription(GetSignallType(), strSignalDescription, m_szSignalSize));
        }

        // 1. Register Signal
        tSignal oSignal;
        oSignal.strSignalName = GetSignallName();
        oSignal.strSignalType = GetSignallType();
        oSignal.strSignalDesc = GetSignalDescription();
        oSignal.szSampleSize = GetSignallSize();
        oSignal.bIsMapped = false;;
        oSignal.eDirection = SD_Input;
        oSignal.eSerialization = SER_Ddl;
        oSignal.bIsRaw = false;
        oSignal.bIsReliable = false;
        oSignal.bRTIAsyncPub = false;
        oSignal.bRTILowLat = true;
        oSignal.strRTIMulticast = std::string("");
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.RegisterSignal(oSignal, hReceiverSignal));
        m_oSignalRegistryPrivateMock.MockSetup(hReceiverSignal, GetSignallSize(), 1);

        // 2. initializing default sample
        {
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.CreateUserDataSample(m_pDefaultUserDataSample, hReceiverSignal));
            m_oTransmissionAdapterMock.MockSetRecentSample(m_pDefaultUserDataSample);

            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.SignalRegistered(hReceiverSignal));
            ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.SignalBacklogChanged(hReceiverSignal, 1));
        }

        // Create coder
        poFactory= new ddl::CodecFactory("MinimalStruct", GetSignalDescription());
        ASSERT_EQ(a_util::result::SUCCESS, poFactory->isValid());
    }

    virtual void TearDown()
    {
        delete poFactory;

        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.Finalize());

        m_oTransmissionAdapterMock.MockReset();
        m_oSignalRegistryPrivateMock.MockReset();

        delete m_pDefaultUserDataSample;
    }

public:
    void SetBacklogSize(const size_t nBacklogSize)
    {
        m_oSignalRegistryPrivateMock.MockSetup(hReceiverSignal, GetSignallSize(), nBacklogSize);
        ASSERT_EQ(a_util::result::SUCCESS, oDataAccess.SignalBacklogChanged(hReceiverSignal, nBacklogSize));
    }

public:
    cDataAccess oDataAccess;
    ddl::CodecFactory* poFactory;
    handle_t hReceiverSignal;

private:
    std::string m_strSignalDescription;
    size_t m_szSignalSize;

private:
    cMyMockTxAdapter m_oTransmissionAdapterMock;
    cMyMockSignalRegistryPrivate m_oSignalRegistryPrivateMock;
    cMyMockSignalMappingPrivate m_oSignalMappingPrivateMock;
    cMockUpIncidentHandler m_oIncidentHandlerMock;
    cMockPropertyTree m_oPropertyTreeBaseMock;
    IUserDataSample* m_pDefaultUserDataSample;
};

#endif // _TESTER_FEP_DATA_ACCESS_H_INC_