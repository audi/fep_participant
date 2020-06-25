/**
 * Implementation of the helper functions for the tester for the FEP Automation Interface
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

#ifndef _TESTER_AI_HELPER_FUNCTIONS_H_
#define _TESTER_AI_HELPER_FUNCTIONS_H_

#define REM_PROP_TIMEOUT 800
#define SYSTEM_STATE_TIMEOUT 2000

inline std::string StringListToString(const fep::IStringList * poList)
{
    std::vector<std::string> strList;
    for (size_t n = 0; n < poList->GetListSize(); ++n)
    {
        strList.push_back(poList->GetStringAt(n));
    }

    std::sort(strList.begin(), strList.end());

    return a_util::strings::join(strList, ":");
}

// returns a simple DDL for one scalar signal
#define RETURN_MEDIA_DESC(signalname, elementname, datatype) \
    a_util::strings::format(s_strDescriptionTemplateSkalar.c_str(), \
    (signalname), (elementname), (datatype)).c_str()

// DDL template, RETURN_MEDIA_DESC(signalname, elementname, datatype)
static const std::string s_strDescriptionTemplateSkalar = std::string(
    "<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>"
    "<adtf:ddl xmlns:adtf=\"adtf\">"
    "    <header>"
    "        <language_version>3.00</language_version>"
    "        <author>AUDI AG</author>"
    "        <date_creation/>"
    "        <date_change/>"
    "        <description>ADTF Common Description File</description>"
    "    </header>"
    "    <units>"
    "    </units>"
    "    <datatypes>"
    "<datatype name=\"tBool\" size=\"8\" />"
    "<datatype name=\"tChar\" size=\"8\" />"
    "<datatype name=\"tUInt8\" size=\"8\" />"
    "<datatype name=\"tInt8\" size=\"8\" />"
    "<datatype name=\"tUInt16\" size=\"16\" />"
    "<datatype name=\"tInt16\" size=\"16\" />"
    "<datatype name=\"tUInt32\" size=\"32\" />"
    "<datatype name=\"tInt32\" size=\"32\" />"
    "<datatype name=\"tUInt64\" size=\"64\" />"
    "<datatype name=\"tInt64\" size=\"64\" />"
    "<datatype name=\"tFloat32\" size=\"32\" />"
    "<datatype name=\"tFloat64\" size=\"64\" />"
    "    </datatypes>"
    "    <enums>"
    "    </enums>"
    "    <structs>"
    "    <struct alignment=\"1\" name=\"%s\" version=\"2\">"
    "        <element alignment=\"1\" arraysize=\"1\" byteorder=\"LE\" bytepos=\"0\" name=\"%s\" type=\"%s\" />"
    "    </struct>"
    "    </structs>"
    "    <streams>"
    "    </streams>"
    "</adtf:ddl>");

class cReceiverModuleUI16 : public cTestBaseModule, public IUserDataListener
{
public:
    cReceiverModuleUI16()
        :   cTestBaseModule(),
        m_nLastValueReceived(0),
        m_nSumOfReceivedValues(0),
        m_nSamplesReceived(0)
    { }
    fep::Result Update(const fep::IUserDataSample * poSample)
    {
        m_nLastValueReceived = *((uint16_t *) poSample->GetPtr());
        m_nSumOfReceivedValues += m_nLastValueReceived;
        m_nSamplesReceived++;
        return ERR_NOERROR;
    }

public:
    uint16_t m_nLastValueReceived;
    uint16_t m_nSumOfReceivedValues;
    uint16_t m_nSamplesReceived;

};

class cSampListener : public fep::IUserDataListener
{
private:
    a_util::concurrency::semaphore* m_pSampNotif;
    a_util::concurrency::fast_mutex m_oCountMutex;
    uint32_t m_nSampCount;
    uint32_t m_nExpCount;
public:
    cSampListener(a_util::concurrency::semaphore* pSampNotif)
        : m_pSampNotif(pSampNotif), m_oCountMutex(), m_nSampCount(0), m_nExpCount(0) {}
    ~cSampListener() {}
public:
    fep::Result Update(const fep::IUserDataSample* poSample)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oCountMutex);
        m_nSampCount++;
        if (m_nSampCount == m_nExpCount)
        {
            m_pSampNotif->notify();
        }
        return ERR_NOERROR;
    }
    void SetExpectedSamples(uint32_t nExpCount)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oCountMutex);
        m_nExpCount = nExpCount;
    }
    void ResetCount()
    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oCountMutex);
        m_nSampCount = 0;
    }
    uint32_t GetSampleCount()
    {
        a_util::concurrency::unique_lock<a_util::concurrency::fast_mutex> oGuard(m_oCountMutex);
        return m_nSampCount;
    }
};

class cConcurrentSystemStateAwaitor
{
public:
    tState m_eState;
    fep::AutomationInterface * m_pAI;
    std::vector<std::string> m_strModuleList;
    fep::Result m_nResult;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;

    void ThreadFunc()
    {
        m_nResult = m_pAI->WaitForSystemState(m_eState, m_strModuleList, REM_PROP_TIMEOUT * 20);
    }

public:
    cConcurrentSystemStateAwaitor(tState eState, fep::AutomationInterface * pAI,
        std::vector<std::string> strModuleList) :
      m_eState(eState), m_pAI(pAI), m_strModuleList(strModuleList),
      m_nResult(ERR_NOERROR)
    {
        m_pThread.reset(new a_util::concurrency::thread(
            &cConcurrentSystemStateAwaitor::ThreadFunc, this));
    }
    ~cConcurrentSystemStateAwaitor()
    {
        Join();
    }

    void Join()
    {
        if (m_pThread->joinable()) m_pThread->join();
    }
};

class cThreadedStateAwaitor
{
    fep::AutomationInterface * m_pAI;
    std::string m_strModule;
    tState m_eState;
    timestamp_t m_tmTimeout;
    fep::Result m_nResult;
    a_util::memory::unique_ptr<a_util::concurrency::thread> m_pThread;

public:
    void ThreadFunc()
    {
        m_nResult = m_pAI->WaitForParticipantState(m_eState, m_strModule, m_tmTimeout);
    }

public:
    cThreadedStateAwaitor(fep::AutomationInterface * pAI, const char * strModuleName,
        tState eState, timestamp_t tmTimeout) :
        m_pAI(pAI), m_strModule(strModuleName), m_eState(eState),
        m_tmTimeout(tmTimeout), m_nResult(ERR_NOT_READY)
    {
        m_pThread.reset(new a_util::concurrency::thread(&cThreadedStateAwaitor::ThreadFunc, this));
    }

    fep::Result GetAwaitorResult()
    {
        return m_nResult;
    }

    void Join()
    {
        m_pThread->join();
    }
};
#endif 