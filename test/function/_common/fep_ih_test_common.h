/**
 * Implementation of common auxiliary classes used by the FEP Incident Handler tests
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

#ifndef _FEP_IH_TEST_COMMON_H_INC_
#define _FEP_IH_TEST_COMMON_H_INC_

#include <fstream>
#include <fep_participant_sdk.h>

#include <a_util/filesystem.h>
#include <incident_handler/fep_incident_handler.h>
#include <messages/fep_notification_incident.h>
#include <fep3/components/legacy/property_tree/property.h>

#define SOME_STRATEGY_PROPERTY "sMyStrategy.bWhatever"
static const std::string strTestModuleName = "MyIncidentTestModule";

#define INCIDENCE_FEEDBACK_TIMEOUT static_cast<timestamp_t>(1e1)*1000

enum eTestIncidentNo
{
    // Critical error; no action
    TI_CriticalNoAction = 255,
    // Critical error to handle -> returns error code to handler
    TI_HandleFailure,
    // Critical error -> local error state; manually resolvable.
    TI_CriticalFailure,
    // Critical error with global scope -> local error state,
    // triggers external notifications; manually resolvable
    TI_CriticalFailureGlobal,
    // Critical error; module will go into IDLE mode (fail silent)
    TI_CriticalStop,
    // Critical error; module will attempt automatic restart (fail safe)
    TI_CriticalFailSafe
};


template <class T>
static fep::Result ConfigureIncidentHandler(fep::cIncidentHandler * pIncidentHandler,
     char const * strRelativePath, T value)
{
    fep::cProperty oMyProperty = cProperty(strRelativePath, value);
    return pIncidentHandler->ProcessPropertyChange(&oMyProperty, &oMyProperty, strRelativePath);
}

//############################### Custom Strategy ########################################

class cMyTestStrategy : public fep::IIncidentStrategy
{
public:
    cMyTestStrategy()
    {
        m_sLastEvent.eSeverity = fep::SL_Info;
        m_sLastEvent.nIncident = 0;
        m_fStrategyProperty = 0.0;
        m_nIncidentCount = 0;
        m_nRemoteIncidentCount = 0;
        m_nRefreshPropCount = 0;
    }
    ~cMyTestStrategy() {}

    // IIncidentStrategy interface
public:
    fep::Result HandleLocalIncident(fep::IModule *pModuleContext, const int16_t nIncident,
                           const tSeverityLevel eSeverity,const char* strOrigin, int nLine,
                                const char* strFile,const timestamp_t tmSimTime,
                                const char *strDescription)
    {
        fep::Result nResult = ERR_NOERROR;

        m_sLastEvent.nIncident = nIncident;
        m_sLastEvent.eSeverity = eSeverity;
        m_sLastEvent.strLastDescription = std::string(strDescription);
        m_sLastEvent.strOriginName = std::string(pModuleContext->GetName());
        m_sLastEvent.bExternalIncident = false;

        switch(nIncident)
        {
        case TI_CriticalFailure:
            pModuleContext->GetStateMachine()->ErrorEvent();
            break;
        case TI_HandleFailure:
            nResult = ERR_FAILED;
            break;
        case TI_CriticalFailureGlobal:
        case TI_CriticalStop:
            pModuleContext->GetStateMachine()->StopEvent();
            break;
        case TI_CriticalFailSafe:
            pModuleContext->GetStateMachine()->StopEvent();
            pModuleContext->GetStateMachine()->InitDoneEvent();
            pModuleContext->GetStateMachine()->StartEvent();
            break;
        default:
            break;
        }

        m_oIncidentEvent.notify();
        m_nIncidentCount++;

        return nResult;
    }

    fep::Result HandleGlobalIncident(const char* strSource, const int16_t nIncident,
                                 const tSeverityLevel eSeverity,
                                 const timestamp_t tmSimTime,
                                 const char *strDescription)
    {
        fep::Result nResult = ERR_NOERROR;

        m_sLastEvent.nIncident = nIncident;
        m_sLastEvent.eSeverity = eSeverity;
        m_sLastEvent.strLastDescription = std::string(strDescription);
        m_sLastEvent.strOriginName = std::string(strSource);
        m_sLastEvent.bExternalIncident = true;

        m_oIncidentEvent.notify();
        m_nRemoteIncidentCount++;

        return nResult;
    }

    fep::Result RefreshConfiguration(const IProperty *pBaseProperty,
                                 const IProperty *pAffectedProperty)
    {
        m_nRefreshPropCount ++;

        fep::Result nResult = ERR_NOERROR;

        if (!pBaseProperty || !pAffectedProperty)
        {
            nResult = ERR_POINTER;
        }

        if (fep::isOk(nResult))
        {
            if (std::string(pAffectedProperty->GetPath()) == SOME_STRATEGY_PROPERTY)
            {
                nResult = pAffectedProperty->GetValue(m_fStrategyProperty);
            }
        }

        return nResult;
    }

public: // global member
    struct sLastEvent
    {
        int16_t nIncident;
        fep::tSeverityLevel eSeverity;
        std::string strLastDescription;
        std::string strOriginName;
        std::string strOrigin;
        std::string strFile;
        int nLine;
        bool bExternalIncident;
    } m_sLastEvent;

    a_util::concurrency::semaphore m_oIncidentEvent;
    double m_fStrategyProperty;
    uint32_t m_nIncidentCount;
    uint32_t m_nRemoteIncidentCount;
    uint32_t m_nRefreshPropCount;
};

//############################### Log File Tester ########################################

class cLogFileTester
{
public:
    std::vector<std::string> m_oLastRcvMsgs;
    a_util::filesystem::Path m_strFilename;
    std::fstream m_fs;

public:
    cLogFileTester()
    { }

    bool Open(const a_util::filesystem::Path& strFilename, bool bWrite)
    {
        m_strFilename = strFilename;
        m_fs.open(strFilename.toString().c_str(), m_fs.binary | (bWrite ? m_fs.out : m_fs.in));

        return m_fs.is_open();
    }

    void Close()
    {
        m_fs.close();
        m_strFilename = "";
    }

    fep::Result TruncateLogFile()
    {
        if (m_fs.is_open())
        {
            // close and reopen
            m_fs.close();
            // OM_Write will truncate the file.
            if (!Open(m_strFilename.toString().c_str(), true))
            {
                return ERR_FAILED;
            }

            m_fs.close();

            if (Open(m_strFilename.toString().c_str(), false))
            {
                return ERR_NOERROR;
            }
        }

        return ERR_INVALID_FILE;
    }

    bool WasLoggedToFileExact(const char* strText)
    {
        if (std::find(m_oLastRcvMsgs.begin(), m_oLastRcvMsgs.end(), std::string(strText)) != m_oLastRcvMsgs.end())
        {
            return false;
        }
        return true;
    }

    bool WasLoggedToFileExactLastLine(const char* strText)
    {
        if (!m_oLastRcvMsgs.empty() &&
            m_oLastRcvMsgs[m_oLastRcvMsgs.size() - 1].find(std::string(strText)) != std::string::npos)
        {
            return false;
        }
        return true;
    }

    bool WasLoggedToFilePartial(const char* strText)
    {
        // // DO NOT USE!  Well, this didnt work.... is it buggy?
//        if (0 > m_oLastRcvMsgs.Match(a_util::strings::format("*%s*", strText)))
//        {
//            return false;
//        }
        for (unsigned int nStrCnt = 0; nStrCnt < m_oLastRcvMsgs.size(); nStrCnt++)
        {
            if (m_oLastRcvMsgs[nStrCnt].find(strText, 0) != std::string::npos)
            {
                return true;
            }
        }

        return false;
    }

    bool WasLoggedToFilePartialLastLine(const char* strText)
    {
        if (!m_oLastRcvMsgs.empty() &&
            m_oLastRcvMsgs[m_oLastRcvMsgs.size() - 1].find(strText, 0) != std::string::npos)
        {
            return true;
        }

        return false;
    }

    fep::Result ParseLogFile()
    {
        Clear();
        m_fs.close();

        if (!Open(m_strFilename, false))
        {
            return ERR_INVALID_FILE;
        }

        m_fs.seekp(0);

        std::string strLine;
        while (std::getline(m_fs, strLine))
        {
            if (!strLine.empty())
            {
                m_oLastRcvMsgs.push_back(strLine);
            }
        }

        return ERR_NOERROR;
    }

    void Clear()
    {
        m_oLastRcvMsgs.clear();
    }

    bool GetLoggedTimeStamp(int &value)
    {
        std::string startString = std::string("ST: ");
        size_t nStrCnt = m_oLastRcvMsgs.size() - 1;
        if (m_oLastRcvMsgs[nStrCnt].find(startString, 0) != std::string::npos)
        {
            size_t start = m_oLastRcvMsgs[nStrCnt].find(std::string("ST: "));
            size_t end = m_oLastRcvMsgs[nStrCnt].find(std::string("[us]"));
            start+= startString.size();
            std::string strTimeStamp = std::string(m_oLastRcvMsgs[nStrCnt].c_str()+start);
            strTimeStamp = strTimeStamp.substr(0, end-start);
            value = a_util::strings::toInt32(strTimeStamp);
            return true;
        }
        return false;
    }

    size_t GetSize() const
    {
        std::ifstream file(m_strFilename.toString().c_str(), std::ios::binary | std::ios::ate);
        return file.tellg();
    }
};


#endif // _FEP_IH_TEST_COMMON_H_INC_
