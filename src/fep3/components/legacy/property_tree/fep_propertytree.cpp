/**
 * Implementation of the Class cPropertyTree.
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

#include "fep3/components/legacy/property_tree/fep_propertytree.h"
#include <cassert>
#include <iostream>
#include <list>
#include <utility>
#include <a_util/concurrency/chrono.h>
#include <a_util/concurrency/semaphore.h>
#include <a_util/strings/strings_functions.h>

#include "_common/fep_timestamp.h"

#include "fep3/components/legacy/property_tree/property.h"
#include "fep3/components/legacy/property_tree/propertytreebase.h"
#include "fep3/components/legacy/property_tree/property.h"
#include "fep3/base/properties/properties.h"
#include "fep3/base/properties/property_type.h"
#include "fep3/base/properties/property_type_conversion.h"

#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep_errors.h"
#include "messages/fep_command_access_intf.h"
#include "messages/fep_command_delete_property.h"
#include "messages/fep_command_delete_property_intf.h"
#include "messages/fep_command_get_property.h"
#include "messages/fep_command_get_property_intf.h"
#include "messages/fep_command_reg_prop_listener.h"
#include "messages/fep_command_reg_prop_listener_intf.h"
#include "messages/fep_command_set_property.h"
#include "messages/fep_command_set_property_intf.h"
#include "messages/fep_command_unreg_prop_listener.h"
#include "messages/fep_command_unreg_prop_listener_intf.h"
#include "messages/fep_notification_access_intf.h"
#include "messages/fep_notification_name_changed_intf.h"
#include "messages/fep_notification_prop_changed.h"
#include "messages/fep_notification_property.h"
#include "messages/fep_notification_property_intf.h"
#include "messages/fep_notification_reg_prop_listener_ack.h"
#include "messages/fep_notification_reg_prop_listener_ack_intf.h"
#include "messages/fep_notification_unreg_prop_listener_ack.h"
#include "messages/fep_notification_unreg_prop_listener_ack_intf.h"
#include "module/fep_module_intf.h"


using namespace fep;

#define FEP_PREP_CMD_VERSION 1.0

/// PIMPL implementation, see \ref page_d_pointer for details
FEP_UTILS_P_DECLARE(cPropertyTree)
{
public:
    fep::cPropertyTreeBase m_oTree;
};

/// receives one remote property notification
/// needed for getRemoteProperty
class cRemotePropertyNotificationListener : public cNotificationListener
{
private:
    a_util::concurrency::semaphore & m_oEvent;
    char const * m_strModuleName;
    char const * m_strPropPath;
    cProperty ** m_poProperty;
    bool m_bReceivedOne;

public:
    cRemotePropertyNotificationListener(a_util::concurrency::semaphore & oEvent,
        char const * strElementName, char const * strPropPath, cProperty ** poProperty) :
        m_oEvent(oEvent), m_strModuleName(strElementName), m_strPropPath(strPropPath),
        m_poProperty(poProperty), m_bReceivedOne(false) { }

    fep::Result Update(IPropertyNotification const * pPropertyNotification)
    {
        if (m_bReceivedOne)
        {
            return ERR_NOERROR;
        }

        if (std::string(pPropertyNotification->GetSender()) != m_strModuleName)
        {
            // not the sender module we're waiting for here
            return ERR_NOERROR;
        }

        if (std::string(pPropertyNotification->GetPropertyPath()) != m_strPropPath)
        {
            // not the property path we're waiting for here
            return ERR_NOERROR;
        }

        cProperty * pProp =
            dynamic_cast<cProperty *>(
                const_cast<IPropertyNotification *>(pPropertyNotification)
                    ->TakeProperty());
        if (pProp)
        {
            *m_poProperty = pProp;

            m_oEvent.notify();
            m_bReceivedOne = true;

            return ERR_NOERROR;
        }
        else
        {
            assert(pProp);
            return ERR_INVALID_ADDRESS;
        }
    }
};

/// this listener is needed if using an synchronous setRemoteProperty call
/// and to notify the waiting method that the property is set
class cRemoteSetPropertyAckNotListener : public cNotificationListener
{
private:
    a_util::concurrency::semaphore & m_oEvent;
    char const * m_strModuleName;
    char const * m_strPropPath;

public:
    cRemoteSetPropertyAckNotListener(a_util::concurrency::semaphore & oEvent,
        char const * strElementName, char const * strPropPath) :
        m_oEvent(oEvent), m_strModuleName(strElementName), m_strPropPath(strPropPath)
    { 
    }

    fep::Result Update(IPropertyNotification const * pNotification)
    {
        if (std::string(pNotification->GetSender()) != m_strModuleName)
        {
            // not the sender module we're waiting for here
            return ERR_NOERROR;
        }

        if (std::string(pNotification->GetPropertyPath()) != m_strPropPath)
        {
            // not the property path we're waiting for here
            return ERR_NOERROR;
        }

        m_oEvent.notify();
        return ERR_NOERROR;
    }
};

/// receives one register or unregister property listener ack notification
// yep, thats a looooong name - sorry
/// needed for Reg/Unreg MirrorProperties
class cRemotePropListAckNotificationListener : public cNotificationListener
{
private:
    a_util::concurrency::semaphore & m_oEvent;
    char const * m_strModuleName;
    char const * m_strPropPath;
    cProperty * m_poProperty;
    bool m_bWaitingForReg;
    bool m_bReceivedOne;

public:
    cRemotePropListAckNotificationListener(a_util::concurrency::semaphore & oEvent,
        char const * strElementName, char const * strPropPath, bool bWaitingForReg) :
        m_oEvent(oEvent), m_strModuleName(strElementName), m_strPropPath(strPropPath), m_poProperty(NULL),
        m_bWaitingForReg(bWaitingForReg), m_bReceivedOne(false) { }

    ~cRemotePropListAckNotificationListener()
    {
        delete m_poProperty;
    }

    IProperty * GetProperty()
    {
        return m_poProperty;
    }

    fep::Result Update(IRegPropListenerAckNotification const * pNotification)
    {
        // only receive _one_ reg or unreg notification in this instance
        if (m_bReceivedOne)
        {
            return ERR_NOERROR;
        }

        // are we waiting for reg or unreg acknowledgements?
        if (!m_bWaitingForReg)
        {
            return ERR_NOERROR;
        }

        if (std::string(pNotification->GetSender()) != m_strModuleName)
        {
            // not the sender module we're waiting for here
            return ERR_NOERROR;
        }

        if (std::string(pNotification->GetPropertyPath()) != m_strPropPath)
        {
            // not the property path we're waiting for here
            return ERR_NOERROR;
        }

        cProperty * poProp =
            dynamic_cast<cProperty *>(
                const_cast<IRegPropListenerAckNotification *>(pNotification)
                    ->TakeProperty());
        assert(poProp);
        m_poProperty = poProp;

        m_oEvent.notify();
        m_bReceivedOne = true;
        return ERR_NOERROR;
    }

    fep::Result Update(IUnregPropListenerAckNotification const * pNotification)
    {
        // only receive _one_ reg or unreg notification in this instance
        if (m_bReceivedOne)
        {
            return ERR_NOERROR;
        }

        // are we waiting for reg or unreg acknowledgements?
        if (m_bWaitingForReg)
        {
            return ERR_NOERROR;
        }

        if (std::string(pNotification->GetSender())!= m_strModuleName)
        {
            // not the sender module we're waiting for here
            return ERR_NOERROR;
        }

        if (std::string(pNotification->GetPropertyPath()) != m_strPropPath)
        {
            // not the property path we're waiting for here
            return ERR_NOERROR;
        }

        m_oEvent.notify();
        m_bReceivedOne = true;
        return ERR_NOERROR;
    }
};

// FIXME: Find a better solution for adding the root to the path everywhere
cPropertyTree::cPropertyTree() : m_pModule(NULL), m_mapMirroredProperties(), m_mapProxyChangeListeners(),
    m_mapRemoteChangeListeners()
{
    FEP_UTILS_D_CREATE(cPropertyTree);
}

cPropertyTree::~cPropertyTree ()
{
    SetModule(NULL);
}

fep::Result cPropertyTree::SetModule(const fep::IModule* pModule)
{
    if (pModule != m_pModule)
    {
        if (NULL != m_pModule)
        {
            m_pModule->GetCommandAccess()->UnregisterCommandListener(this);
            m_pModule->GetNotificationAccess()->UnregisterNotificationListener(this);

            // delete remote listeners
            tMirroredChangeListener::iterator itRemoteListeners = m_mapRemoteChangeListeners.begin();
            for (;itRemoteListeners != m_mapRemoteChangeListeners.end(); ++itRemoteListeners)
            {
                m_pModule->GetNotificationAccess()->UnregisterNotificationListener(
                    itRemoteListeners->second);
                delete itRemoteListeners->second;
            }
            m_mapRemoteChangeListeners.clear();

            {
                a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLockGuard(m_oMapProxyChangeMutex);

                // delete proxy listeners
                tProxyMap::iterator itProxyListeners = m_mapProxyChangeListeners.begin();
                for (;itProxyListeners != m_mapProxyChangeListeners.end(); ++itProxyListeners)
                {
                    itProxyListeners->first->UnregisterListener(itProxyListeners->second);
                    delete itProxyListeners->second;
                }
                m_mapProxyChangeListeners.clear();
            }
        }

        m_pModule = pModule;

        if (NULL != m_pModule)
        {
            m_pModule->GetCommandAccess()->RegisterCommandListener(this);
            m_pModule->GetNotificationAccess()->RegisterNotificationListener(this);
        }
    }
    return ERR_NOERROR;
}


template <typename T>
fep::Result SetPropertyValuesTemplate(char const * strPropPath,
    T const * firstValue, size_t szArraySize, fep::IPropertyTree * pTree)
{
    fep::Result nResult;
    if (NULL == pTree ||
        NULL == strPropPath ||
        NULL == firstValue)
    {
        nResult = ERR_POINTER;
    }

    if (0 == szArraySize)
    {
        nResult = ERR_INVALID_ARG;
    }

    if (fep::isOk(nResult))
    {
        nResult |= pTree->SetPropertyValue(strPropPath,firstValue[0]);
        if (fep::isOk(nResult))
        {
            IProperty * pProp = pTree->GetProperty(strPropPath);
            if (pProp)
            {
                for (size_t nPos=1; nPos < szArraySize; ++nPos)
                {
                    nResult |= pProp->AppendValue(firstValue[nPos]);
                }
            }
        }
    }
    return nResult;
}

fep::Result cPropertyTree::SetPropertyValues(char const * strPropPath,
    char const *const * strFirstValue, size_t szArraySize)
{
    return SetPropertyValuesTemplate(strPropPath,strFirstValue,szArraySize,this);
}
fep::Result cPropertyTree::SetPropertyValues(char const * strPropPath,
    double const * f64FirstValue, size_t szArraySize)
{
    return SetPropertyValuesTemplate(strPropPath,f64FirstValue,szArraySize,this);
}
fep::Result cPropertyTree::SetPropertyValues(char const * strPropPath,
    int32_t const * n32FirstValue, size_t szArraySize)
{
    return SetPropertyValuesTemplate(strPropPath,n32FirstValue,szArraySize,this);
}
fep::Result cPropertyTree::SetPropertyValues(char const * strPropPath,
    bool const * bFirstValue, size_t szArraySize)
{
    return SetPropertyValuesTemplate(strPropPath,bFirstValue,szArraySize,this);
}


template <typename T>
fep::Result SetRemotePropertyValueTemplate(
    char const * strElementName, char const * strPropPath, T const value,
    const fep::IModule * pModule, const timestamp_t tmTimeout /* = 0 */)
{
    fep::Result nResult;
    if (NULL == strElementName ||
        NULL == strPropPath ||
        NULL == pModule)
    {
        nResult = ERR_POINTER;
    }

    // create notification listener
    a_util::concurrency::semaphore oEvent;
    cRemoteSetPropertyAckNotListener oListener(oEvent, strElementName,
        strPropPath);
    if (tmTimeout > 0)
    {
        nResult = pModule->GetNotificationAccess()->RegisterNotificationListener(&oListener);
    }

    if (fep::isOk(nResult))
    {
        cSetPropertyCommand oCmd(
            value, strPropPath,
            pModule->GetName(), strElementName, GetTimeStampMicrosecondsUTC(),
            pModule->GetTimingInterface()->GetTime());
        nResult = pModule->GetCommandAccess()->TransmitCommand(&oCmd);
    }

    if (tmTimeout > 0)
    {
        // wait for the event to signal the arrival of the notification
        nResult = oEvent.wait_for(a_util::chrono::milliseconds(tmTimeout));
        if (fep::isFailed(nResult))
        {
            nResult = ERR_TIMEOUT;
        }

        // unregister listener before doing anything else
        pModule->GetNotificationAccess()->UnregisterNotificationListener(&oListener);
    }

    return nResult;
}

