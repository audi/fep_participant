/**
* Implementation of the tester for Mapping
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
* Test Case:   TestMappingBenchmark
* Test ID:     1.14
* Test Title:  Test FEP Mapping Benchmark
* Description: Measures average mapping time per signal
* Strategy:    no errors occur
* Passed If:   see strategy
* Ticket:      #32178
* Requirement: FEPSDK-1398
*/

#include <gtest/gtest.h>
#include "fep_participant_sdk.h"
#include "fep_test_common.h"

#include "a_util/concurrency.h"
#include "mapping/fep_mapping.h"
#include "signal_registry/fep_signal_registry.h"
#include "transmission_adapter/fep_data_sample_factory.h"
#include <ddl.h>

using namespace mapping::oo;
using namespace mapping::rt;
using namespace ddl;

// stripped down version of the internal helper class fep::DDLManager used below
class cTestDDLManager
{
    ddl::DDLDescription* m_poDDL;

public:
    cTestDDLManager() : m_poDDL(NULL)
    {
        m_poDDL = DDLDescription::createDefault();
    }

    ~cTestDDLManager()
    {
        delete m_poDDL;
        m_poDDL = NULL;
    }

    fep::Result LoadDDLFile(const std::string& strFile)
    {
        std::string strDDL;
        if (a_util::filesystem::readTextFile(strFile, strDDL) != a_util::filesystem::OK)
        {
            return ERR_INVALID_FILE;
        }

        ddl::DDLImporter oImp;
        fep::Result nRes = oImp.setXML(strDDL).getErrorCode();
        if (fep::isOk(nRes))
        {
            nRes = oImp.createNew().getErrorCode();
        }

        if (fep::isFailed(nRes))
        {
            nRes = ERR_INVALID_ARG;
        }

        if (fep::isOk(nRes))
        {
            delete m_poDDL;
            m_poDDL = oImp.getDDL();
        }

        return nRes;
    }

    fep::Result ResolveType(const std::string& strType, std::string& strDestination) const
    {
        fep::Result nRes;
        if (strType.empty())
        {
            nRes = ERR_NOT_FOUND;
        }

        if (fep::isOk(nRes))
        {
            nRes = ERR_NOT_FOUND;

            // since the DDL resolver searches units, datatypes, structs and streams
            // we need to make sure first that the type is a struct - since only them are supported
            DDLComplexVec vecStructs = m_poDDL->getStructs();
            DDLComplexIt itStruct =
                std::find_if(vecStructs.begin(), vecStructs.end(), DDLCompareFunctor<>(strType));

            if (itStruct != vecStructs.end())
            {
                DDLResolver oDDLResolver;
                oDDLResolver.setTargetName(strType);
                if (fep::isOk(oDDLResolver.visitDDL(m_poDDL).getErrorCode()))
                {
                    strDestination = oDDLResolver.getResolvedXML();
                    nRes = ERR_NOERROR;
                }
            }
        }
        return nRes;
    }

    const ddl::DDLDescription& GetDDL() const
    {
        return *m_poDDL;
    }
};

/// Test class that drives the mapping engine in a standalone fashion
/// It also provides easy access to source and target buffers to make the test easy to read
#ifdef _MSC_VER
#pragma warning(disable: 4355)
#endif
class MappingDriver : private IMappingEnvironment
{
    struct Signal
    {
        std::string strName;
        std::string strType;
        ISignalListener* pListener;
    };

    struct sPeriodicWrapper
    {
        a_util::system::Timer oTimer;
        mapping::rt::IPeriodicListener* pListener;
        sPeriodicWrapper() : pListener(NULL) {}

        void TimerFunc()
        {
            pListener->onTimer(a_util::system::getCurrentMicroseconds());
        }
    };
    typedef std::map<mapping::rt::IPeriodicListener*, sPeriodicWrapper*> tPeriodicWrappers;

