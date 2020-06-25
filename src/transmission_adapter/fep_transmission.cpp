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
/// Someone should add a header here some time

#include <algorithm>
#include <cstring>
#include <sys/types.h>
#include <a_util/memory/memory.h>
#include <a_util/regex/regularexpression.h>
#include <a_util/strings/strings_format.h>
#include <a_util/strings/strings_functions.h>
#include <a_util/system/system.h>
#ifdef __QNX__
#include <a_util/process/process.h>
#endif

#include "_common/fep_stringlist_intf.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/fep_module_header_config.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep_transmission_adapter_common.h"
#include "fep_sdk_participant_version.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler.h"
#include "messages/fep_command_control.h"
#include "messages/fep_command_custom.h"
#include "messages/fep_command_delete_property.h"
#include "messages/fep_command_get_property.h"
#include "messages/fep_command_get_schedule.h"
#include "messages/fep_command_get_signal_info.h"
#include "messages/fep_command_intf.h"
#include "messages/fep_command_mapping_configuration.h"
#include "messages/fep_command_mute_signal.h"
#include "messages/fep_command_name_change.h"
#include "messages/fep_command_reg_prop_listener.h"
#include "messages/fep_command_resolve_signal_type.h"
#include "messages/fep_command_rpc.h"
#include "messages/fep_command_set_property.h"
#include "messages/fep_command_signal_description.h"
#include "messages/fep_command_unreg_prop_listener.h"
#include "messages/fep_message.h"
#include "messages/fep_message_intf.h"
#include "messages/fep_notification_incident.h"
#include "messages/fep_notification_intf.h"
#include "messages/fep_notification_name_changed.h"
#include "messages/fep_notification_prop_changed.h"
#include "messages/fep_notification_property.h"
#include "messages/fep_notification_reg_prop_listener_ack.h"
#include "messages/fep_notification_resultcode.h"
#include "messages/fep_notification_schedule.h"
#include "messages/fep_notification_signal_description.h"
#include "messages/fep_notification_signal_info.h"
#include "messages/fep_notification_state.h"
#include "messages/fep_notification_unreg_prop_listener_ack.h"
#include "signal_registry/fep_signal_struct.h"
#include "transmission_adapter/RTI_DDS/fep_dds_driver.h"
#include "transmission_adapter/fep_preparation_data_sample_intf.h"
#include "transmission_adapter/fep_receive_intf.h"
#include "transmission_adapter/fep_receiver.h"
#include "transmission_adapter/fep_signal_direction.h"
#include "transmission_adapter/fep_transmission.h"
#include "transmission_adapter/fep_transmission_driver_intf.h"
#include "transmission_adapter/fep_transmission_type.h"
#include "transmission_adapter/fep_transmit_intf.h"
#include "transmission_adapter/fep_transmitter.h"

#ifdef WITH_ZYRE
#include "transmission_adapter/zmq/fep_zmq_driver.h"
#endif

#if __GNUC__
// Avoid lots of warnings in libjson
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#endif
#include <libjson.h>
#if __GNUC__
// Restore previous behaviour
#pragma GCC diagnostic pop
#endif

const uint32_t fep::cTransmissionAdapter::s_nMessageStringLength = 2097152;

/* constants used in this module only */
static uint32_t s_nPreAllocCount = 100;
static const int32_t s_nNumberOfWorkers = 4;