fep::Result cPropertyTree::SetRemotePropertyValue(char const * strElementName,
    char const * strPropPath, char const * strValue, const timestamp_t tmTimeout/* =0 */)
{
    std::lock_guard<std::recursive_mutex> lock(m_oLockDamagedRemoteAccess);
    if (!strValue) { return ERR_POINTER; }
    return SetRemotePropertyValueTemplate(strElementName, strPropPath, strValue, m_pModule, tmTimeout);
}

fep::Result cPropertyTree::SetRemotePropertyValue(char const * strElementName,
    char const * strPropPath, double const f64Value, const timestamp_t tmTimeout/* =0 */)
{
    std::lock_guard<std::recursive_mutex> lock(m_oLockDamagedRemoteAccess);
    return SetRemotePropertyValueTemplate(strElementName, strPropPath, f64Value, m_pModule, tmTimeout);
}

fep::Result cPropertyTree::SetRemotePropertyValue(char const * strElementName,
    char const * strPropPath, int32_t const n32Value, const timestamp_t tmTimeout/* =0 */)
{
    std::lock_guard<std::recursive_mutex> lock(m_oLockDamagedRemoteAccess);
    return SetRemotePropertyValueTemplate(strElementName, strPropPath, n32Value, m_pModule, tmTimeout);
}

fep::Result cPropertyTree::SetRemotePropertyValue(char const * strElementName,
    char const * strPropPath, bool const bValue, const timestamp_t tmTimeout /* =0 */)
{
    std::lock_guard<std::recursive_mutex> lock(m_oLockDamagedRemoteAccess);
    return SetRemotePropertyValueTemplate(strElementName, strPropPath, bValue, m_pModule, tmTimeout);
}