    cTestDDLManager m_oManager;
    MapConfiguration m_oConfig;
    MappingEngine m_oEngine;
    std::map<std::string, Signal> mapSources;
    std::map<std::string, a_util::memory::shared_ptr<ddl::StaticCodec> > mapSourceCoders;
    std::map<std::string, Target::MemoryBuffer> mapSourceBuffers;
    std::map<std::string, a_util::memory::shared_ptr<ddl::StaticCodec> > mapTargetCoders;
    std::map<std::string, Target::MemoryBuffer> mapTargetBuffers;
    std::map<std::string, handle_t> mapTargetHandle;
    std::map<handle_t, std::string> mapHandleTarget;
    tPeriodicWrappers m_mapPeriodicWrappers;

public:
    MappingDriver(const std::string& strDDL, const std::string& strMapping) : m_oEngine(*this)
    {

        // setup engine
        ASSERT_RESULT_OR_THROW(m_oManager.LoadDDLFile(strDDL));
        ASSERT_RESULT_OR_THROW(m_oConfig.setDescription(&m_oManager.GetDDL()).getErrorCode());
        ASSERT_RESULT_OR_THROW(m_oConfig.loadFromFile(strMapping, MapConfiguration::mc_load_mapping).getErrorCode());
        ASSERT_RESULT_OR_THROW(m_oEngine.setConfiguration(m_oConfig).getErrorCode());

    }

    ~MappingDriver()
    {
        m_oEngine.stop();
        m_oEngine.unmapAll();
        mapSourceCoders.clear();
        mapTargetCoders.clear();
    }

    fep::Result AddTarget(const std::string& strTarget)
    {
        ASSERT_OR_THROW(mapTargetCoders.find(strTarget) == mapTargetCoders.end());

        // map target
        ASSERT_RESULT_OR_THROW(m_oEngine.Map(strTarget, mapTargetHandle[strTarget]).getErrorCode());
        mapHandleTarget[mapTargetHandle[strTarget]] = strTarget;
        const MapTarget* pTarget = m_oConfig.getTarget(strTarget);

        // create source buffers and coders
        const MapSourceNameList& lstSources = pTarget->getReferencedSources();
        for (MapSourceNameList::const_iterator it = lstSources.begin(); it != lstSources.end(); ++it)
        {
            const MapSource* pMapSource = m_oConfig.getSource(*it);
            if(mapSourceCoders.find(std::string(pMapSource->getName())) == mapSourceCoders.end())
            {
                std::string strSourceDesc;
                ASSERT_RESULT_OR_THROW(m_oManager.ResolveType(pMapSource->getType(), strSourceDesc));

                ddl::CodecFactory oFac(pMapSource->getType().c_str(), strSourceDesc.c_str());
                ASSERT_RESULT_OR_THROW(oFac.isValid().getErrorCode());

                Target::MemoryBuffer& oSourceBuffer = mapSourceBuffers[pMapSource->getName()];
                oSourceBuffer.resize(oFac.getStaticBufferSize(), 0);

                mapSourceCoders.insert(std::make_pair(std::string(pMapSource->getName()),
                    a_util::memory::shared_ptr<ddl::StaticCodec>(new ddl::StaticCodec(
                    oFac.makeStaticCodecFor(&oSourceBuffer[0], oSourceBuffer.size())))));
            }
        }

        // create target buffer and coder
        std::string strTargetDesc;
        ASSERT_RESULT_OR_THROW(m_oManager.ResolveType(pTarget->getType(), strTargetDesc));

        ddl::CodecFactory oFac(pTarget->getType().c_str(), strTargetDesc.c_str());
        ASSERT_RESULT_OR_THROW(oFac.isValid().getErrorCode());

        Target::MemoryBuffer& oTargetBuffer = mapTargetBuffers[strTarget];
        oTargetBuffer.resize(oFac.getStaticBufferSize(), 0);

        mapTargetCoders.insert(std::make_pair(strTarget, a_util::memory::shared_ptr<ddl::StaticCodec>(new ddl::StaticCodec(
            oFac.makeStaticCodecFor(&oTargetBuffer[0], oTargetBuffer.size())))));

        return ERR_NOERROR;
    }