namespace fep
{
    class INotificationListener;
    class IPreparationDataListener;
    namespace detail
    {
        struct cMessageContainer
        {
            size_t szSize;
            char strMessage[fep::cTransmissionAdapter::s_nMessageStringLength];
        };
    }

using namespace detail;

cTransmissionAdapter::cTransmissionAdapter() :
    m_poTransmissionDriver(NULL),
    m_pMessageTransmitter(NULL),
    m_pMessageReceiver(NULL),
    m_bGlobalDisabled(true),
    m_pPropertyTree(NULL),
    m_pIncidentInvocationHandler(NULL),
    m_bInternalDriver(true),
    m_bInitialized(false)
{
}
cTransmissionAdapter::~cTransmissionAdapter()
{
    Destroy();
}

fep::Result cTransmissionAdapter::Setup(fep::IPropertyTree* pPropertyTree,
    fep::IIncidentInvocationHandler* pIncidentInvocationHandler,
    const cModuleOptions oModuleOptions,
    ITransmissionDriver* pDriver)
{
    if (!pPropertyTree || !pIncidentInvocationHandler ) 
    { 
        return ERR_POINTER;
    }
    tMutexLockGuard oAdapterGuard(m_oAdapterMutex);
    tMutexLockGuard oContForkGuard(m_oContForkMutex);

    fep::Result nResult = ERR_NOERROR;

    m_pPropertyTree = pPropertyTree;
    m_pIncidentInvocationHandler = pIncidentInvocationHandler;

    m_oModuleOptions = oModuleOptions;
    nResult = m_pPropertyTree->SetPropertyValue(
        fep::component_config::g_strTxAdapterPath_nNumberOfWorkerThreads, s_nNumberOfWorkers);
    //Select Driver 
    if(fep::isOk(nResult))
    {
        if(NULL != pDriver)
        {
            m_poTransmissionDriver = pDriver;
            m_bInternalDriver = false;
        }
        else
        {
            if (static_cast<int>(oModuleOptions.GetTransmissionType()) == 1)
            {
                // Previous value for: fep::TT_ConnLib:
                nResult = ERR_NOT_SUPPORTED;
            }
            else
            {
                switch (oModuleOptions.GetTransmissionType())
                {
                case fep::TT_RTI_DDS:
                {
                    uint64_t nSenderID = std::hash<std::string>()(std::string(oModuleOptions.GetParticipantName()));
                    m_poTransmissionDriver = new RTI_DDS::cDDSDriver(nSenderID);
                    if (NULL == m_poTransmissionDriver)
                    {
                        nResult = ERR_MEMORY;
                    }
                    break;
                }

#ifdef WITH_ZYRE
                case fep::TT_ZMQ:
                {
                    m_poTransmissionDriver = new zmq::cZMQDriver();
                    if (NULL == m_poTransmissionDriver)
                    {
                        nResult = ERR_MEMORY;
                    }
                    break;
                }
#endif
                default:
                {
                    nResult = ERR_INVALID_ARG;
                }
                }
            }
        }
        //Setup Driver
        if(fep::isOk(nResult))
        {
            nResult = GatherDriverOptions();
            if(fep::isOk(nResult))
            {
                nResult = m_poTransmissionDriver->Initialize(m_oDriverOptions);
                if(fep::isOk(nResult))
                {
                    nResult = m_poTransmissionDriver->RegisterLogging(cTransmissionAdapter::LogMessage, this);
                }
                if(fep::isOk(nResult))
                {
                    nResult = CreateMessageChannel();
                }
                if(fep::isOk(nResult))
                {
                    m_bInitialized = true;
                }
            }  
        }
    }
    return nResult;
}

fep::Result cTransmissionAdapter::GatherDriverOptions()
{
    fep::Result nResult= ERR_NOERROR;
    nResult = m_oOptionsFactory.Initialize(m_poTransmissionDriver);
    if(fep::isOk(nResult))
    {
        m_oDriverOptions = m_oOptionsFactory.GetDriverOptions();

        if(!m_oDriverOptions.SetOption("DomainID",static_cast<int>(m_oModuleOptions.GetDomainId())))
        {
            INVOKE_INCIDENT(m_pIncidentInvocationHandler, FSI_GENERAL_WARNING, SL_Warning,
                a_util::strings::format("The transmission driver has no Option %s", "DomainID").c_str())
        }
        if(!m_oDriverOptions.SetOption("ModuleName",m_oModuleOptions.GetParticipantName()))
        {
            INVOKE_INCIDENT(m_pIncidentInvocationHandler, FSI_GENERAL_WARNING, SL_Warning,
                a_util::strings::format("The transmission driver has no Option %s", "ModuleName").c_str())
        }

        //Get comma seperated list of allowed interfaces
        std::string strAllowInterfaces = GetNetworkInterfaceList();
        if (!strAllowInterfaces.empty())
        {
            if (!m_oDriverOptions.SetOption("AllowedInterfaces", strAllowInterfaces))
            {
                INVOKE_INCIDENT(m_pIncidentInvocationHandler, FSI_GENERAL_WARNING, SL_Warning,
                    a_util::strings::format("The transmission driver has no Option %s", "AllowedInterfaces").c_str())
            }
        }
    }
    return nResult;
}

std::string cTransmissionAdapter::GetNetworkInterfaceList()
{
    IStringList* pStringList = NULL;
    std::string strAllowInterfaces = "";
    if (fep::isOk(m_oModuleOptions.GetNetworkInterfaceList(pStringList)))
    {
        if (NULL != pStringList)
        {
            if (0 != pStringList->GetListSize())
            { 
                for (size_t i = 0; i< pStringList->GetListSize(); ++i)
                {
                    const char* strInterfaceStr = pStringList->GetStringAt(i);
                    if (strInterfaceStr)
                    {
                        if (!strAllowInterfaces.empty())
                        {
                            strAllowInterfaces.append(",");
                        }
                        strAllowInterfaces.append(strInterfaceStr);
                    }
                }
            }
            delete pStringList;
        }
    }
    return strAllowInterfaces;
}

fep::Result cTransmissionAdapter::CreateMessageChannel()
{
    fep::Result nResult = GatherMessageSignalOptions();
    if(fep::isOk(nResult))
    {
        nResult = m_poTransmissionDriver->CreateReceiver(m_pMessageReceiver, m_oMessageSignalOptions);

        if(fep::isOk(nResult))
        {
            m_pMessageReceiver->Enable();
            nResult = m_poTransmissionDriver->CreateTransmitter(m_pMessageTransmitter, m_oMessageSignalOptions);
            if (fep::isOk(nResult))
            {
                m_pMessageTransmitter->Enable();
            }
        }
    }
    else
    {
        nResult = ERR_FAILED;
    }
    if(fep::isOk(nResult))
    {
        // preallocating message buffer
        for (uint32_t i = 0; i < s_nPreAllocCount; ++i)
        {
            cMessageContainer* pAlloc = new cMessageContainer();
            m_qPreAllocQueue.Enqueue(pAlloc);
        }
        nResult = m_pMessageReceiver->SetReceiver(ReceiveMessage, this);
    }
    //Giving the participants time to find each other
    a_util::system::sleepMilliseconds(500);
    // Creating message worker thread
    if(fep::isOk(nResult))
    {
        m_pMessageThread.reset(new std::thread(&cTransmissionAdapter::ThreadFunc, this));
    }
    return nResult;
}

fep::Result cTransmissionAdapter::DestroyMessageChannel()
{
    fep::Result nResult = ERR_NOERROR;
    m_vecNotificationListeners.clear();
    m_vecCommandListeners.clear();
    // Prevent any undesired callbacks when the transmission adapter goes down (asynchronously)
    // and some components missed to unregister before for any reason.
    m_oShutdownSignal.notify();
    if(m_pMessageThread)
    {
        m_pMessageThread->join();
        m_pMessageThread.reset();
    }
    m_oShutdownSignal.reset();
    nResult = m_poTransmissionDriver->DestroyReceiver(m_pMessageReceiver);
    nResult = m_poTransmissionDriver->DestroyTransmitter(m_pMessageTransmitter);

    cMessageContainer* pTmp;

    while(m_qReceiveQueue.TryDequeue(pTmp,1))
    {
        delete pTmp;
    }
    while (m_qPreAllocQueue.TryDequeue(pTmp,1))
    {
        delete pTmp;
    }
    return nResult;
}

fep::Result cTransmissionAdapter::GatherMessageSignalOptions()
{
    fep::Result nResult = ERR_NOERROR;
    m_oMessageSignalOptions = m_oOptionsFactory.GetSignalOptions();

    if(!m_oMessageSignalOptions.SetOption("IsReliable", true))
    {
        INVOKE_INCIDENT(m_pIncidentInvocationHandler, FSI_GENERAL_WARNING, SL_Warning,
            "The transmission driver can not transmit reliable. Creating message channel anyway!\n")
    }
    std::string strSignalName ="command";

    if(!m_oMessageSignalOptions.SetOption("SignalName", strSignalName))
    {
        INVOKE_INCIDENT(m_pIncidentInvocationHandler, FSI_GENERAL_WARNING, SL_Critical_Local,
            "The transmission driver does not support signalnames. This is fatal!\n")
            nResult = ERR_FAILED;
    }
    if(!m_oMessageSignalOptions.SetOption("SignalSize", static_cast<size_t>(s_nMessageStringLength)))
    {
        INVOKE_INCIDENT(m_pIncidentInvocationHandler, FSI_GENERAL_WARNING, SL_Critical_Local,
            "The transmission driver does not support setting signal size to message size. This is fatal!\n")
            nResult = ERR_FAILED;
    }
    m_oMessageSignalOptions.SetOption("IsVariableSignalSize",true);

#ifdef __QNX__
    // We need to be able to change the number of preallocated message buffers...
    std::string sMessageBuffers = a_util::process::getEnvVar("FEP_MESSAGE_BUFFERS", "");
    if (!sMessageBuffers.empty())
    {
        uint32_t nMessageBuffers = a_util::strings::toUInt32(sMessageBuffers);
        s_nPreAllocCount = nMessageBuffers;
    }
#endif
    return nResult;
}

fep::Result cTransmissionAdapter::CreateQueueManager()
{
    int32_t nNumberOfThreads;
    fep::Result nResult = m_pPropertyTree->GetPropertyValue(fep::component_config::g_strTxAdapterPath_nNumberOfWorkerThreads, nNumberOfThreads);
    if(fep::isFailed(nResult))
    {
        nNumberOfThreads = s_nNumberOfWorkers;
    }
    nResult = m_oQueueManager.Create(nNumberOfThreads);
    return nResult;
}

fep::Result cTransmissionAdapter::DestroyQueueManager()
{
    return m_oQueueManager.Destroy();
}

fep::Result cTransmissionAdapter::Destroy()
{
    fep::Result nResult = ERR_NOERROR;
    tMutexLockGuard oAdapterGuard(m_oAdapterMutex);
    tMutexLockGuard oContForkGuard(m_oContForkMutex);
    if (m_bInitialized)
    {
        m_bInitialized = false;
        //Destroy Channel will wait a looooooooooooooooooooong time (this means forever!)
        //   if a message is received (ProvideMessage Call) between the above MutexLock of m_oAdapterMutex 
        //   and this DestroyMessageChannel Call
        //we need to unlock before and lock again after that (need to unlock both, beceuse otherwise mybe another deadlock will occure)
        m_oContForkMutex.unlock();
        m_oAdapterMutex.unlock();
        
        nResult = DestroyMessageChannel();

        m_oAdapterMutex.lock();
        m_oContForkMutex.lock();

        std::vector<cDataReceiver*>::iterator itReceivers = m_vecDataReceiver.begin();
        for (; itReceivers != m_vecDataReceiver.end(); ++itReceivers)
        {
            delete (*itReceivers);
        }
        m_vecDataReceiver.clear();

        std::vector<cTransmitter*>::iterator itTransmitters = m_vecDataTransmitter.begin();
        for (; itTransmitters != m_vecDataTransmitter.end(); ++itTransmitters)
        {
            delete (*itTransmitters);
        }
        m_vecDataTransmitter.clear();

        fep::Result nLocalRes = m_poTransmissionDriver->Deinitialize();
        if(fep::isFailed(nLocalRes))
        {
            nResult = nLocalRes;
        }
        if(m_bInternalDriver)
        {
            delete m_poTransmissionDriver;
        }
    }
    return nResult;
}

fep::Result cTransmissionAdapter::Enable()
{   
    fep::Result nResult = ERR_NOERROR;
    if(m_bGlobalDisabled)
    {
        m_bGlobalDisabled = false;
        nResult = CreateQueueManager();
        std::vector<cDataReceiver*>::iterator itReceivers = m_vecDataReceiver.begin();
        for(; itReceivers != m_vecDataReceiver.end(); ++itReceivers)
        {
            if(fep::isFailed((*itReceivers)->Enable()))
            {
                nResult = ERR_FAILED;
            }
        }
        std::vector<cTransmitter*>::iterator itTransmitter = m_vecDataTransmitter.begin();
        for(; itTransmitter != m_vecDataTransmitter.end(); ++itTransmitter)
        {
            if(fep::isFailed((*itTransmitter)->Enable()))
            {
                nResult = ERR_FAILED;
            }
        }
    }
    return nResult;
}

fep::Result cTransmissionAdapter::Disable()
{
    fep::Result nResult = ERR_NOERROR;
    if(!m_bGlobalDisabled)
    {
        m_bGlobalDisabled = true;
        nResult = DestroyQueueManager();
        std::vector<cDataReceiver*>::iterator itReceivers = m_vecDataReceiver.begin();
        for(; itReceivers != m_vecDataReceiver.end(); ++itReceivers)
        {
            if(fep::isFailed((*itReceivers)->Disable()))
            {
                nResult = ERR_FAILED;
            }
            if(fep::isOk(nResult))
            {
                if(fep::isFailed((*itReceivers)->FlushQueue()))
                {
                    nResult = ERR_FAILED;
                }
            }
        }
        std::vector<cTransmitter*>::iterator itTransmitter = m_vecDataTransmitter.begin();
        for(; itTransmitter != m_vecDataTransmitter.end(); ++itTransmitter)
        {
            if(fep::isFailed((*itTransmitter)->Disable()))
            {
                nResult = ERR_FAILED;
            }
        }
    }
    return nResult;
}

void cTransmissionAdapter::ReceiveMessage(void* pInstance, const void* pMessage, size_t szSize)
{
    cTransmissionAdapter* pTA = reinterpret_cast<cTransmissionAdapter*>(pInstance);
    cMessageContainer* pMsgContainer;
    if (pTA->m_qPreAllocQueue.TryDequeue(pMsgContainer))
    {
        pMsgContainer->szSize = szSize;
        if(a_util::memory::copy(pMsgContainer->strMessage, sizeof(pMsgContainer->strMessage), pMessage, szSize))
        {
            pTA->m_qReceiveQueue.Enqueue(pMsgContainer);
        }
    }
    else
    {
        // need to do something here
    }
}

void cTransmissionAdapter::LogMessage(void* pInstance, const char* strMessage, const tSeverityLevel severity)
{
    if(NULL == strMessage || strlen(strMessage))
    {
        cTransmissionAdapter* pAdapter = reinterpret_cast<cTransmissionAdapter*>(pInstance);
        std::string strIncidentMessage = a_util::strings::format("%s - Driver Issue: %s", pAdapter->GetModuleName(), strMessage);
        INVOKE_INCIDENT(pAdapter->m_pIncidentInvocationHandler, FSI_DRIVER_ISSUE, severity, strIncidentMessage.c_str());
    }
}

void cTransmissionAdapter::ThreadFunc()
{
    while (!m_oShutdownSignal.is_set())
    {
        cMessageContainer* pMessageItem;
        if(true == m_qReceiveQueue.TryDequeue(pMessageItem, (100 * 1000)))
        {
            Update(static_cast<char *>(pMessageItem->strMessage));
            m_qPreAllocQueue.Enqueue(pMessageItem);
        }
    }
}

fep::Result cTransmissionAdapter::RegisterCommandListener(ICommandListener * const poCommandListener)
{
    tMutexLockGuard oAdapterGuard(m_oAdapterMutex);
    tMutexLockGuard oContForkGuard(m_oContForkMutex);

    fep::Result nResult = ERR_NOERROR;
    if (NULL == poCommandListener)
    {
        nResult = ERR_POINTER;
    }
    else
    {
        // ensure uniqueness of registered listeners
        tCommandListeners::const_iterator itFound = std::find(m_vecCommandListeners.begin(),
            m_vecCommandListeners.end(), poCommandListener);
        if (m_vecCommandListeners.end() == itFound)
        {
            m_vecCommandListeners.push_back(poCommandListener);
        }
        else
        {
            // we do not expect duplicated listeners!
            nResult = ERR_UNEXPECTED;
        }
    }

    return nResult;
}

fep::Result cTransmissionAdapter::RegisterNotificationListener(INotificationListener * poStatusListener)
{
    tMutexLockGuard oAdapterGuard(m_oAdapterMutex);
    tMutexLockGuard oContForkGuard(m_oContForkMutex);

    fep::Result nResult = ERR_NOERROR;


    if (NULL == poStatusListener)
    {
        nResult = ERR_POINTER;
    }
    else
    {
        // ensure uniqueness of registered listeners
        tNotificationListeners::const_iterator itFound = std::find(m_vecNotificationListeners.begin(),
            m_vecNotificationListeners.end(), poStatusListener);
        if (m_vecNotificationListeners.end() == itFound)
        {
            m_vecNotificationListeners.push_back(poStatusListener);
        }
        else
        {
            // we do not expect duplicated listeners!
            nResult = ERR_UNEXPECTED;
        }
    }
    return nResult;
}


fep::Result cTransmissionAdapter::UnregisterCommandListener(ICommandListener * poCommandListener)
{
    tMutexLockGuard oAdapterGuard(m_oAdapterMutex);
    tMutexLockGuard oContForkGuard(m_oContForkMutex);

    fep::Result nResult = ERR_NOERROR;
    if (NULL == poCommandListener)
    {
        nResult = ERR_POINTER;
    }
    else
    {
        tCommandListeners::iterator itListener = std::find(m_vecCommandListeners.begin(),
            m_vecCommandListeners.end(), poCommandListener);
        if (m_vecCommandListeners.end() == itListener)
        {
            nResult = ERR_NOT_FOUND;
        }
        else
        {
            m_vecCommandListeners.erase(itListener);
        }
    }
    return nResult;
}

fep::Result cTransmissionAdapter::Update(char const * strMessage)
{
    // Check: are we a receiver or sender?
    fep::Result nResult = ERR_NOERROR;
    cMessage oMessage(strMessage);
    uint8_t nMajorVer = oMessage.GetMajorVersion();
    std::string strReceiver = oMessage.GetReceiver();
    a_util::strings::replace(strReceiver, "*", ".*");
    a_util::strings::replace(strReceiver, "?", ".");
    a_util::regex::RegularExpression oReceiverFilter(strReceiver);
    const char* strModuleName = GetModuleName();
    if (!strModuleName)
    {
        nResult = ERR_UNEXPECTED;
    }

    if (isOk(nResult))
    {
        if (oReceiverFilter.fullMatch(strModuleName) && !a_util::strings::isEqual(strModuleName, oMessage.GetSender()))
        {
            if ((FEP_SDK_PARTICIPANT_VERSION_MAJOR == nMajorVer))
            {
                // Identify the type of the message
                JSONNode oMessageNode = libjson::parse(std::string(strMessage));
                JSONNode::iterator oCommandNodeIter = oMessageNode.find("Command");
                if (oMessageNode.end() != oCommandNodeIter)
                {
                    // It's a command
                    JSONNode::iterator oTypeNodeIter = oCommandNodeIter->find("Type");
                    if (oCommandNodeIter->end() != oTypeNodeIter)
                    {
                        std::string strCmd(oTypeNodeIter->as_string());
                        if (a_util::strings::isEqual(strCmd.c_str(), "set_property"))
                        {
                            // It's a set property command
                            cSetPropertyCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                        else if (a_util::strings::isEqual(strCmd.c_str(), "get_property"))
                        {
                            // It's a get property command
                            cGetPropertyCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                        else if (a_util::strings::isEqual(strCmd.c_str(), "delete_property"))
                        {
                            // It's a delete property command
                            cDeletePropertyCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                        else if (a_util::strings::isEqual(strCmd.c_str(), "control"))
                        {
                            // It's a control command
                            cControlCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                        else if (a_util::strings::isEqual(strCmd.c_str(), "reg_prop_listener"))
                        {
                            // It's a register property listener command
                            cRegPropListenerCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                        else if (a_util::strings::isEqual(strCmd.c_str(), "unreg_prop_listener"))
                        {
                            // It's an unregister property listener command
                            cUnregPropListenerCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                        else if (a_util::strings::isEqual(strCmd.c_str(), "signal_info"))
                        {
                            // It's a get signal information command
                            cGetSignalInfoCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                        else if (a_util::strings::isEqual(strCmd.c_str(), "resolve_signal_description"))
                        {
                            // It's a resolve signal description command
                            cResolveSignalTypeCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                        else if (a_util::strings::isEqual(strCmd.c_str(), "signal_description"))
                        {
                            // It's a signal description command
                            cSignalDescriptionCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                        else if (a_util::strings::isEqual(strCmd.c_str(), "mapping_configuration"))
                        {
                            // It's a mapping configuration command
                            cMappingConfigurationCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                        else if (a_util::strings::isEqual(strCmd.c_str(), "name_change"))
                        {
                            // It's a name change command
                            cNameChangeCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                        else if (a_util::strings::isEqual(strCmd.c_str(), "mute_signal"))
                        {
                            // It's a mute signal command
                            cMuteSignalCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                        else if (a_util::strings::isEqual(strCmd.c_str(), "get_schedule"))
                        {
                            // It's a get schedule command
                            cGetScheduleCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                        else if (a_util::strings::isEqual(strCmd.c_str(), "rpc"))
                        {
                            // It's a RPC command
                            cRPCCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                        else
                        {
                            // It's a custom command
                            cCustomCommand oCmd(strMessage);
                            nResult = ProvideCommand(&oCmd);
                        }
                    }
                }
                else
                {
                    JSONNode::iterator oNotificationIter = oMessageNode.find("Notification");
                    if (oMessageNode.end() != oNotificationIter)
                    {
                        JSONNode::iterator oTypeNodeIter = oNotificationIter->find("Type");
                        if (oNotificationIter->end() != oTypeNodeIter)
                        {
                            std::string strNot(oTypeNodeIter->as_string());
                            if (a_util::strings::isEqual(strNot.c_str(), "log"))
                            {
                                // It's a log notification
                                cIncidentNotification oNoti(strMessage);
                                nResult = ProvideNotification(&oNoti);
                            }
                            else if (a_util::strings::isEqual(strNot.c_str(), "state"))
                            {
                                // It's a state notification
                                cStateNotification oNoti(strMessage);
                                nResult = ProvideNotification(&oNoti);
                            }
                            else if (a_util::strings::isEqual(strNot.c_str(), "property"))
                            {
                                // It's a property notification
                                cPropertyNotification oNoti(strMessage);
                                nResult = ProvideNotification(&oNoti);
                            }
                            else if (a_util::strings::isEqual(strNot.c_str(), "property_changed"))
                            {
                                // It's a property changed notification
                                cPropertyChangedNotification oNoti(strMessage);
                                nResult = ProvideNotification(&oNoti);
                            }
                            else if (a_util::strings::isEqual(strNot.c_str(), "name_changed"))
                            {
                                // It's a name changed notification
                                cNameChangedNotification oNoti(strMessage);
                                nResult = ProvideNotification(&oNoti);
                            }
                            else if (a_util::strings::isEqual(strNot.c_str(), "reg_prop_listener_ack"))
                            {
                                // It's a reg prop listener ack notification
                                cRegPropListenerAckNotification oNoti(strMessage);
                                nResult = ProvideNotification(&oNoti);
                            }
                            else if (a_util::strings::isEqual(strNot.c_str(), "unreg_prop_listener_ack"))
                            {
                                // It's an unreg prop listener ack notification
                                cUnregPropListenerAckNotification oNoti(strMessage);
                                nResult = ProvideNotification(&oNoti);
                            }
                            else if (a_util::strings::isEqual(strNot.c_str(), "signal_info"))
                            {
                                /* it's a signal information notification */
                                cSignalInfoNotification oNoti(strMessage);
                                nResult = ProvideNotification(&oNoti);
                            }
                            else if (a_util::strings::isEqual(strNot.c_str(), "signal_description"))
                            {
                                // It's a resolve signal description notification
                                cSignalDescriptionNotification oNoti(strMessage);
                                nResult = ProvideNotification(&oNoti);
                            }
                            else if (a_util::strings::isEqual(strNot.c_str(), "result_code"))
                            {
                                /* it's a result code notification */
                                cResultCodeNotification oNoti(strMessage);
                                nResult = ProvideNotification(&oNoti);
                            }
                            else if (a_util::strings::isEqual(strNot.c_str(), "schedule"))
                            {
                                /* it's a schedule notification */
                                cScheduleNotification oNoti(strMessage);
                                nResult = ProvideNotification(&oNoti);
                        }
                    }
                    }
                    else
                    {
                        // What is it???
                    }
                }
            }
            else
            {
                INVOKE_INCIDENT(m_pIncidentInvocationHandler, FSI_TRANSM_MESSAGE_MAJOR_VERSION_FAILED, fep::SL_Warning,
                    a_util::strings::format(
                        "Received a message from a different FEP SDK major version - "
                        "dropping message! (Received version: %d - Own version: %d)",
                        nMajorVer, FEP_SDK_PARTICIPANT_VERSION_MAJOR).c_str());
            }
        }
    }
    return nResult;
}

fep::Result cTransmissionAdapter::UnregisterNotificationListener(INotificationListener * poStatusListener)
{
    tMutexLockGuard oAdapterGuard(m_oAdapterMutex);
    tMutexLockGuard oContForkGuard(m_oContForkMutex);

    fep::Result nResult = ERR_NOERROR;
    if (NULL == poStatusListener)
    {
        nResult = ERR_POINTER;
    }
    else
    {
        tNotificationListeners::iterator itListener = std::find(m_vecNotificationListeners.begin(),
            m_vecNotificationListeners.end(), poStatusListener);
        if (m_vecNotificationListeners.end() == itListener)
        {
            nResult = ERR_NOT_FOUND;
        }
        else
        {
            m_vecNotificationListeners.erase(itListener);
        }
    }
    return nResult;
}

fep::Result cTransmissionAdapter::GetRecentSample(handle_t hSignalHandle, IPreparationDataSample* pSample) const
{
    fep::Result nResult = ERR_NOT_FOUND;
    if(m_bInitialized)
    {
        return reinterpret_cast<cDataReceiver*>(hSignalHandle)->GetCurrentSample(pSample);
    }
    else
    {
        nResult = ERR_NOT_INITIALISED;
    }
    return nResult;
}

fep::Result fep::cTransmissionAdapter::MuteSignal(handle_t hSignalHandle)
{
    fep::Result nResult = ERR_NOT_FOUND;
    for (std::vector<cDataReceiver*>::iterator it = m_vecDataReceiver.begin();
        it != m_vecDataReceiver.end(); ++it)
    {
        if ((*it) == hSignalHandle)
        {
            reinterpret_cast<cDataReceiver*>(*it)->Mute();
            nResult = ERR_NOERROR;
        }
    }

    if (fep::isFailed(nResult))
    {
        for (std::vector<cTransmitter*>::iterator it = m_vecDataTransmitter.begin();
            it != m_vecDataTransmitter.end(); ++it)
        {
            if ((*it) == hSignalHandle)
            {
                reinterpret_cast<cTransmitter*>(*it)->Mute();
                nResult = ERR_NOERROR;
            }
        }
    }
    return nResult;
}

fep::Result fep::cTransmissionAdapter::UnmuteSignal(handle_t hSignalHandle)
{
    fep::Result nResult = ERR_NOT_FOUND;
    for (std::vector<cDataReceiver*>::iterator it = m_vecDataReceiver.begin();
        it != m_vecDataReceiver.end(); ++it)
    {
        if ((*it) == hSignalHandle)
        {
            reinterpret_cast<cDataReceiver*>(*it)->Unmute();
            nResult = ERR_NOERROR;
        }
    }

    if (fep::isFailed(nResult))
    {
        for (std::vector<cTransmitter*>::iterator it = m_vecDataTransmitter.begin();
            it != m_vecDataTransmitter.end(); ++it)
        {
            if ((*it) == hSignalHandle)
            {
                reinterpret_cast<cTransmitter*>(*it)->Unmute();
                nResult = ERR_NOERROR;
            }
        }
    }
    return nResult;
}

fep::Result cTransmissionAdapter::RegisterSignal(const tSignal &oSignal,
    handle_t& hSignalHandle)
{
    fep::Result nResult = ERR_NOERROR;
    if (m_bInitialized)
    {
        if (fep::isOk(nResult))
        {
            if (SD_Output == oSignal.eDirection)
        {
                cTransmitter* poTransmitter = new cTransmitter();
                if ((NULL != poTransmitter))
                {
                    nResult = poTransmitter->Create(m_poTransmissionDriver,
                        m_pPropertyTree,
                        m_pIncidentInvocationHandler,
                        oSignal);

                    if (fep::isOk(nResult))
                    {
                        m_vecDataTransmitter.push_back(poTransmitter);
                        hSignalHandle = static_cast<void*>(poTransmitter);
                    }
                    else
                    {
                        delete poTransmitter;
                    }
                }
                else
                {
                    nResult = ERR_MEMORY;
                }
            }
            else if (SD_Input == oSignal.eDirection)
            {
                cDataReceiver* poDataReceiver = new cDataReceiver();
                if (NULL != poDataReceiver)
                {
                    nResult = poDataReceiver->Create(m_poTransmissionDriver,
                        m_pPropertyTree,
                        m_pIncidentInvocationHandler,
                        &m_oQueueManager,
                        oSignal);

                    if (fep::isOk(nResult))
                    {
                        m_vecDataReceiver.push_back(poDataReceiver);
                        hSignalHandle = static_cast<void*>(poDataReceiver);
                    }
                    else
                    {
                        delete poDataReceiver;
                    }
                }
                else
                {
                    nResult = ERR_MEMORY;
                }

            }
            else
            {
                nResult = ERR_UNEXPECTED;
            }
        }
    }
    else
    {
        nResult = ERR_NOT_INITIALISED;
    }
    return nResult;
}

fep::Result cTransmissionAdapter::UnregisterSignal(handle_t hSignalHandle)
{
    fep::Result nResult = ERR_NOT_FOUND;
    if(m_bInitialized)
    {
        for(std::vector<cDataReceiver*>::iterator it = m_vecDataReceiver.begin();
            it != m_vecDataReceiver.end(); ++it)
        {
            if(hSignalHandle == static_cast<void*>(*it))
            {
                delete (*it);
                m_vecDataReceiver.erase(it);
                nResult = ERR_NOERROR;
                break;
            }
        }
        if(fep::isFailed(nResult))
        {
            for(std::vector<cTransmitter*>::iterator it = m_vecDataTransmitter.begin();
                it != m_vecDataTransmitter.end(); ++it)
            {
                if(hSignalHandle == static_cast<void*>(*it))
                {
                    delete (*it);
                    m_vecDataTransmitter.erase(it);
                    nResult = ERR_NOERROR;
                    break;
                }
            }
        }
    }
    else
    {
        nResult = ERR_NOT_INITIALISED;
    }
    return nResult;
}

fep::Result cTransmissionAdapter::RegisterDataListener(fep::IPreparationDataListener* poDataListener, handle_t hSignalHandle)
{
    fep::Result nResult = ERR_NOT_FOUND;
    if(m_bInitialized)
    {
        for(std::vector<cDataReceiver*>::iterator it = m_vecDataReceiver.begin();
            it != m_vecDataReceiver.end(); ++it)
        {
            if( hSignalHandle == static_cast<void*>(*it))
            {
                nResult = (*it)->RegisterListener(poDataListener);
            }
        }
    }
    else
    {
        nResult = ERR_NOT_INITIALISED;
    }
    return nResult;
}

fep::Result cTransmissionAdapter::UnregisterDataListener(fep::IPreparationDataListener* poDataListener,
    const handle_t hSignalHandle)
{
    fep::Result nResult = ERR_NOT_FOUND;
    if(m_bInitialized)
    {
        for(std::vector<cDataReceiver*>::iterator it = m_vecDataReceiver.begin();
            it != m_vecDataReceiver.end(); ++it)
        {
            if( hSignalHandle == static_cast<void*>(*it))
            {
                nResult = (*it)->UnregisterListener(poDataListener);
            }
        }
    }
    else
    {
        nResult = ERR_NOT_INITIALISED;
    }
    return nResult;
}

fep::Result cTransmissionAdapter::TransmitData(fep::IPreparationDataSample* poPreparationSample)
{
    fep::Result nResult = ERR_NOT_FOUND;
    if(m_bInitialized)
    {
        handle_t hHandle = poPreparationSample->GetSignalHandle();
        if(!m_bGlobalDisabled)
        {
            std::vector<cTransmitter*>::const_iterator it = m_vecDataTransmitter.begin();
            for(; it != m_vecDataTransmitter.end(); ++it)
            {
                if( static_cast<void*>(*it) == hHandle)
                {
                    nResult = (*it)->TransmitData(poPreparationSample);
                }   
            }
        }
    }
    else
    {
        nResult = ERR_NOT_INITIALISED;
    }
    return nResult;
}

fep::Result cTransmissionAdapter::TransmitCommand(ICommand* poCommand)
{
    return TransmitMessage(poCommand);
}

fep::Result cTransmissionAdapter::TransmitNotification(INotification const * pNotification)
{
    return TransmitMessage(pNotification);
}

fep::Result cTransmissionAdapter::TransmitMessage(IMessage const *pMessage)
{
    fep::Result nResult = ERR_NOERROR;
    if(m_bInitialized)
    {
        const char *strMessage = pMessage->ToString();
        size_t szMsgLength  = a_util::strings::getLength(strMessage);
        if (s_nMessageStringLength < szMsgLength)
        {
            INVOKE_INCIDENT(m_pIncidentInvocationHandler,
                FSI_TRANSM_MSG_TX_FAILED, SL_Critical_Local,
                "Message size exceeds maximum message size. Message dropped!");
            nResult = ERR_INVALID_ARG;
        }

        if(fep::isOk(nResult))
        {
            nResult = m_pMessageTransmitter->Transmit(static_cast<const void *>(strMessage), szMsgLength+1);

            if(fep::isOk(nResult))
            {
                if (pMessage->GetMajorVersion() != FEP_SDK_PARTICIPANT_VERSION_MAJOR
                    || pMessage->GetMinorVersion() != FEP_SDK_PARTICIPANT_VERSION_MINOR)
                {
                    INVOKE_INCIDENT(m_pIncidentInvocationHandler,
                        FSI_TRANSM_MESSAGE_VERSION_MISMATCH_WARNING, SL_Warning,
                        "Transmitting message with differing version from FEP SDK version. "
                        "Warning: sent message might be ignored by other FEP Elements!");
                }
            }
            else
            {
                INVOKE_INCIDENT(m_pIncidentInvocationHandler,FSI_TRANSM_MSG_TX_FAILED,
                    SL_Critical_Local, "Transmitting sample failed. Driver could not send sample.")
            }
        }
    }
    else
    {
        nResult = ERR_NOT_INITIALISED;
    }
    return nResult;
}

const char* cTransmissionAdapter::GetModuleName()
{
    const char* strModuleName = NULL;
    m_pPropertyTree->GetPropertyValue(g_strElementHeaderPath_strElementName, strModuleName);
    return strModuleName;
}

} // ns fep