template <typename T>
fep::Result SetRemotePropertyValuesTemplate(char const * strElementName,
    char const * strPropPath, T const * firstValue, size_t szArraySize, const fep::IModule * pModule, 
    const timestamp_t tmTimeout /* =0 */)
{
    fep::Result nResult;
    if (NULL == strElementName ||
        NULL == strPropPath ||
        NULL == firstValue ||
        NULL == pModule)
    {
        nResult = ERR_POINTER;
    }

    if (0 == szArraySize)
    {
        nResult = ERR_INVALID_ARG;
    }

    // create notification listener
    a_util::concurrency::semaphore oEvent;
    cRemoteSetPropertyAckNotListener oListener(oEvent, strElementName,
        strPropPath);
    if (tmTimeout > 0)
    {
        nResult = pModule->GetNotificationAccess()->RegisterNotificationListener(&oListener);
    }

    if (fep::isOk(nResult))
    {
        cSetPropertyCommand oCmd(
            firstValue[0], strPropPath,
            pModule->GetName(), strElementName, GetTimeStampMicrosecondsUTC(),
            pModule->GetTimingInterface()->GetTime());
        for (size_t nPos = 1; nPos < szArraySize; ++ nPos)
        {
             nResult |= oCmd.AppendValue(firstValue[nPos]);
        }
        if (fep::isOk(nResult))
        {
            nResult = pModule->GetCommandAccess()->TransmitCommand(&oCmd);
        }
        else
        {
            nResult |= ERR_UNEXPECTED;
        }
    }

    if (tmTimeout > 0)
    {
        // wait for the event to signal the arrival of the notification
        nResult = oEvent.wait_for(a_util::chrono::milliseconds(tmTimeout));
        if (fep::isFailed(nResult))
        {
            nResult = ERR_TIMEOUT;
        }

        // unregister listener before doing anything else
        pModule->GetNotificationAccess()->UnregisterNotificationListener(&oListener);
    }
    return nResult;
}

fep::Result cPropertyTree::SetRemotePropertyValues(char const * strElementName,
            char const * strPropPath,char const *const * strFirstValue, size_t szArraySize, 
            const timestamp_t tmTimeout /*=0*/)
{
    std::lock_guard<std::recursive_mutex> lock(m_oLockDamagedRemoteAccess);
    return SetRemotePropertyValuesTemplate(strElementName, strPropPath, strFirstValue, szArraySize,
        m_pModule, tmTimeout);
}

fep::Result cPropertyTree::SetRemotePropertyValues(char const * strElementName,
    char const * strPropPath, double const * f64FirstValue, size_t szArraySize,
    const timestamp_t tmTimeout /*=0*/)
{
    std::lock_guard<std::recursive_mutex> lock(m_oLockDamagedRemoteAccess);
    return SetRemotePropertyValuesTemplate(strElementName, strPropPath, f64FirstValue, szArraySize,
        m_pModule, tmTimeout);
}

fep::Result cPropertyTree::SetRemotePropertyValues(char const * strElementName,
    char const * strPropPath, int32_t const * n32FirstValue, size_t szArraySize,
    const timestamp_t tmTimeout /*=0*/)
{
    std::lock_guard<std::recursive_mutex> lock(m_oLockDamagedRemoteAccess);
    return SetRemotePropertyValuesTemplate(strElementName, strPropPath, n32FirstValue, szArraySize,
        m_pModule, tmTimeout);
}

fep::Result cPropertyTree::SetRemotePropertyValues(char const * strElementName,
    char const * strPropPath, bool const * bFirstValue, size_t szArraySize,
    const timestamp_t tmTimeout /*=0*/)
{
    std::lock_guard<std::recursive_mutex> lock(m_oLockDamagedRemoteAccess);
    return SetRemotePropertyValuesTemplate(strElementName, strPropPath, bFirstValue, szArraySize,
        m_pModule, tmTimeout);
}

IProperty * cPropertyTree::GetLocalProperty(char const * strPropPath)
{
    return GetProperty(strPropPath);
}

IProperty const * cPropertyTree::GetLocalProperty(char const * strPropPath) const
{
    return GetProperty(strPropPath);
}

fep::Result cPropertyTree::DeleteRemoteProperty(char const * strElementName, char const * strPropPath)
{
    std::lock_guard<std::recursive_mutex> lock(m_oLockDamagedRemoteAccess);
    fep::Result nResult;
    if (NULL == strElementName || NULL == strPropPath)
    {
        nResult = ERR_POINTER;
    }

    if (fep::isOk(nResult))
    {
        cDeletePropertyCommand oCmd(
            strPropPath,
            m_pModule->GetName(), strElementName, GetTimeStampMicrosecondsUTC(),
            m_pModule->GetTimingInterface()->GetTime());
        nResult = m_pModule->GetCommandAccess()->TransmitCommand(&oCmd);
    }
    return nResult;
}

fep::Result cPropertyTree::GetRemoteProperty(char const * strElementName,
    char const * strPropPath, IProperty ** pProperty, timestamp_t const tmTimeout) 
{
    std::lock_guard<std::recursive_mutex> lock(m_oLockDamagedRemoteAccess);
    if (!strElementName ||
        !strPropPath ||
        !pProperty ||
        NULL == m_pModule)
    {
        return ERR_POINTER;
    }

    // check for wildcards (additional checks already performed in cModule::Create())
    std::string strName(strElementName);
    if (tmTimeout < 0 ||
        strName.empty() ||
        strName.find('*') != std::string::npos ||
        strName.find('?') != std::string::npos)
    {
        return ERR_INVALID_ARG;
    }

    cProperty * poProp = NULL;

    // create notification listener
    a_util::concurrency::semaphore oEvent;
    cRemotePropertyNotificationListener oListener(oEvent, strElementName,
        strPropPath, &poProp);
    RETURN_IF_FAILED(m_pModule->GetNotificationAccess()->RegisterNotificationListener(&oListener));

    // create & send get property command
    cGetPropertyCommand oCmd(strPropPath,
        m_pModule->GetName(), strElementName, GetTimeStampMicrosecondsUTC(), m_pModule->GetTimingInterface()->GetTime());

    if (fep::isFailed(m_pModule->GetCommandAccess()->TransmitCommand(&oCmd)))
    {
        m_pModule->GetNotificationAccess()->UnregisterNotificationListener(&oListener);
        return ERR_FAILED;
    }

    // wait for the event to signal the arrival of the property notification
    fep::Result nRes = oEvent.wait_for(a_util::chrono::milliseconds(tmTimeout));
    if (fep::isFailed(nRes))
    {
        nRes = ERR_TIMEOUT;
    }

    // unregister listener before doing anything else
    m_pModule->GetNotificationAccess()->UnregisterNotificationListener(&oListener);

    // received property was filled by listener, put it into the destination property
    *pProperty = poProp;

    return nRes;
}