    void ResetEngine()
    {
        // reset engine
        ASSERT_RESULT_OR_THROW(m_oEngine.reset().getErrorCode());
    }

    void StartEngine()
    {
        // start engine
        ASSERT_RESULT_OR_THROW(m_oEngine.start().getErrorCode());
    }

    void StopEngine()
    {
        // start engine
        ASSERT_RESULT_OR_THROW(m_oEngine.stop().getErrorCode());
    }

    ddl::StaticCodec& GetTargetCoder(const std::string& strTarget)
    {
        return *mapTargetCoders[strTarget];
    }

    ddl::StaticCodec& GetSourceCoder(const std::string& strSource)
    {
        return *mapSourceCoders[strSource];
    }

    fep::Result SendSourceBuffer(const std::string& strSource)
    {
        Target::MemoryBuffer& oSourceBuf = mapSourceBuffers[strSource];
        return mapSources[strSource].pListener->onSampleReceived(&oSourceBuf[0], oSourceBuf.size()).getErrorCode();
    }

    fep::Result ReceiveTargetBuffer(const std::string& strTarget)
    {
        Target::MemoryBuffer& oTargetBuffer = mapTargetBuffers[strTarget];
        handle_t hTargetHandle = mapTargetHandle[strTarget];
        return m_oEngine.getCurrentData(hTargetHandle, &oTargetBuffer[0], oTargetBuffer.size()).getErrorCode();
    }

private:
    fep::Result registerSource(const char* strSourceName, const char* strTypeName,
        ISignalListener* pListener, handle_t& hHandle)
    {
        Signal sig;
        sig.strName = strSourceName;
        sig.strType = strTypeName;
        sig.pListener = pListener;
        mapSources[strSourceName] = sig;
        hHandle = (handle_t)&mapSources[strSourceName];
        return ERR_NOERROR;
    }

    fep::Result unregisterSource(handle_t hHandle)
    {   // dont care
        return ERR_NOERROR;
    }

    fep::Result sendTarget(handle_t hTarget, const void* pData,
        size_t szSize, timestamp_t tmTimeStamp)
    {
        if(szSize == mapTargetBuffers[mapHandleTarget[hTarget]].size())
        {
            a_util::memory::copy(&mapTargetBuffers[mapHandleTarget[hTarget]][0], szSize, pData, szSize);
        }
        return ERR_NOERROR;
    }

    fep::Result targetMapped(const char* strTargetName,
        const char* strTargetType, handle_t hTarget, size_t szTargetType)
    {
        return ERR_NOERROR;
    }

    fep::Result targetUnmapped(const char* strTargetName, handle_t hTarget)
    {
        return ERR_NOERROR;
    }

    fep::Result resolveType(const char* strTypeName, const char*& strTypeDescription)
    {
        static std::string strDesc;
        ASSERT_RESULT_OR_THROW(m_oManager.ResolveType(strTypeName, strDesc));
        strTypeDescription = strDesc.c_str();
        return ERR_NOERROR;
    }

    timestamp_t getTime() const
    {
        return a_util::system::getCurrentMicroseconds();
    }

    fep::Result registerPeriodicTimer(timestamp_t tmPeriod_us, mapping::rt::IPeriodicListener* pListener)
    {
        sPeriodicWrapper* pWrap = new sPeriodicWrapper;
        pWrap->pListener = pListener;

        m_mapPeriodicWrappers[pListener] = pWrap;
        pWrap->oTimer.setPeriod(tmPeriod_us + 1000);
        pWrap->oTimer.setCallback(&sPeriodicWrapper::TimerFunc, *pWrap);
        return pWrap->oTimer.start() ? ERR_NOERROR : fep::Result(ERR_FAILED);
    }

