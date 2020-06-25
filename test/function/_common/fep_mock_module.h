/**
 * Implementation of module mockup used by FEP functional test cases!
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

#ifndef _FEP_TEST_MOCK_MODULE_H_INC_
#define _FEP_TEST_MOCK_MODULE_H_INC_

#include <fep_participant_sdk.h>

class cMockModule : public fep::IModule
{
public:
    cMockModule() : m_pPropertyTree(NULL), m_pTxAdapter(NULL), m_pSTM(NULL), m_pTiming(NULL) {} 
    
    virtual ~cMockModule() { }
public:
    virtual const char* GetName() const
    {
        return "";
    }

    virtual fep::IStateMachine* GetStateMachine() const
    {
        return m_pSTM;
    }

    virtual fep::IPropertyTree* GetPropertyTree() const
    {
        return m_pPropertyTree;
    }

    virtual fep::IIncidentHandler* GetIncidentHandler() const
    {
        return NULL;
    }

    virtual fep::INotificationAccess* GetNotificationAccess() const
    {
        return m_pTxAdapter;
    }

    virtual fep::IUserDataAccess* GetUserDataAccess() const
    {
        return NULL;
    }

    virtual fep::ICommandAccess* GetCommandAccess() const
    {
        return m_pTxAdapter;
    }

    virtual fep::ISignalRegistry* GetSignalRegistry() const
    {
        return NULL;
    }

    virtual fep::ISignalMapping* GetSignalMapping() const
    {
        return NULL;
    }

    virtual fep::ITiming* GetTimingInterface() const
    {
        return m_pTiming;
    }

    virtual fep::IRPC* GetRPC() const
    {
        return nullptr;
    }

    virtual uint16_t GetDomainId() const
    {
        return 0;
    }

    virtual const char* GetDomainPrefix() const
    {
        return "";
    }

    void SetMock(fep::IPropertyTree* pPropertyTree)
    {
        m_pPropertyTree = pPropertyTree;
    }

    void SetMock(fep::ITransmissionAdapter* pTxAdapter)
    {
        m_pTxAdapter = pTxAdapter;
    }

    void SetMock(fep::IStateMachine* pSTM)
    {
        m_pSTM = pSTM;
    }

    void SetMock(fep::ITiming* pTiming)
    {
        m_pTiming = pTiming;
    }

    void SetMock(fep::IIncidentHandler* pIncidentHandler)
    {
        m_pIncidentHandler = pIncidentHandler;
    }

private:
    fep::IPropertyTree* m_pPropertyTree;
    fep::ITransmissionAdapter* m_pTxAdapter;
    fep::IStateMachine* m_pSTM;
    fep::ITiming* m_pTiming;
    fep::IIncidentHandler* m_pIncidentHandler;
};

#endif // _FEP_TEST_MOCK_MODULE_H_INC_