fep::Result cPropertyTree::MirrorRemoteProperty(char const * strElementName,
    char const * strRemotePath, char const * strLocalPath, timestamp_t const tmTimeout)
{
    if (!strElementName ||
        !strRemotePath ||
        !strLocalPath ||
        NULL == m_pModule)
    {
        return ERR_POINTER;
    }

    // check for wildcards
    std::string strName(strElementName);
    if (tmTimeout < 0 ||
        strName.empty() ||
        strName.find('*') != std::string::npos ||
        strName.find('?') != std::string::npos)
    {
        return ERR_INVALID_ARG;
    }

    // find a potentially already existing mirrored property subscription
    sMirroredRemoteProperty oMirrInfo;
    oMirrInfo.strElementName = strElementName;
    oMirrInfo.strRemotePath = strRemotePath;
    oMirrInfo.strLocalPath = strLocalPath;

    std::map<sMirroredRemoteProperty, IProperty *>::iterator itMirrProp =
        m_mapMirroredProperties.find(oMirrInfo);
    if (itMirrProp != m_mapMirroredProperties.end())
    {
        // i.e there is an existing subscription
        // -> remove it locally and act as if it never existed
        RETURN_IF_FAILED(RemoveMirroredPropertyLocalSubscription(itMirrProp->second, itMirrProp->first));
    }

    // create notification listener
    a_util::concurrency::semaphore oEvent;
    cRemotePropListAckNotificationListener oListener(oEvent, strElementName,
        strRemotePath, true);
    m_pModule->GetNotificationAccess()->RegisterNotificationListener(&oListener);

    // create & send register property listener command
    cRegPropListenerCommand oCmd(strRemotePath,
        m_pModule->GetName(), strElementName, GetTimeStampMicrosecondsUTC(), m_pModule->GetTimingInterface()->GetTime());

    if (fep::isFailed(m_pModule->GetCommandAccess()->TransmitCommand(&oCmd)))
    {
        return ERR_FAILED;
    }

    // wait for the event to signal the arrival of the notification
    fep::Result nRes = oEvent.wait_for(a_util::chrono::milliseconds(tmTimeout));
    if (fep::isFailed(nRes))
    {
        nRes = ERR_TIMEOUT;
    }

    // unregister listener before doing anything else
    m_pModule->GetNotificationAccess()->UnregisterNotificationListener(&oListener);

    // check for timeout
    if (fep::isFailed(nRes))
    {
        return ERR_TIMEOUT;
    }

    // get the property that was received with the reg ack notification
    IProperty * poProperty = oListener.GetProperty();

    // set name if whole tree was fetched
    if (a_util::strings::isEqual(strRemotePath, ""))
    {
        poProperty->SetName(strElementName);
    }

    // insert received property into local tree
    IProperty * poResProperty = _d->m_oTree.MergeProperty(strLocalPath, poProperty);
    if (!poResProperty)
    {
        return ERR_FAILED;
    }

    // insert into map of mirrored properties so that the notification listener can find it
    oMirrInfo.strElementName = std::string(strElementName);
    oMirrInfo.strRemotePath = std::string(strRemotePath);
    oMirrInfo.strLocalPath = std::string(strLocalPath);

    // we need the iterator to the inserted element in order to get a valid pointer later
    std::pair<std::map<sMirroredRemoteProperty, IProperty *>::iterator, bool> ret;
    ret = m_mapMirroredProperties.insert(
        std::pair<sMirroredRemoteProperty, IProperty *>(oMirrInfo, poResProperty)
    );

    // create property changed listener
    cMirroredPropertyChangedNotificationListener * poListener =
        new cMirroredPropertyChangedNotificationListener(&ret.first->first, poResProperty, this);
    m_pModule->GetNotificationAccess()->RegisterNotificationListener(poListener);

    m_mapRemoteChangeListeners.insert(std::make_pair(poResProperty, poListener));

    return ERR_NOERROR;
}

fep::Result cPropertyTree::ElementNameChanged(char const * strOldElementName, char const * strNewElementName)
{
    if (a_util::strings::isEqual(strOldElementName, strNewElementName))
    {
        return ERR_NOERROR;
    }
    // change name in mirrored properties
    std::map<sMirroredRemoteProperty, IProperty*> mapCopy;
    for (std::map<sMirroredRemoteProperty, IProperty*>::iterator mapIt = m_mapMirroredProperties.begin();
        mapIt != m_mapMirroredProperties.end(); ++mapIt)
    {
        sMirroredRemoteProperty oCopy = mapIt->first;

        if (mapIt->first.strElementName == strOldElementName)
        {
            oCopy.strElementName = strNewElementName;
        }

        mapCopy.insert(
            std::pair<sMirroredRemoteProperty, IProperty *>(oCopy, mapIt->second));
    }

    // first, lock all listeners
    for (std::map<IProperty*, cMirroredPropertyChangedNotificationListener*>::iterator mapIt =
        m_mapRemoteChangeListeners.begin();
        mapIt != m_mapRemoteChangeListeners.end(); ++mapIt)
    {
        mapIt->second->LockUpdates();
    }

    // now do the change
    m_mapMirroredProperties.swap(mapCopy);

    for (std::map<IProperty*, cMirroredPropertyChangedNotificationListener*>::iterator mapIt =
        m_mapRemoteChangeListeners.begin();
        mapIt != m_mapRemoteChangeListeners.end(); ++mapIt)
    {
        // due to limitation of API (we dont want to change the key but the associated value) we need a const cast here
        const sMirroredRemoteProperty*& pInfo = mapIt->second->m_poInfo;
        sMirroredRemoteProperty*& pEditable = const_cast<sMirroredRemoteProperty*&>(pInfo);

        for (std::map<sMirroredRemoteProperty, IProperty*>::iterator mapItInner = m_mapMirroredProperties.begin();
            mapItInner != m_mapMirroredProperties.end(); ++mapItInner)
        {
            if (mapItInner->second == mapIt->first)
            {
                const sMirroredRemoteProperty& pInnerInfo = mapItInner->first;
                pEditable = &const_cast<sMirroredRemoteProperty&>(pInnerInfo);
                
                break;
            }
        }
        mapIt->second->UnlockUpdates();
    }

    // change name in proxy listeners
    a_util::concurrency::unique_lock<a_util::concurrency::mutex> oGuard(m_oMapProxyChangeMutex);
    for (tProxyMap::iterator it = m_mapProxyChangeListeners.begin(); it != m_mapProxyChangeListeners.end(); ++it)
    {
        if (it->second->m_strRemoteModule == strOldElementName)
        {
            it->second->m_strRemoteModule = strNewElementName;
        }
    }

    return ERR_NOERROR;
}