    fep::Result unregisterPeriodicTimer(timestamp_t tmPeriod_us, mapping::rt::IPeriodicListener* pListener)
    {
        tPeriodicWrappers::iterator it = m_mapPeriodicWrappers.find(pListener);
        it->second->oTimer.stop();
        delete it->second;
        m_mapPeriodicWrappers.erase(it);
        return ERR_NOERROR;
    }
};

class TestSignalRegistry : public cSignalRegistry
{
public:
    handle_t GetHandleFor(const char* strSignalName)
    {
        const tSignal* pSig = FindSignal(strSignalName, SD_Input);
        if (!pSig) return NULL;
        for (tHandleMap::const_iterator it = m_mapHandles.begin();
            it != m_mapHandles.end(); ++it)
        {
            if (it->second == pSig) return it->first;
        }
        return NULL;
    }
};

class MyThread
{
    cSignalMapping* pSM;
    IUserDataSample** pSamples;
    uint32_t nStart;
    uint32_t nStop;
    uint32_t bench_factor;
    a_util::memory::unique_ptr<a_util::concurrency::thread> pThread;
public:
    MyThread(cSignalMapping* pSM,  IUserDataSample** pSamples,
        uint32_t nStart, uint32_t nStop, uint32_t bench_factor) :
    pSM(pSM), pSamples(pSamples), nStart(nStart),
        nStop(nStop), bench_factor(bench_factor)
    {
        pThread.reset(new a_util::concurrency::thread(&MyThread::ThreadFunc, this));
    }

    ~MyThread()
    {
        pThread->join();
    }

    void ThreadFunc()
    {
        LOG_INFO("Thread started");
        for(uint32_t i = nStart; i < bench_factor; i++)
        {
            for (uint32_t j = 0; j < nStop; ++j)
            {
                pSM->Update(pSamples[j]);
            }
        }
    }
};

/**
 * @req_id "FEPSDK-1398"
 */
TEST(cTesterFEPMapping, TestMappingBenchmark)
{

#ifdef _DEBUG
    uint32_t bench_factor = 10;
#else
    uint32_t bench_factor = 10000;
#endif
    // Benchmark actual mapping through internal API
    {
        MappingDriver base_test("./files/benchmark.description", "./files/benchmark.map");
        base_test.AddTarget("Output1");
        base_test.AddTarget("Output2");
        base_test.AddTarget("Output3");
        base_test.StartEngine();

        //getc(stdin); // for profiling
        timestamp_t tmStart = a_util::system::getCurrentMicroseconds();
        for(uint32_t i = 0; i < bench_factor; i++)
        {
            base_test.SendSourceBuffer("Input1");
            base_test.SendSourceBuffer("Input2");
            base_test.SendSourceBuffer("Input3");
        }

        timestamp_t tmStop = a_util::system::getCurrentMicroseconds();
        double tmElapsed = ((double)(tmStop - tmStart)) / 1000;
        LOG_INFO("BENCH1 ###############################################");
        LOG_INFO(a_util::strings::format("%dx3 signals mapped in %f ms. Avg. mapping time per signal: %f us",
            bench_factor, tmElapsed, tmElapsed / bench_factor * 1000.f / 3.f).c_str());
        LOG_INFO("######################################################");
    }

    // Now benchmark using FEP API
    {
        cTestBaseModule oModule;
        ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions( "receiver")));
        oModule.GetStateMachine()->StartupDoneEvent(); ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_IDLE, 5000));
        ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignalDescription(
            "./files/benchmark.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));
        ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalMapping()->RegisterMappingConfiguration(
            "./files/benchmark.map", ISignalMapping::MF_MAPPING_FILE | ISignalMapping::MF_REPLACE));
        ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->
            SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, oModule.GetName()));

        oModule.GetStateMachine()->InitializeEvent(); ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_INITIALIZING, 5000));

        handle_t hMapped1 = NULL, hMapped2 = NULL, hMapped3 = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("Output1",
            SD_Input, "StructIn_Modulverbund_100ms"), hMapped1));
        ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("Output2", SD_Input
            , "Player_out"), hMapped2));
        ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->RegisterSignal(fep::cUserSignalOptions("Output3",
            SD_Input, "tCAN_Config_Codec_Ext_NaviMIT_streamNaviMIT"), hMapped3));

        // to avoid testing the transmission layer, we fake input samples by
        // calling the Update method directly
        cSignalMapping* pMapping = dynamic_cast<cSignalMapping*>(oModule.GetSignalMapping());
        ASSERT_TRUE(pMapping);

        IUserDataSample* pSampleIn1 = NULL, *pSampleIn2 = NULL, *pSampleIn3 = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSampleIn1));
        ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSampleIn2));
        ASSERT_EQ(a_util::result::SUCCESS, cDataSampleFactory::CreateSample(&pSampleIn3));

        { // resize samples
            const char* strDesc = NULL;
            ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->
                ResolveSignalType("StructIn_Modulverbund_100ms", strDesc));
            ddl::CodecFactory oFactory("StructIn_Modulverbund_100ms", strDesc);
            ASSERT_EQ(a_util::result::SUCCESS, oFactory.isValid().getErrorCode());

            pSampleIn1->SetSize(oFactory.getStaticBufferSize());

            ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->
                ResolveSignalType("Player_out", strDesc));
            oFactory = ddl::CodecFactory("Player_out", strDesc);
            pSampleIn2->SetSize(oFactory.getStaticBufferSize());

            ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->
                ResolveSignalType("tCAN_Config_Codec_Ext_NaviMIT_streamNaviMIT", strDesc));
            oFactory = ddl::CodecFactory("tCAN_Config_Codec_Ext_NaviMIT_streamNaviMIT", strDesc);
            pSampleIn3->SetSize(oFactory.getStaticBufferSize());
        }

        { // get correct handles
            cSignalRegistry* pSR = dynamic_cast<cSignalRegistry*>(oModule.GetSignalRegistry());
            ASSERT_TRUE(pSR);
            TestSignalRegistry* pPSR = reinterpret_cast<TestSignalRegistry*>(pSR);
            pSampleIn1->SetSignalHandle(pPSR->GetHandleFor("Input1"));
            pSampleIn2->SetSignalHandle(pPSR->GetHandleFor("Input2"));
            pSampleIn3->SetSignalHandle(pPSR->GetHandleFor("Input3"));
        }

        oModule.GetStateMachine()->InitDoneEvent();
        oModule.GetStateMachine()->StartEvent();
        ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_RUNNING, 5000));

        //getc(stdin); // for profiling
        timestamp_t tmStart = a_util::system::getCurrentMicroseconds();
        for(uint32_t i = 0; i < bench_factor; i++)
        {
            pMapping->Update(pSampleIn1);
            pMapping->Update(pSampleIn2);
            pMapping->Update(pSampleIn3);
        }

        timestamp_t tmStop = a_util::system::getCurrentMicroseconds();
        double tmElapsed = ((double)(tmStop - tmStart)) / 1000;
        LOG_INFO("BENCH2 ###############################################");
        LOG_INFO(a_util::strings::format("%dx3 signals mapped in %f ms. Avg. mapping time per signal: %f us",
            bench_factor, tmElapsed, tmElapsed / bench_factor * 1000.f / 3.f).c_str());
        LOG_INFO("######################################################");

        delete pSampleIn1;
        delete pSampleIn2;
        delete pSampleIn3;
    }

    // now benchmark (using only the FEP api) many sources mapping onto a single target
    {
        cTestBaseModule oModule;
        ASSERT_EQ(a_util::result::SUCCESS, oModule.Create(cModuleOptions( "module")));
        ISignalRegistry* pSR = oModule.GetSignalRegistry();
        oModule.GetStateMachine()->StartupDoneEvent(); ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_IDLE, 5000));
        ASSERT_EQ(a_util::result::SUCCESS, pSR->RegisterSignalDescription(
            "./files/benchmark.description", ISignalRegistry::DF_DESCRIPTION_FILE | ISignalRegistry::DF_REPLACE));

        ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalMapping()->RegisterMappingConfiguration(
            "./files/benchmark2.map", ISignalMapping::MF_MAPPING_FILE | ISignalMapping::MF_REPLACE));
        ASSERT_EQ(a_util::result::SUCCESS, oModule.GetPropertyTree()->
            SetPropertyValue(FEP_TIMING_MASTER_PARTICIPANT, oModule.GetName()));

        oModule.GetStateMachine()->InitializeEvent(); ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_INITIALIZING, 5000));

        handle_t hMappedSignal = NULL;
        ASSERT_EQ(a_util::result::SUCCESS, pSR->RegisterSignal(fep::cUserSignalOptions("Output0", SD_Input, "Player_out"), hMappedSignal));

        // to avoid testing the transmission layer, we fake input samples by
        // calling the Update method directly
        cSignalMapping* pMapping = dynamic_cast<cSignalMapping*>(oModule.GetSignalMapping());
        ASSERT_TRUE(pMapping);

        static const int nSources = 93;

        IUserDataSample* pSamples[nSources] = {NULL};
        { // create and resize samples with correct handles
            const char* strDesc = NULL;
            ASSERT_EQ(a_util::result::SUCCESS, oModule.GetSignalRegistry()->
                ResolveSignalType("Float64Struct", strDesc));
            ddl::CodecFactory oFactory("Float64Struct", strDesc);
            ASSERT_EQ(a_util::result::SUCCESS, oFactory.isValid().getErrorCode());
            size_t sz = oFactory.getStaticBufferSize();

            cSignalRegistry* pRealSR = dynamic_cast<cSignalRegistry*>(oModule.GetSignalRegistry());
            ASSERT_TRUE(pRealSR);
            TestSignalRegistry* pPSR = reinterpret_cast<TestSignalRegistry*>(pRealSR);

            for (int i = 0; i < nSources; ++i)
            {
                cDataSampleFactory::CreateSample(&pSamples[i]);
                pSamples[i]->SetSize(sz);
                pSamples[i]->SetSignalHandle(pPSR->GetHandleFor(a_util::strings::format("Input%d", i + 1).c_str()));
            }
        }

        oModule.GetStateMachine()->InitDoneEvent();
        oModule.GetStateMachine()->StartEvent();
        ASSERT_EQ(a_util::result::SUCCESS, oModule.WaitForState(FS_RUNNING, 5000));

        timestamp_t tmStart = 0;
        {
            MyThread t1(pMapping, pSamples, 0, 24, bench_factor);
            MyThread t2(pMapping, pSamples, 24, 48, bench_factor);
            MyThread t3(pMapping, pSamples, 48, 72, bench_factor);
            MyThread t4(pMapping, pSamples, 72, 93, bench_factor);

            //getc(stdin); // for profiling
            tmStart = a_util::system::getCurrentMicroseconds();
        }

        timestamp_t tmStop = a_util::system::getCurrentMicroseconds();
        double tmElapsed = ((double)(tmStop - tmStart)) / 1000;
        LOG_INFO("BENCH3 ###############################################");
        LOG_INFO(a_util::strings::format("%d sources mapped in %f ms. Avg. mapping time per source: %f us",
            bench_factor * nSources, tmElapsed, tmElapsed / bench_factor * 1000.f / nSources).c_str());
        LOG_INFO("######################################################");

        for (int i = 0; i < nSources; ++i)
        {
            delete pSamples[i];
        }
    }
}