fep::Result cPropertyTree::UnmirrorRemoteProperty(char const * strElementName,
    char const * strRemotePath, char const * strLocalPath, timestamp_t const tmTimeout)
{
    if (!strElementName ||
        !strRemotePath ||
        !strLocalPath ||
        NULL == m_pModule)
    {
        return ERR_POINTER;
    }

    // check for wildcards
    std::string strName(strElementName);
    if (tmTimeout < 0 || strName.find('*') != std::string::npos)
    {
        return ERR_INVALID_ARG;
    }

    // check if it really is a mirrored property
    sMirroredRemoteProperty oProp;
    oProp.strElementName = std::string(strElementName);
    oProp.strRemotePath = std::string(strRemotePath);
    oProp.strLocalPath = std::string(strLocalPath);

    std::map<sMirroredRemoteProperty, IProperty *>::iterator itEntry =
        m_mapMirroredProperties.find(oProp);
    if (m_mapMirroredProperties.end() == itEntry)
    {
        return ERR_INVALID_ARG;
    }

    // remove subscription locally first, all error cases after this line
    // are caused by remote module malfunction so the subscription would
    // be invalid anyway.
    RemoveMirroredPropertyLocalSubscription(itEntry->second, itEntry->first);

    // create notification listener
    a_util::concurrency::semaphore oEvent;
    cRemotePropListAckNotificationListener oListener(oEvent, strElementName,
        strRemotePath, false);
    m_pModule->GetNotificationAccess()->RegisterNotificationListener(&oListener);

    // create & send register property listener command
    cUnregPropListenerCommand oCmd(strRemotePath,
        m_pModule->GetName(), strElementName, GetTimeStampMicrosecondsUTC(),
        m_pModule->GetTimingInterface()->GetTime());

    if (fep::isFailed(m_pModule->GetCommandAccess()->TransmitCommand(&oCmd)))
    {
        m_pModule->GetNotificationAccess()->UnregisterNotificationListener(&oListener);
        return ERR_FAILED;
    }

    // wait for the event to signal the arrival of the notification
    fep::Result nRes = oEvent.wait_for(a_util::chrono::milliseconds(tmTimeout));
    if (fep::isFailed(nRes))
    {
        nRes = ERR_TIMEOUT;
    }

    // unregister listener before doing anything else
    m_pModule->GetNotificationAccess()->UnregisterNotificationListener(&oListener);

    // check for timeout
    if (fep::isFailed(nRes))
    {
        return ERR_TIMEOUT;
    }

    return ERR_NOERROR;
}


IProperty const * cPropertyTree::GetProperty(char const * strPropPath) const
{
    return _d->m_oTree.GetProperty(strPropPath);
}

IProperty * cPropertyTree::GetProperty(char const * strPropPath)
{
    return _d->m_oTree.GetProperty(strPropPath);
}

fep::Result cPropertyTree::SetPropertyValue(char const * strPropPath, char const * strValue)
{
    return _d->m_oTree.SetPropertyValue(strPropPath, strValue);
}

fep::Result cPropertyTree::SetPropertyValue(char const * strPropPath, double const f64Value)
{
    return _d->m_oTree.SetPropertyValue(strPropPath, f64Value);
}

fep::Result cPropertyTree::SetPropertyValue(char const * strPropPath, int32_t const n32Value)
{
    return _d->m_oTree.SetPropertyValue(strPropPath, n32Value);
}

fep::Result cPropertyTree::SetPropertyValue(char const * strPropPath, bool const bValue)
{
    return _d->m_oTree.SetPropertyValue(strPropPath, bValue);
}

fep::Result cPropertyTree::GetPropertyValue(const char * strPropPath, const char *& strValue) const
{
    return _d->m_oTree.GetPropertyValue(strPropPath, strValue);
}

fep::Result cPropertyTree::GetPropertyValue(const char * strPropPath, double & fValue) const
{
    return _d->m_oTree.GetPropertyValue(strPropPath, fValue);
}

fep::Result cPropertyTree::GetPropertyValue(const char * strPropPath, int32_t & nValue) const
{
    return _d->m_oTree.GetPropertyValue(strPropPath, nValue);
}

fep::Result cPropertyTree::GetPropertyValue(const char * strPropPath, bool & bValue) const
{
    return _d->m_oTree.GetPropertyValue(strPropPath, bValue);
}

fep::Result cPropertyTree::RegisterListener(char const * strPropertyPath,
    IPropertyListener* const poListener)
{
    return _d->m_oTree.RegisterListener(strPropertyPath, poListener);
}

fep::Result cPropertyTree::UnregisterListener(char const * strPropertyPath,
    IPropertyListener* const poListener)
{
    return _d->m_oTree.UnregisterListener(strPropertyPath, poListener);
}

fep::Result cPropertyTree::ClearProperty(char const * strPropertyPath)
{
    return _d->m_oTree.ClearProperty(strPropertyPath);
}

fep::Result cPropertyTree::DeleteProperty(char const * strPropertyPath)
{
    if (!strPropertyPath) { return ERR_POINTER; }

    IProperty * poProp = GetProperty(strPropertyPath);
    if (poProp)
    {
        DeletePropertyFromMap(poProp);

        auto lstSubProperties = poProp->GetSubProperties();
        for (const auto& poSubProperty : lstSubProperties)
        {
            if (isFailed(DeleteProperty(poSubProperty->GetPath())))
            {
                return ERR_PATH_NOT_FOUND;
            }
        }
    }

    return _d->m_oTree.DeleteProperty(strPropertyPath);
}

fep::Result cPropertyTree::Update(fep::ISetPropertyCommand const * poCommand)
{
    fep::Result nResult = ERR_NOERROR;
    std::string strPropPath(poCommand->GetPropertyPath());

    // set value to property
    const char * strVal = NULL;
    double fVal;
    int32_t nVal;
    bool bVal;
    if (poCommand->IsBoolean())
    {
        poCommand->GetValue(bVal);
        nResult = SetPropertyValue(poCommand->GetPropertyPath(), bVal);
    }
    else if (poCommand->IsFloat())
    {
        poCommand->GetValue(fVal);
        nResult = SetPropertyValue(poCommand->GetPropertyPath(), fVal);
    }
    else if (poCommand->IsInteger())
    {
        poCommand->GetValue(nVal);
        nResult = SetPropertyValue(poCommand->GetPropertyPath(), nVal);
    }
    else if (poCommand->IsString())
    {
        poCommand->GetValue(strVal);
        nResult = SetPropertyValue(poCommand->GetPropertyPath(), strVal);
    }

    if (fep::isOk(nResult) && poCommand->IsArray())
    {
        IProperty * poProperty = GetLocalProperty(poCommand->GetPropertyPath());
        for (size_t szIndex = 1; szIndex < poCommand->GetArraySize(); ++szIndex)
        {
            if (poCommand->IsBoolean())
            {
                poCommand->GetValue(bVal, szIndex);
                nResult = poProperty->AppendValue(bVal);
            }
            else if (poCommand->IsFloat())
            {
                poCommand->GetValue(fVal, szIndex);
                nResult = poProperty->AppendValue(fVal);
            }
            else if (poCommand->IsInteger())
            {
                poCommand->GetValue(nVal, szIndex);
                nResult = poProperty->AppendValue(nVal);
            }
            else if (poCommand->IsString())
            {
                poCommand->GetValue(strVal, szIndex);
                nResult = poProperty->AppendValue(strVal);
            }
            if (isFailed(nResult))
            {
                break;
            }
        }
    }

    if (isOk(nResult))
    {
        IProperty * pProperty = GetLocalProperty(poCommand->GetPropertyPath());
        if (!pProperty)
        {
            // we can't return an error here, we simply don't send the property notification
            return ERR_NOERROR;
        }

        // create property notification
        cProperty* poProp = dynamic_cast<cProperty*>(pProperty);
        assert(poProp);
        cPropertyNotification oNot(poCommand->GetPropertyPath(), poProp,
            m_pModule->GetName(), poCommand->GetSender(), GetTimeStampMicrosecondsUTC(),
            m_pModule->GetTimingInterface()->GetTime());

        nResult = ERR_NOERROR;
        // transmit
        if (NULL != m_pModule)
        {
            m_pModule->GetNotificationAccess()->TransmitNotification(&oNot);
        }
        else
        {
            nResult = ERR_FAILED;
        }
    }

    return nResult;
}

fep::Result cPropertyTree::Update(fep::IGetPropertyCommand const * poCommand)
{
    IProperty * pProperty = GetLocalProperty(poCommand->GetPropertyPath());
    if (!pProperty)
    {
        // we can't return an error here, we simply don't send the property notification
        return ERR_NOERROR;
    }

    // create property notification
    cProperty* poProp = dynamic_cast<cProperty*>(pProperty);
    assert(poProp);
    cPropertyNotification oNot(poCommand->GetPropertyPath(), poProp,
        m_pModule->GetName(), poCommand->GetSender(), GetTimeStampMicrosecondsUTC(),
        m_pModule->GetTimingInterface()->GetTime());

    fep::Result nResult = ERR_NOERROR;
    // transmit
    if (NULL != m_pModule)
    {
        m_pModule->GetNotificationAccess()->TransmitNotification(&oNot);
    }
    else
    {
        nResult = ERR_FAILED;
    }
    return nResult;
}

fep::Result cPropertyTree::Update(fep::IDeletePropertyCommand const * poCommand)
{
    return DeleteProperty(poCommand->GetPropertyPath());
}

fep::Result cPropertyTree::Update(IRegPropListenerCommand const * poCommand)
{
    if (!m_pModule) { return ERR_POINTER; }
    // find the requested property
    IProperty * pProperty = GetLocalProperty(poCommand->GetPropertyPath());
    if (!pProperty)
    {
        // we can't return an error here, we simply don't send a notification
        return ERR_NOERROR;
    }

    // create notification
    cProperty * poProp = dynamic_cast<cProperty*>(pProperty);
    assert(poProp);
    cRegPropListenerAckNotification oNot(poCommand->GetPropertyPath(),
        poProp,
        m_pModule->GetName(), poCommand->GetSender(), GetTimeStampMicrosecondsUTC(),
        m_pModule->GetTimingInterface()->GetTime());

    {
        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLockGuard(m_oMapProxyChangeMutex);

        // check if we already have a listener for this property & module
        tProxyMap::iterator itListener = FindExistingProxyListener(pProperty, poCommand->GetSender(),
            poCommand->GetPropertyPath());
        if (itListener != m_mapProxyChangeListeners.end())
        {
            // dont create a second listener, just ref and bail (but send notification first)
            itListener->second->Ref();
            m_pModule->GetNotificationAccess()->TransmitNotification(&oNot);
            return ERR_NOERROR;
        }
    }

    // create and register proxy listener
    cProxyPropertyChangedListener * pListener =
        new cProxyPropertyChangedListener(poCommand->GetSender(),
        poCommand->GetPropertyPath(), m_pModule, pProperty);
    fep::Result nRes = pProperty->RegisterListener(pListener);
    if (fep::isFailed(nRes))
    {
        delete pListener;
        return nRes;
    }
    pListener->Ref();

    {
        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLockGuard(m_oMapProxyChangeMutex);

        // add it to the map
        m_mapProxyChangeListeners.insert(std::make_pair(pProperty, pListener));
    }

    // send notification
    m_pModule->GetNotificationAccess()->TransmitNotification(&oNot);

    return ERR_NOERROR;
}

fep::Result cPropertyTree::Update(IUnregPropListenerCommand const * poCommand)
{
    if (!m_pModule) { return ERR_POINTER; }
    // find the requested property
    IProperty * pProperty = const_cast<IProperty *>(GetLocalProperty(poCommand->GetPropertyPath()));
    if (!pProperty)
    {
        // we can't return an error here, we simply don't send a notification
        return ERR_NOERROR;
    }

    {
        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLockGuard(m_oMapProxyChangeMutex);

        // find listener
        tProxyMap::iterator itListener = FindExistingProxyListener(pProperty, poCommand->GetSender(),
            poCommand->GetPropertyPath());
        if (itListener == m_mapProxyChangeListeners.end())
        {
            return ERR_NOERROR;
        }

        // are we the last subscription using this proxy listener?
        if (!itListener->second->Unref())
        {
            // unregister & delete
            itListener->first->UnregisterListener(itListener->second);
            delete itListener->second;
            m_mapProxyChangeListeners.erase(itListener);
        }
    }

    // create notification
    cUnregPropListenerAckNotification oNot(poCommand->GetPropertyPath(),
        m_pModule->GetName(), poCommand->GetSender(), GetTimeStampMicrosecondsUTC(), m_pModule->GetTimingInterface()->GetTime());

    // transmit
    m_pModule->GetNotificationAccess()->TransmitNotification(&oNot);

    return ERR_NOERROR;
}

fep::Result fep::cPropertyTree::Update(INameChangedNotification const * pNotification)
{
    ElementNameChanged(pNotification->GetOldParticipantName(), pNotification->GetSender());
    return ERR_NOERROR;
}

void fep::cPropertyTree::DeletePropertyFromMap(IProperty* poProp)
{
    a_util::concurrency::unique_lock<a_util::concurrency::mutex>
        oLockGuard(m_oMapProxyChangeMutex);

    std::pair<tProxyMap::iterator, tProxyMap::iterator> itProxies =
        m_mapProxyChangeListeners.equal_range(poProp);
    for (tProxyMap::iterator itProxy = itProxies.first; itProxy != itProxies.second; ++itProxy)
    {
        poProp->UnregisterListener(itProxy->second);
        itProxy->second->SendFinalDelete();

        delete itProxy->second;
    }
    m_mapProxyChangeListeners.erase(itProxies.first, itProxies.second);
}

fep::Result cPropertyTree::RemoveMirroredPropertyLocalSubscription(
    IProperty * poProperty, const sMirroredRemoteProperty & poInfo)
{
    if (!m_pModule) { return ERR_POINTER; }
    // find remote change listener
    tMirroredChangeListener::iterator itListener =
        m_mapRemoteChangeListeners.find(poProperty);

    if (itListener == m_mapRemoteChangeListeners.end())
    {
        std::cout << "Doing nothing" << std::endl;
        return ERR_NOERROR;
    }

    // unregister and delete
    m_pModule->GetNotificationAccess()->UnregisterNotificationListener(itListener->second);
    delete itListener->second;
    m_mapRemoteChangeListeners.erase(itListener);

    // remove entry from map of mirrored properties
    std::map<sMirroredRemoteProperty, IProperty *>::iterator itEntry =
        m_mapMirroredProperties.find(poInfo);
    if (itEntry != m_mapMirroredProperties.end())
    {
        m_mapMirroredProperties.erase(itEntry);
    }

    return ERR_NOERROR;
}

cPropertyTree::tProxyMap::iterator cPropertyTree::FindExistingProxyListener
    (IProperty * pProperty, const char * strModule, const char * strPath)
{
    std::pair<tProxyMap::iterator, tProxyMap::iterator> itListeners =
        m_mapProxyChangeListeners.equal_range(pProperty);
    for (tProxyMap::iterator itIter = itListeners.first; itIter != itListeners.second;
        ++itIter)
    {
        if (itIter->second->m_strRemoteModule == strModule &&
            itIter->second->m_strPath == strPath)
        {
            return itIter;
        }
    }

    return m_mapProxyChangeListeners.end();
}

cPropertyTree::cProxyPropertyChangedListener::cProxyPropertyChangedListener(
    const char * strRemoteModule, const char * strPath,
    const fep::IModule * pModule, IProperty * poProperty) : m_strRemoteModule(strRemoteModule),
    m_strPath(strPath), m_pModule(pModule),
    m_poProperty(poProperty), m_nRefCount(0)
{
}

void cPropertyTree::cProxyPropertyChangedListener::Ref()
{
    m_nRefCount++;
}

bool cPropertyTree::cProxyPropertyChangedListener::Unref()
{
    m_nRefCount--;
    return m_nRefCount > 0;
}

fep::Result cPropertyTree::cProxyPropertyChangedListener::SendFinalDelete()
{
    return SendPropertyChangedNotification(m_poProperty, IPropertyChangedNotification::CE_Delete);
}

fep::Result cPropertyTree::cProxyPropertyChangedListener::
    SendPropertyChangedNotification(const IProperty * poProperty,
    IPropertyChangedNotification::tChangeEvent ceEvent)
{

    cPropertyChangedNotification oNoti(poProperty,
        ceEvent, poProperty->GetPath(), m_pModule->GetName(),
        m_strRemoteModule.c_str(), GetTimeStampMicrosecondsUTC(), m_pModule->GetTimingInterface()->GetTime());
    return m_pModule->GetNotificationAccess()->TransmitNotification(&oNoti);
}

fep::Result cPropertyTree::cProxyPropertyChangedListener::
    ProcessPropertyAdd(IProperty const * poProperty,
    IProperty const * poAffectedProperty, char const * strRelativePath)
{
    SendPropertyChangedNotification(poAffectedProperty,
        IPropertyChangedNotification::CE_New);
    return ERR_NOERROR;
}

fep::Result cPropertyTree::cProxyPropertyChangedListener::
    ProcessPropertyChange(IProperty const * poProperty,
    IProperty const * poAffectedProperty, char const * strRelativePath)
{
    SendPropertyChangedNotification(poAffectedProperty,
        IPropertyChangedNotification::CE_Change);
    return ERR_NOERROR;
}

fep::Result cPropertyTree::cProxyPropertyChangedListener::
    ProcessPropertyDelete(IProperty const * poProperty,
    IProperty const * poAffectedProperty, char const * strRelativePath)
{
    SendPropertyChangedNotification(poAffectedProperty,
        IPropertyChangedNotification::CE_Delete);
    return ERR_NOERROR;
}

cPropertyTree::cMirroredPropertyChangedNotificationListener::
    cMirroredPropertyChangedNotificationListener(const sMirroredRemoteProperty * poInfo,
    IProperty * poProperty, cPropertyTree * pTree)
    : m_poInfo(poInfo), m_poProperty(poProperty), m_pTree(pTree)
{
}

IProperty * cPropertyTree::cMirroredPropertyChangedNotificationListener::
    FindAffectedProperty(const std::string & strPath, bool bEmptyRemPath)
{
    std::string strRemotePath = strPath;
    std::string strLocalPath = m_poProperty->GetPath();
    if (strRemotePath.size() == 0)
    {
        // received property is empty: do nothing
        return NULL;
    }
    else if (strRemotePath == m_poInfo->strRemotePath)
    {
        // received property is the root of the mirrored property
        return m_poProperty;
    }
    else
    {
        // remove the remote prefix and attach the local prefix
        if (!bEmptyRemPath)
        {
            strRemotePath.erase(0,m_poInfo->strRemotePath.size()+1);
        }
        if (!strLocalPath.empty())
        {
            strLocalPath.push_back('.');
        }
        strLocalPath.append(strRemotePath);
        return m_pTree->GetLocalProperty(strLocalPath.c_str());
    }
}


fep::Result cPropertyTree::cMirroredPropertyChangedNotificationListener::
    Update(IPropertyChangedNotification const * pPropertyChangedNotification)
{
    fep::Result nResult = ERR_NOERROR;

    if (!a_util::strings::isEqual(pPropertyChangedNotification->GetSender(), m_poInfo->strElementName.c_str()))
    {
        // not the module we're listening to here
        return ERR_NOERROR;
    }

    std::string strNotiPath = pPropertyChangedNotification->GetPropertyPath();
    if (strNotiPath.find(m_poInfo->strRemotePath) != 0)
    {
        // not the property path we're listening to here
        return ERR_NOERROR;
    }

    // if its a new event, we need to remove the last property in the path, because it doesnt exist yet
    std::string strNewPropName;
    if (pPropertyChangedNotification->GetEvent() == IPropertyChangedNotification::CE_New)
    {
        size_t nIndex = strNotiPath.rfind('.');
        if (nIndex == std::string::npos)
        {
            return ERR_NOERROR;
        }

        strNewPropName = strNotiPath.substr(nIndex + 1);
        strNotiPath.erase(nIndex);
    }

    // find affected property (might be a subproperty below the subscribed property)
    IProperty * poProperty = FindAffectedProperty(strNotiPath, m_poInfo->strRemotePath.empty());
    if (!poProperty)
    {
        // hmm, maybe the affected property was deleted locally after the initial
        // subscription? we don't handle this and simply ignore any updates on that.
        return ERR_NOERROR;
    }

    // easy case, just update the value
    if (pPropertyChangedNotification->GetEvent() == IPropertyChangedNotification::CE_Change)
    {
        cProperty::CopyPropertyValue(pPropertyChangedNotification->GetProperty(), poProperty);
    }
    else if (pPropertyChangedNotification->GetEvent() == IPropertyChangedNotification::CE_Delete)
    {
        if (poProperty == m_poProperty)
        {
            sMirroredRemoteProperty oProp;
            oProp.strElementName = pPropertyChangedNotification->GetSender();
            oProp.strLocalPath = poProperty->GetParent()->GetPath();
            oProp.strRemotePath = pPropertyChangedNotification->GetPropertyPath();
            std::string strCompletePath = poProperty->GetPath();
            // neither the local nor the remote path contain the elements name
            // ergo, we must not shorten the strings

            // remove subscription and delete local copy
            // RemoveMirroredPropertyLocalSubscription invalidates m_pTree, hence delete first
            nResult |= m_pTree->DeleteProperty(strCompletePath.c_str());
            nResult |= m_pTree->RemoveMirroredPropertyLocalSubscription(m_poProperty, oProp);
        }
        else
        {
            // it's a subproperty
            IProperty * poParent = const_cast<IProperty *>(poProperty->GetParent());
            nResult |= poParent->DeleteSubproperty(poProperty->GetName());
        }
    }
    else if (pPropertyChangedNotification->GetEvent() == IPropertyChangedNotification::CE_New)
    {
        nResult |= poProperty->SetSubproperty(strNewPropName.c_str(), "");
        nResult |= cProperty::CopyPropertyValue(pPropertyChangedNotification->GetProperty(),
            poProperty->GetSubproperty(strNewPropName.c_str()));
    }
    return nResult;
}

fep::Result fep::cPropertyTree::cMirroredPropertyChangedNotificationListener::LockUpdates()
{
    m_mtxUpdates.lock();
    return ERR_NOERROR;
}

fep::Result fep::cPropertyTree::cMirroredPropertyChangedNotificationListener::UnlockUpdates()
{
    m_mtxUpdates.unlock();
    return ERR_NOERROR;
}

namespace fep
{
    namespace detail
    {
        std::string getValueAsString(const IProperty* prop)
        {
            if (prop->IsArray())
            {
                std::string array_value;
                bool first = true;
                for (size_t idx = 0;
                    idx < prop->GetArraySize();
                    idx++)
                {
                    if (first)
                    {
                        first = false;
                    }
                    else
                    {
                        array_value += ";";
                    }
                    if (prop->IsBoolean())
                    {
                        bool val;
                        prop->GetValue(val, idx);
                        array_value += a_util::strings::toString(val);
                    }
                    else if (prop->IsFloat())
                    {
                        double val;
                        prop->GetValue(val, idx);
                        array_value += a_util::strings::toString(val);
                    }
                    else if (prop->IsInteger())
                    {
                        int32_t val;
                        prop->GetValue(val, idx);
                        array_value += a_util::strings::toString(val);
                    }
                    else if (prop->IsString())
                    {
                        const char* val;
                        prop->GetValue(val, idx);
                        std::string returnval = val;
                        array_value += returnval;
                    }
                }
                return array_value;
            }
            else
            {
                if (prop->IsBoolean())
                {
                    bool val;
                    prop->GetValue(val);
                    return a_util::strings::toString(val);
                }
                else if (prop->IsFloat())
                {
                    double val;
                    prop->GetValue(val);
                    return a_util::strings::toString(val);
                }
                else if (prop->IsInteger())
                {
                    int32_t val;
                    prop->GetValue(val);
                    return a_util::strings::toString(val);
                }
                else if (prop->IsString())
                {
                    const char* val;
                    prop->GetValue(val);
                    std::string returnval = val;
                    return returnval;
                }
                return "";
            }
        }
        std::string getTypeAsString(const IProperty* prop)
        {
            std::string retval = "";
            if (prop->IsArray())
            {
                retval = "array-";
            }
            if (prop->IsBoolean())
            {
                retval = retval + fep::PropertyType<bool>::getTypeName();
            }
            else if (prop->IsFloat())
            {
                retval = retval + fep::PropertyType<double>::getTypeName();
            }
            else if (prop->IsInteger())
            {
                retval = retval + fep::PropertyType<int32_t>::getTypeName();
            }
            else if (prop->IsString())
            {
                retval = retval + fep::PropertyType<std::string>::getTypeName();
            }
            return retval;
        }

        RPCConfigServer::RPCConfigServer(cPropertyTree& tree) : _tree(tree)
        {
        }
        void RPCConfigServer::normalizePathForTree(std::string& internal_property_path)
        {
            if (internal_property_path == "/" || internal_property_path.empty())
            {
                internal_property_path = "";
            }
            else if (internal_property_path.at(0) == '/')
            {
                internal_property_path = internal_property_path.substr(1, internal_property_path.size() - 1);
            }
            a_util::strings::replace(internal_property_path, "/", ".");
        }
        std::string RPCConfigServer::getProperties(const std::string& property_path)
        {
            std::string replaced_path(property_path);
            normalizePathForTree(replaced_path);
            auto prop = _tree.GetLocalProperty(replaced_path.c_str());
            if (prop)
            {
                bool first = true;
                std::string retval;
                auto list_sub_props = prop->GetSubProperties();
                for (const auto& sub_prop : list_sub_props)
                {
                    if (!first)
                    {
                        retval += ",";
                    }
                    retval += std::string(sub_prop->GetName());
                    first = false;
                }
                return retval;
            }
            else
            {
                return std::string();
            }
        }

        bool resetProperty(IProperty* prop, 
                           const std::string& value,
                           const std::string& type)
        {
            bool valueset = false;
            if (type.size() > 6
                && type.substr(0, 6) == "array-")
            {
                auto value_vec = a_util::strings::split(value, ";");
                bool first = true;
                for (auto& current_val : value_vec)
                {
                    if (type == fep::PropertyType<std::vector<bool>>::getTypeName())
                    {
                        if (first)
                        {
                            prop->SetValue(a_util::strings::toBool(current_val));
                        }
                        else
                        {
                            prop->AppendValue(a_util::strings::toBool(current_val));
                        }
                        valueset = true;
                    }
                    else if (type == fep::PropertyType<std::vector<int32_t>>::getTypeName())
                    {
                        if (first)
                        {
                            prop->SetValue(a_util::strings::toInt32(current_val));
                        }
                        else
                        {
                            prop->AppendValue(a_util::strings::toInt32(current_val));
                        }
                        valueset = true;
                    }
                    else if (type == fep::PropertyType<std::vector<double>>::getTypeName())
                    {
                        if (first)
                        {
                            prop->SetValue(a_util::strings::toDouble(current_val));
                        }
                        else
                        {
                            prop->AppendValue(a_util::strings::toDouble(current_val));
                        }
                        valueset = true;
                    }
                    else if (type == fep::PropertyType<std::vector<std::string>>::getTypeName())
                    {
                        if (first)
                        {
                            prop->SetValue(current_val.c_str());
                        }
                        else
                        {
                            prop->AppendValue(current_val.c_str());
                        }
                        valueset = true;
                    }
                    first = false;
                }
            }
            else
            {
                if (type == fep::PropertyType<bool>::getTypeName())
                {
                    prop->SetValue(a_util::strings::toBool(value));
                    valueset = true;
                }
                else if (type == fep::PropertyType<int32_t>::getTypeName())
                {
                    prop->SetValue(a_util::strings::toInt32(value));
                    valueset = true;
                }
                else if (type == fep::PropertyType<double>::getTypeName())
                {
                    prop->SetValue(a_util::strings::toDouble(value));
                    valueset = true;
                }
                else if (type == fep::PropertyType<std::string>::getTypeName())
                {
                    prop->SetValue(value.c_str());
                    valueset = true;
                }
            }
            return valueset;
        }

        Json::Value RPCConfigServer::getProperty(const std::string& property_path)
        {
            Json::Value retval;
            retval["value"] = "";
            retval["type"] = "";
            if (property_path.empty() || property_path == "/")
            {
                retval["value"] =  fep::DefaultPropertyTypeConversion<bool>::toString(true);
                retval["type"] = fep::PropertyType<bool>::getTypeName();
                return retval;
            }

            std::string replaced_path(property_path);
            normalizePathForTree(replaced_path);
            auto prop = _tree.GetLocalProperty(replaced_path.c_str());
            if (prop)
            {
                retval["value"] = getValueAsString(prop);
                retval["type"] = getTypeAsString(prop);
                return retval;
            }
            else
            {
                return retval;
            }
        }
        int RPCConfigServer::setProperty(const std::string& property_path,
                                         const std::string& type,
                                         const std::string& value)
        {
            if (property_path.empty() || property_path == "/")
            {
                return fep::ResultType_ERR_ACCESS_DENIED::getCode();
            }
            else
            {
                std::string replaced_path(property_path);
                normalizePathForTree(replaced_path);
                auto prop = _tree.GetLocalProperty(replaced_path.c_str());
                if (prop)
                {
                    if (!resetProperty(prop, value, type))
                    {
                        return fep::ResultType_ERR_INVALID_TYPE::getCode();
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    return fep::ResultType_ERR_INVALID_ADDRESS::getCode();
                }
            }
        }
        bool RPCConfigServer::exists(const std::string& property_path)
        {
            if (property_path.empty() || property_path == "/")
            {
                return true;
            }
            else
            {
                std::string replaced_path(property_path);
                normalizePathForTree(replaced_path);
                auto prop = _tree.GetLocalProperty(replaced_path.c_str());
                if (prop)
                {

                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }
}
