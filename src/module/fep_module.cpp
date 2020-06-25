/**
* Implementation of the Class cModule.
*

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

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <a_util/base/types.h>
#include <a_util/result/result_type.h>
#include <a_util/result/error_def.h>
#include <a_util/strings/strings_format.h>
#include <a_util/strings/strings_functions.h>
#include <a_util/system/system.h>
#include <a_util/system/uuid.h>

#include "_common/fep_timestamp.h"
#include "data_access/fep_data_access.h"
#include "data_access/fep_user_data_access_intf.h"
#include "distributed_data_buffer/fep_ddb.h"
#include "distributed_data_buffer/fep_ddb_strategies.h"
#include "fep3/components/base/component_intf.h"
#include "fep3/components/base/component_registry.h"
#include "fep3/components/clock/local_clock_service.h"
#include "fep3/components/clock_sync_default/clock_sync_service.h"
#include "fep3/components/clock_sync_default/clock_sync_service_intf.h"
#include "fep3/components/data_registry/data_registry_fep2/data_registry_fep2.h"
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/components/legacy/property_tree/fep_component_config.h"
#include "fep3/components/legacy/property_tree/fep_function_config.h"
#include "fep3/components/legacy/property_tree/fep_module_header_config.h"
#include "fep3/components/legacy/property_tree/fep_propertytree.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep3/components/legacy/timing/interface/timing_intf_leg_component.h"
#include "fep3/components/legacy/timing/locked_step_legacy/timing_client_master_leg_comp.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep3/components/legacy/timing/timing_master_intf.h"
#include "fep3/components/rpc/fep_rpc_impl.h"
#include "fep3/components/rpc/fep_rpc_intf.h"
#include "fep3/components/clock/clock_service_intf.h"
#include "fep3/components/scheduler/scheduler_service_intf.h"
#include "fep3/components/scheduler/local_scheduler_service.h"
#include "fep_dptr.h"
#include "fep_errors.h"
#include "fep_module_common.h"
#include "fep_module_private.h"
#include "fep_sdk_participant_version.h"
#include "incident_handler/fep_incident_codes.h"
#include "incident_handler/fep_incident_handler.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "mapping/fep_mapping.h"
#include "mapping/fep_mapping_intf.h"
#include "messages/fep_command_access_intf.h"
#include "messages/fep_command_name_change_intf.h"
#include "messages/fep_notification_access_intf.h"
#include "messages/fep_notification_name_changed.h"
#include "module/fep_module_intf.h"
#include "module/fep_module_options.h"
#include "signal_registry/fep_signal_registry.h"
#include "signal_registry/fep_signal_registry_intf.h"
#include "signal_registry/fep_user_signal_options.h"
#include "fep3/base/states/fep2_state.h"
#include "statemachine/fep_state_helper.h"
#include "statemachine/fep_statemachine.h"
#include "statemachine/fep_statemachine_intf.h"
#include "transmission_adapter/fep_signal_direction.h"
#include "transmission_adapter/fep_transmission.h"
// include all components
#include "module/fep_module.h"

namespace fep {
class IDDBAccess;
class ITransmissionDriver;
}  // namespace fep


#define STARTUP_SETTLING_TIME_MS 1500

using namespace fep;

/**
* See \ref sec_element_header for more details about the participant header
* @cond no_doc
*/
char const * fep::g_strElementHeaderBase = FEP_PARTICIPANT_HEADER;

static double const s_fPropertyElementVersionDefault = 0.0;
char const * fep::g_strElementHeaderPath_fElementVersion =
FEP_PARTICIPANT_HEADER_VERSION;

char const * fep::g_strElementHeaderPath_strElementName =
FEP_PARTICIPANT_HEADER_NAME;

char const * fep::g_strElementHeaderPath_strElementDescription =
FEP_PARTICIPANT_HEADER_DESCRIPTION;

static double const s_fPropertyFEPVersionDefault = 0.0;
char const * fep::g_strElementHeaderPath_fFEPVersion =
FEP_PARTICIPANT_HEADER_FEP_VERSION;

char const * fep::g_strElementHeaderPath_strElementPlatform =
FEP_PARTICIPANT_HEADER_PLATFORM;

char const * fep::g_strElementHeaderPath_strElementContext =
FEP_PARTICIPANT_HEADER_CONTEXT;

static double const s_fPropertyModuleContextVersionDefault = 0.0;
char const * fep::g_strElementHeaderPath_fElementContextVersion =
FEP_PARTICIPANT_HEADER_CONTEXT_VERSION;

char const * fep::g_strElementHeaderPath_strElementVendor =
FEP_PARTICIPANT_HEADER_VENDOR;

char const * fep::g_strElementHeaderPath_strElementDisplayName =
FEP_PARTICIPANT_HEADER_DISPLAY_NAME;

static std::string s_strPropertyElementCompilationDateValue =
a_util::strings::format("%s %s", __DATE__, __TIME__);

char const * fep::g_strElementHeaderPath_strElementCompilationDate =
FEP_PARTICIPANT_HEADER_COMPILATION_DATE;

char const * fep::g_strElementHeaderPath_strElementCurrentState =
FEP_PARTICIPANT_HEADER_CURRENT_STATE;

char const * fep::g_strElementHeaderField_strElementCurrentState =
FEP_PARTICIPANT_HEADER_CURRENT_STATE_FIELD;

char const * fep::g_strElementHeaderPath_strElementHost =
FEP_PARTICIPANT_HEADER_HOST;

char const * fep::g_strElementHeaderPath_strInstanceID =
FEP_PARTICIPANT_HEADER_INSTANCE_ID;

char const * fep::g_strElementHeaderPath_strTypeID =
FEP_PARTICIPANT_HEADER_TYPE_ID;

char const * fep::g_strElementHeaderPath_bGlobalMute =
FEP_PARTICIPANT_HEADER_GLOBAL_MUTE;

template<typename T>
fep::Result setProperty(cModule::cModulePrivate& module, const char* path, const T& value)
{
    IPropertyTree* tree = module.GetModule()->GetPropertyTree();
    if (tree)
    {
        return setProperty(*tree, path, value);
    }
    RETURN_ERROR_DESCRIPTION(fep::ERR_POINTER, "not ready to use");
}

/**
* @endcond
*/

cModule::cModule()
{
    FEP_UTILS_D_CREATE(cModule);
    _d->m_poModule = this;

    // In the beginning FEP created all the components
    // Property Tree is the most important, because every other depend on it!
    _d->_component_registry.registerComponent<IPropertyTree>(new detail::PropertyTreeComponent(*this));
    _d->_component_registry.registerComponent<IDataRegistry>(new DataRegistryFEP2(*this));
}

cModule::~cModule()
{
    // block here if Create or Destroy are currently runnning
    while (true)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(_d->m_oInitMutex);
        if (_d->m_eModuleInitState == CS_FullyCreated)
        {
            Destroy();
            break;
        }
        else if (_d->m_eModuleInitState == CS_NotCreated)
        {
            break;
        }
        else
        {
            a_util::system::sleepMilliseconds(1);
        }
    }

    // Module is destroyed at this point!
}

fep::Result cModule::Create(const cModuleOptions& oModuleOptions,
                            ITransmissionDriver* pTransmissionDriver)
{
    fep::Result nResult = ERR_NOERROR;

    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(_d->m_oInitMutex);
        if (_d->m_eModuleInitState != CS_NotCreated)
        {
            nResult = ERR_RESOURCE_IN_USE;

            // The incident handler is only available after a successful Create
            if (_d->m_eModuleInitState == CS_FullyCreated)
            {
                INVOKE_INCIDENT(GetIncidentHandler(), FSI_MODULE_CREATED_AGAIN,
                    SL_Warning, "cModule::Create was called on an already initialized module!");
            }
        }
        else
        {
            _d->m_eModuleInitState = CS_DuringCreation;
        }
    }

    if (fep::isOk(nResult))
    {
        nResult = oModuleOptions.CheckValidity();
    }

    if (fep::isOk(nResult))
    {
        _d->m_strModuleName = oModuleOptions.GetParticipantName();
        _d->m_nDomainId = oModuleOptions.GetDomainId();
    }

    if (!_d->_component_registry.contains<IPropertyTree>())
    {
        _d->_component_registry.registerComponent<IPropertyTree>(new detail::PropertyTreeComponent(*this));
    }
    if (!_d->_component_registry.contains<IDataRegistry>())
    {
        _d->_component_registry.registerComponent<IDataRegistry>(new DataRegistryFEP2(*this));
    }

    if (fep::isOk(nResult))
    {
        if (fep::isOk(nResult))
        {
            nResult = CreateDefaultProperties(_d->m_strModuleName.c_str(),
                oModuleOptions.GetDefaultTimingSupport());
        }
    }

    // Data access etc. must exist at
    if (fep::isOk(nResult))
    {
        _d->m_poBusAdapter = new cTransmissionAdapter();
        _d->m_poIncidentHandler = new cIncidentHandler();
        _d->m_poSignalRegistry = new cSignalRegistry();
        _d->m_poSignalMapping = new cSignalMapping();
        _d->m_poDataAccess = new cDataAccess();
        _d->m_poStateMachine = new cStateMachine();


        // And FEP looked at its creation...
        if (NULL == _d->m_poStateMachine ||
            NULL == _d->m_poIncidentHandler ||
            NULL == _d->m_poSignalRegistry ||
            NULL == _d->m_poSignalMapping ||
            NULL == _d->m_poDataAccess)
        {
            nResult = ERR_MEMORY;
        }
        // ...and FEP saw it was good.
    }

    //this is RPC  
    if (fep::isOk(nResult))
    {
        cRPC* rpc = new cRPC(*this);
        nResult = _d->_component_registry.registerComponent<IRPCInternal>(rpc);
        nResult = _d->_component_registry.registerComponent<IRPC>(rpc);
    }

    if (fep::isOk(nResult))
    {
        nResult = _d->_component_registry.registerComponent<IClockService>(
                                                           new detail::LocalClockService(*_d->m_poIncidentHandler));
    }
    if (fep::isOk(nResult))
    {
        nResult = _d->_component_registry.registerComponent<IClockSyncService>(
                                                            new detail::ClockSynchronizationService());
    }
    
    if (fep::isOk(nResult))
    {
        IStateMachine* state_machine = _d->m_poStateMachine;
        std::function<fep::Result()> set_participant_to_error_state = [state_machine]() {
            return state_machine->ErrorEvent();
        };
        nResult = _d->_component_registry.registerComponent<ISchedulerService>(
                                                            new detail::LocalSchedulerService(*_d->m_poIncidentHandler, set_participant_to_error_state));
    }
    if (fep::isOk(nResult))
    {
        auto timing_legacy = new legacy::TimingInterfaceLegacy(*this);
        _d->m_pTiming = timing_legacy;
        nResult = _d->_component_registry.registerComponent<legacy::TimingInterfaceLegacy>(timing_legacy);
    }
    if (fep::isOk(nResult))
    {
        nResult = _d->m_poStateMachine->Initialize(GetCommandAccess(),
            GetIncidentHandler(),
            GetTimingInterface(),
            GetPropertyTree(),
            GetNotificationAccess(),
            _d,
            _d);
    }
    if (fep::isOk(nResult))
    {
        setProperty(*this, g_strElementHeaderPath_bGlobalMute, false);
        nResult = _d->m_poBusAdapter->Setup(GetPropertyTree(), _d->m_poIncidentHandler,
            oModuleOptions, pTransmissionDriver);
    }
    if (fep::isOk(nResult))
    {
        nResult = _d->m_poIncidentHandler->SetModule(this);
    }
    if (fep::isOk(nResult))
    {
        nResult = _d->m_poSignalRegistry->SetModule(_d);
    }
    if (fep::isOk(nResult))
    {
        nResult = _d->m_poSignalMapping->SetModule(_d);
    }
    if (fep::isOk(nResult))
    {
        nResult = _d->m_poDataAccess->Initialize(
            _d->GetTransmissionAdapter(),
            _d->GetSignalRegistry(),
            _d->GetSignalMapping(),
            _d->GetModule()->GetIncidentHandler(),
            _d->GetModule()->GetPropertyTree()
        );
    }
    if (oModuleOptions.GetDefaultTimingSupport() == fep::timing_FEP_20)
    {
        if (fep::isOk(nResult))
        {
            nResult = _d->_component_registry.registerComponent<legacy::ITimingSchedulerLegacy>(
                                       new legacy::TimingSchedulerLegacy(*this, *_d));
                           
        }
    }

    if (fep::isOk(nResult))
    {
        nResult = _d->_component_registry.create();
    }

    if (fep::isOk(nResult))
    {
        // register the _d Pointer as a catch-all strategy. The _d Pointer will
        // be responsible for calling the cModule::HandleFEPEvent() callback.
        // note: this "strategy" is not configurable
        nResult = _d->m_poIncidentHandler->AssociateCatchAllStrategy(_d, "");
    }

    if (fep::isOk(nResult))
    {
        // register as command listener to be able to receive name change commands
        nResult = GetCommandAccess()->RegisterCommandListener(_d);
    }

    if (fep::isOk(nResult))
    {
        nResult |= _d->m_poStateMachine->RegisterStateRequestListener(_d);
    }

    if (fep::isOk(nResult))
    {
        nResult |= _d->m_poStateMachine->RegisterStateEntryListener(this);
        /* even as we do not implement any Process<State>Entry method in cModule,
        * we need to register as listener anyway to give derived modules the possibility
        * to recognize the entry of state STARTUP */
    }

    if (fep::isOk(nResult))
    {
        nResult = _d->m_poStateMachine->FireupStateMachine();
    }

    a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(_d->m_oInitMutex);
    if (nResult != ERR_RESOURCE_IN_USE)
    {
        _d->m_eModuleInitState = fep::isOk(nResult) ? CS_FullyCreated : CS_NotCreated;
    }

    if (CS_NotCreated == _d->m_eModuleInitState)
    {
        // Something went wrong.  We have to forget everything we know.
        _d->Rollback();
    }

    // Give it some time to get "ready" and "reliable" (RTI DDS is somehow sleepy when getting up)
     a_util::system::sleepMilliseconds(STARTUP_SETTLING_TIME_MS);
    return nResult;
}

fep::Result cModule::Destroy()
{
    fep::Result nResult = ERR_NOERROR;

    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(_d->m_oInitMutex);
        if (_d->m_eModuleInitState != CS_FullyCreated)
        {
            nResult = ERR_NOT_INITIALISED;
        }
        else
        {
            _d->m_eModuleInitState = CS_DuringDestruction;
        }
    }
    if (fep::isOk(nResult))
    {
        nResult = _d->Rollback();
    }
    return nResult;
}

fep::Result cModule::cModulePrivate::Rollback()
{
    // this needs to be done before anything else is destroyed
    // as any state callbacks must still have access to the modules components
    if (NULL != m_poStateMachine)
    {
        // DONT unregister this, SetModule(NULL) will do it anyway
        // and this way the user gets a chance to react
        //m_poStateMachine->UnregisterStateEntryListener(this);

        m_poStateMachine->PerformShutdown();
    }

    // now we can destroy the remaining ddb entries
    while (!m_mapDDBEntries.empty())
    {
        _p->DestroyDDBEntry(m_mapDDBEntries.begin()->first);
    } 

    auto res_destroy = _component_registry.destroy();
    if (isFailed(res_destroy))
    {
       //we should LOG here (because the IncidentHandling is destroyed afterwards)
       //but there is no other logging concept than incidents
    }

    // take away module references from all components
    if (NULL != m_poStateMachine)
    {
        m_poStateMachine->Finalize();
    }

    if (NULL != m_poIncidentHandler)
    {
        m_poIncidentHandler->DisassociateCatchAllStrategy(this);
        m_poIncidentHandler->SetModule(NULL);
    }
    if (NULL != m_poBusAdapter)
    {
        m_poBusAdapter->UnregisterCommandListener(this);
        m_poBusAdapter->Destroy();
    }
    
    if (NULL != m_poSignalMapping)
    {
        m_poSignalMapping->SetModule(NULL);
    }
    if (NULL != m_poDataAccess)
    {
        m_poDataAccess->Finalize();
    }
    if (NULL != m_poSignalRegistry)
    {
        m_poSignalRegistry->SetModule(NULL);
    }  

    if (NULL != m_poIncidentHandler)
    {
        delete m_poIncidentHandler;
        m_poIncidentHandler = NULL;
    }
    if (NULL != m_poDataAccess)
    {
        delete m_poDataAccess;
        m_poDataAccess = NULL;
    }
    if (NULL != m_poSignalMapping)
    {
        delete m_poSignalMapping;
        m_poSignalMapping = NULL;
    }
    if (NULL != m_poSignalRegistry)
    {
        delete m_poSignalRegistry;
        m_poSignalRegistry = NULL;
    }
    if (NULL != m_poStateMachine)
    {
        delete m_poStateMachine;
        m_poStateMachine = NULL;
    }

    if (m_poBusAdapter != NULL)
    {
        delete m_poBusAdapter;
        m_poBusAdapter = NULL;
    }
    _component_registry.clear();
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_oInitMutex);
        m_eModuleInitState = CS_NotCreated;
    }
    return ERR_NOERROR;
}

bool cModule::IsShutdown() const
{
    return NULL == GetStateMachine() ||
        FS_SHUTDOWN == GetStateMachine()->GetState();
}

fep::Result cModule::WaitForShutdown(const timestamp_t tmTimeout) const
{
    fep::Result nResult = ERR_NOERROR;
    auto state_machine = GetStateMachine();
    if (state_machine)
    {
        nResult = state_machine->WaitForState(FS_SHUTDOWN, tmTimeout, false);
    }
    return nResult;
}

fep::Result cModule::WaitForState(const tState eState,
                                  const timestamp_t tmTimeout) const
{
    fep::Result nResult = ERR_NOERROR;
    auto state_machine = GetStateMachine();
    if (state_machine)
    {
        nResult = state_machine->WaitForState(eState, tmTimeout, true);
    }
    return nResult;
}

const char *cModule::GetName() const
{
    return _d->m_strModuleName.c_str();
}

fep::Result cModule::DestroyDDBEntry(handle_t const hSignal)
{
    fep::Result nResult = ERR_NOERROR;
    a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLock(_d->m_oDDBEntriesStateEntryMutex);
    cModulePrivate::tDDBMapItor itDDBEntry = _d->m_mapDDBEntries.find(hSignal);
    if (_d->m_mapDDBEntries.end() == itDDBEntry)
    {
        nResult = ERR_NOT_FOUND;
    }
    else
    {
        if (fep::isOk(nResult))
        {
            nResult = _d->m_poBusAdapter->UnregisterDataListener(itDDBEntry->second, hSignal);
        }
        if (fep::isOk(nResult))
        {
            nResult = GetSignalRegistry()->UnregisterSignal(hSignal);
        }
        delete itDDBEntry->second;
        _d->m_mapDDBEntries.erase(itDDBEntry);
    }
    return nResult;
}

IStateMachine* cModule::GetStateMachine() const
{
    return _d->m_poStateMachine;
}

fep::Result cModule::InitDDBEntry(char const * strSignalName,
    char const * strSignalType, size_t const szMaxDepth,
    fep::tDDBDeliveryStrategy const eDDBDeliveryStrategy, handle_t & hSignal, IDDBAccess ** ppoDDBAccess)
{
    fep::Result nResult = ERR_NOERROR;
    a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLock(_d->m_oDDBEntriesStateEntryMutex);
    if (std::string(strSignalName).empty() || std::string(strSignalType).empty() || 0 == szMaxDepth ||
        NULL == ppoDDBAccess)
    {
        nResult = ERR_EMPTY;
    }
    cDDB * poDDBEntry = new cDDB(GetIncidentHandler());

    if (NULL != poDDBEntry)
    {
        *ppoDDBAccess = NULL;
    }
    if (fep::isOk(nResult))
    {
        cUserSignalOptions oOpions(strSignalName, SD_Input, strSignalType);
        nResult = GetSignalRegistry()->RegisterSignal(oOpions, hSignal);
    }
    if (fep::isOk(nResult) &&
        _d->m_mapDDBEntries.end() != _d->m_mapDDBEntries.find(hSignal))
    {
        // duplicate handle => critical error
        nResult = ERR_INVALID_HANDLE;
        // FIXME: Unregister signal?
    }

    if (fep::isOk(nResult))
    {
        size_t szSampleSize = 0;
        GetSignalRegistry()->GetSignalSampleSize(hSignal, szSampleSize);
        nResult = poDDBEntry->CreateEntry(hSignal, szMaxDepth, szSampleSize, eDDBDeliveryStrategy);
    }

    if (fep::isOk(nResult))
    {
        nResult = _d->m_poBusAdapter->RegisterDataListener(poDDBEntry, hSignal);
    }
    if (fep::isOk(nResult))
    {
        // store DDB entry
        _d->m_mapDDBEntries[hSignal] = poDDBEntry;
        // connect ppoDDBAccess
        *ppoDDBAccess = poDDBEntry;
    }
    // Rollback if error occured
    else if (NULL != poDDBEntry)
    {
        _d->m_poBusAdapter->UnregisterDataListener(poDDBEntry, hSignal);
        GetSignalRegistry()->UnregisterSignal(hSignal);

        delete poDDBEntry;
    }
    return nResult;
}

fep::INotificationAccess* cModule::GetNotificationAccess() const
{
    return _d->m_poBusAdapter;
}

IUserDataAccess* cModule::GetUserDataAccess() const
{
    return _d->m_poDataAccess;
}

fep::ICommandAccess* cModule::GetCommandAccess() const
{
    return _d->m_poBusAdapter;
}

fep::IPropertyTree* cModule::GetPropertyTree() const
{
    auto a = _d->_component_registry.getComponent<fep::IPropertyTree>();
    return a;
}

fep::IIncidentHandler* cModule::GetIncidentHandler() const
{
    return _d->m_poIncidentHandler;
}

fep::ISignalRegistry* cModule::GetSignalRegistry() const
{
    return _d->m_poSignalRegistry;
}

fep::ISignalMapping* cModule::GetSignalMapping() const
{
    return _d->m_poSignalMapping;
}


fep::ITiming* cModule::GetTimingInterface() const
{
    if (_d->m_pTiming)
    {
        return _d->m_pTiming;
    }
    else
    {
        return static_cast<fep::ITiming*>(_d->_component_registry.getComponent<legacy::ITimingInterfaceLegacy>());
    }
}

fep::ITimingMaster* cModule::GetTimingMaster() const
{
    return static_cast<fep::ITimingMaster*>(_d->_component_registry.getComponent<legacy::ITimingInterfaceLegacy>());
}  

fep::IRPC* cModule::GetRPC() const
{
    return _d->_component_registry.getComponent<fep::IRPC>();
}

uint16_t cModule::GetDomainId() const
{
    return _d->m_nDomainId;
}

const char* cModule::GetDomainPrefix() const
{
    return "";
}

cModule::cModulePrivate::cModulePrivate() :
    m_poBusAdapter(NULL),
    m_poStateMachine(NULL), m_poIncidentHandler(NULL),
    m_poSignalRegistry(NULL), m_poSignalMapping(NULL), m_poDataAccess(NULL),
    m_pTiming(nullptr),
    m_eModuleInitState(CS_NotCreated), 
    m_eState(FS_UNKNOWN), m_nDomainId(0)
{

}

cModule::cModulePrivate::~cModulePrivate()
{
}

fep::Result cModule::cModulePrivate::ValidateModuleHeader()
{
    fep::Result nResult = ERR_NOERROR;

    if (!IsNumericPropertyValid(g_strElementHeaderPath_fElementVersion,
        s_fPropertyElementVersionDefault))
    {
        INVOKE_INCIDENT(m_poIncidentHandler,
            FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID, SL_Warning,
            "Element Version in Element Header has not been filled properly!");
        nResult = ERR_NOT_READY;
    }
    if (!IsStringPropertyValid(g_strElementHeaderPath_strElementName))
    {
        INVOKE_INCIDENT(m_poIncidentHandler,
            FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID, SL_Warning,
            "Element Name in Element Header has not been filled properly!");
        nResult = ERR_NOT_READY;
    }
    // module description will not be checked
    if (!IsNumericPropertyValid(g_strElementHeaderPath_fFEPVersion,
        s_fPropertyFEPVersionDefault))
    {
        INVOKE_INCIDENT(m_poIncidentHandler,
            FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID, SL_Warning,
            "FEP Version in Element Header has not been filled properly!");
        nResult = ERR_NOT_READY;
    }
    if (!IsStringPropertyValid(g_strElementHeaderPath_strElementPlatform))
    {
        INVOKE_INCIDENT(m_poIncidentHandler,
            FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID, SL_Warning,
            "Element Platform in Element Header has not been filled properly!");
        nResult = ERR_NOT_READY;
    }
    if (!IsStringPropertyValid(g_strElementHeaderPath_strElementContext))
    {
        INVOKE_INCIDENT(m_poIncidentHandler,
            FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID, SL_Warning,
            "Element Context in Element Header has not been filled properly!");
        nResult = ERR_NOT_READY;
    }
    if (!IsNumericPropertyValid(g_strElementHeaderPath_fElementContextVersion,
        s_fPropertyModuleContextVersionDefault))
    {
        INVOKE_INCIDENT(m_poIncidentHandler,
            FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID, SL_Warning,
            "Context Version in Element Header has not been filled properly!");
        nResult = ERR_NOT_READY;
    }
    if (!IsStringPropertyValid(g_strElementHeaderPath_strElementVendor))
    {
        INVOKE_INCIDENT(m_poIncidentHandler,
            FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID, SL_Warning,
            "Element Vendor in Element Header has not been filled properly!");
        nResult = ERR_NOT_READY;
    }
    if (!IsStringPropertyValid(g_strElementHeaderPath_strElementDisplayName))
    {
        INVOKE_INCIDENT(m_poIncidentHandler,
            FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID, SL_Warning,
            "Element Display name in Element Header has not been filled properly!");
        nResult = ERR_NOT_READY;
    }
    if (!IsStringPropertyValid(g_strElementHeaderPath_strElementCompilationDate))
    {
        INVOKE_INCIDENT(m_poIncidentHandler,
            FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID, SL_Warning,
            "Compilation Date in Element Header has not been filled properly!");
        nResult = ERR_NOT_READY;
    }
    if (!IsStringPropertyValid(g_strElementHeaderPath_strElementHost))
    {
        INVOKE_INCIDENT(m_poIncidentHandler,
            FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID, SL_Warning,
            "Element Host in Element Header has not been filled properly!");
        nResult = ERR_NOT_READY;
    }
    if (!IsStringPropertyValid(g_strElementHeaderPath_strInstanceID))
    {
        INVOKE_INCIDENT(m_poIncidentHandler,
            FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID, SL_Warning,
            "InstanceID in Element Header has not been filled properly!");
        nResult = ERR_NOT_READY;
    }
    if (!IsStringPropertyValid(g_strElementHeaderPath_strTypeID))
    {
        INVOKE_INCIDENT(m_poIncidentHandler,
            FSI_PROP_TREE_PARTICIPANT_HEADER_INVALID, SL_Warning,
            "TypeID in Element Header has not been filled properly!");
        nResult = ERR_NOT_READY;
    }

    return nResult;
}

bool cModule::cModulePrivate::IsStringPropertyValid(char const * strPropertyPath) const
{
    IPropertyTree* tree = _component_registry.getComponent<IPropertyTree>();
    IProperty const * poPropTemp =
        tree->GetLocalProperty(strPropertyPath);
    if (NULL == poPropTemp || !poPropTemp->IsString())
    {
        // property does not exist or is empty
        return false;
    }

    const char * strVal = NULL;
    if (fep::isFailed(poPropTemp->GetValue(strVal)))
    {
        return false;
    }

    return a_util::strings::getLength(strVal) != 0;
}

bool cModule::cModulePrivate::IsNumericPropertyValid(char const * strPropertyPath,
    double const fDefaultValue) const
{
    bool bPropertyValid = true;
    double fPropValueTemp = 0.0;
    IPropertyTree* tree = _component_registry.getComponent<IPropertyTree>();
    IProperty const * poPropTemp = tree->GetLocalProperty(strPropertyPath);
    if (NULL != poPropTemp && fep::isOk(poPropTemp->GetValue(fPropValueTemp)))
    {
        if (fDefaultValue == fPropValueTemp)
        {
            // value has not been changed since property creation and so is invalid
            bPropertyValid = false;
        }
    }
    else
    {
        // something went wrong during property creation
        bPropertyValid = false;
    }
    return bPropertyValid;
}

fep::Result cModule::cModulePrivate::HandleLocalIncident(fep::IModule* pModuleContext,
    const int16_t nIncident,
    const tSeverityLevel eSeverity,
    const char *strOrigin,
    int nLine,
    const char *strFile,
    const timestamp_t tmSimTime,
    const char* strDescription)
{
    fep::Result nResult = ERR_NOERROR;

    // only accept calls for our very own module context ;)
    if (pModuleContext != _p)
    {
        nResult = ERR_NOACCESS;
    }
    else
    {
        nResult = _p->HandleLocalIncident(nIncident, eSeverity, strOrigin, nLine, strFile, tmSimTime, strDescription);
    }
    return nResult;
}

fep::Result cModule::cModulePrivate::HandleGlobalIncident(const char *strSource,
    const int16_t nIncident,
    const tSeverityLevel eSeverity,
    const timestamp_t tmSimTime,
    const char *strDescription)
{
    // Per default, this is not a built-in part of a module. Remote incidents
    // require the user to implement his own strategy for handling it.
    return ERR_NOERROR;
}

fep::Result cModule::cModulePrivate::RefreshConfiguration(
    const fep::IProperty* pStrategyProperty,
    const fep::IProperty* pAffectedProperty)
{
    return ERR_NOERROR;
}

fep::Result cModule::cModulePrivate::ProcessStartupEntry(const fep::tState eOldState)
{
    //ATTENTION!!!
    //Never change the component order in this call unless you know
    // what you are doing!

    m_eState = FS_STARTUP;

    setProperty(*this,
                g_strElementHeaderPath_strElementCurrentState,
               cState::ToString(m_eState));

    return ERR_NOERROR;
}

fep::Result  cModule::cModulePrivate::ProcessIdleEntry(const fep::tState eOldState)
{
    //ATTENTION!!!
    //Never change the component order in this call unless you know
    // what you are doing!
    fep::Result nResult = ERR_NOERROR;

    if (fep::FS_READY == eOldState)
    {

        m_poBusAdapter->Disable();

        auto res = _component_registry.deinitializing();
        if (isFailed(res))
        {
            INVOKE_ERROR_AS_WARNING(*m_poIncidentHandler, "error while deinitialinzing", res);
        }

        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLock(m_oDDBEntriesStateEntryMutex);
        std::map < handle_t, fep::cDDB*>::iterator it = m_mapDDBEntries.begin();
        while (it != m_mapDDBEntries.end())
        {
            it->second->ResetData();
            ++it;
        }        
    }

    if (fep::FS_RUNNING == eOldState)
    {
        auto res = _component_registry.stop();
        if (isFailed(res))
        {
            INVOKE_ERROR_AS_WARNING(*m_poIncidentHandler, "error while stopping", res);
        }

        m_poBusAdapter->Disable();

        res = _component_registry.deinitializing();
        if (isFailed(res))
        {
            INVOKE_ERROR_AS_WARNING(*m_poIncidentHandler, "error while deinitializing", res);
        }

        fep::Result nResultLocal = m_poDataAccess->ClearAll();

        if (fep::isFailed(nResultLocal))
        {
            nResult = nResultLocal;
        }

        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLock(m_oDDBEntriesStateEntryMutex);
        std::map < handle_t, fep::cDDB*>::iterator it = m_mapDDBEntries.begin();
        while (it != m_mapDDBEntries.end())
        {
            it->second->ResetData();
            ++it;
        }
    }

    if (fep::FS_ERROR == eOldState)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::mutex> oLock(m_oDDBEntriesStateEntryMutex);
        std::map < handle_t, fep::cDDB*>::iterator it = m_mapDDBEntries.begin();
        while (it != m_mapDDBEntries.end())
        {
            it->second->ResetData();
            ++it;
        }
    }

    if (fep::FS_STARTUP != eOldState && m_poSignalMapping)
    {
        m_poSignalMapping->ClearMappingConfiguration();
    }

    if (fep::FS_STARTUP != eOldState && m_poSignalRegistry)
    {
        m_poSignalRegistry->ClearSignalDescriptions();
    }


    m_eState = FS_IDLE;
    setProperty(*this,
                g_strElementHeaderPath_strElementCurrentState,
                cState::ToString(m_eState));
    
    return nResult;
}

fep::Result cModule::cModulePrivate::ProcessInitializingEntry(const tState eOldState)
{
    //ATTENTION!!!
    //Never change the component order in this call unless you know
    // what you are doing!
    if (m_poSignalRegistry)
    {
        m_poSignalRegistry->ConfigureRemoteDescription();
    }
    if (m_poSignalMapping)
    {
        m_poSignalMapping->ConfigureRemoteMapping();
    }

    RETURN_IF_FAILED(_component_registry.initializing());

    m_eState = FS_INITIALIZING;
    setProperty(*this,
                g_strElementHeaderPath_strElementCurrentState,
                cState::ToString(m_eState));

    return ERR_NOERROR;
}

fep::Result  cModule::cModulePrivate::ProcessReadyEntry(const fep::tState eOldState)
{
    //ATTENTION!!!
    //Never change the component order in this call unless you know
    // what you are doing!

    m_poBusAdapter->Enable();
    if (eOldState != FS_RUNNING)
    {
        m_poSignalRegistry->DisallowSignalRegistration();
    }

    m_eState = FS_READY;
    setProperty(*this,
                g_strElementHeaderPath_strElementCurrentState,
                cState::ToString(m_eState));


    return ERR_NOERROR;
}

fep::Result cModule::cModulePrivate::ProcessRunningEntry(const fep::tState eOldState)
{
    fep::Result nResult = ERR_NOERROR;

    //ATTENTION!!!
    //Never change the component order in this call unless you know
    // what you are doing!

    m_poSignalMapping->StartMappingEngine();
    if (eOldState != FS_READY)
    {
        m_poSignalRegistry->DisallowSignalRegistration();
    }
    

    RETURN_IF_FAILED(_component_registry.start());

    m_eState = FS_RUNNING;
    setProperty(*this,
                g_strElementHeaderPath_strElementCurrentState,
                cState::ToString(m_eState));


    return nResult;
}

fep::Result cModule::cModulePrivate::ProcessErrorEntry(const fep::tState eOldState)
{
    fep::Result nResult = ERR_NOERROR;

    //ATTENTION!!!
    //Never change the component order in this call unless you know
    // what you are doing!

    if (FS_RUNNING == eOldState)
    {
        auto res = _component_registry.stop();
        if (isFailed(res))
        {
            INVOKE_ERROR_AS_WARNING(*m_poIncidentHandler, "error while stopping in error state", res);
        }

    }

    if (FS_READY == eOldState || FS_RUNNING == eOldState)
    {
        m_poBusAdapter->Disable();

        auto res = _component_registry.deinitializing();
        if (isFailed(res))
        {
            INVOKE_ERROR_AS_WARNING(*m_poIncidentHandler, "error while deinitialing in error state", res);
        }
    }

    m_eState = FS_ERROR;
    setProperty(*this,
                g_strElementHeaderPath_strElementCurrentState,
                cState::ToString(m_eState));


    return nResult;
}

fep::Result cModule::cModulePrivate::ProcessShutdownEntry(const tState eOldState)
{
    //ATTENTION!!!
    //Never change the component order in this call unless you know
    // what you are doing!

    m_eState = FS_SHUTDOWN;
    setProperty(*this,
                g_strElementHeaderPath_strElementCurrentState,
                cState::ToString(m_eState));
    

    if (m_oShutdownHandler)
    {
        m_oShutdownHandler();
    }
    return ERR_NOERROR;
}

fep::Result cModule::cModulePrivate::CleanUp(const fep::tState eOldState)
{
    //ATTENTION!!!
    //Never change the component order in this call unless you know
    // what you are doing!

    fep::Result nResult = ERR_NOERROR;
    if (m_poStateMachine)
    {
        m_poStateMachine->ResetStandAloneProperty();
    }

    if (m_poIncidentHandler)
    {
        nResult = m_poIncidentHandler->SetDefaultProperties();
    }
 
    return nResult;
}

fep::Result cModule::cModulePrivate::ProcessStartupExit(const fep::tState eNewState)
{
    //ATTENTION!!!
    //Never change the component order in this call unless you know
    // what you are doing!

    ValidateModuleHeader();
    return ERR_NOERROR;
}

fep::Result cModule::cModulePrivate::ProcessIdleExit(const fep::tState eNewState)
{
    //ATTENTION!!!
    //Never change the component order in this call unless you know
    // what you are doing!

    return ERR_NOERROR;
}

fep::Result cModule::cModulePrivate::ProcessInitializingExit(const fep::tState eNewState)
{
    //ATTENTION!!!
    //Never change the component order in this call unless you know
    // what you are doing!

    return ERR_NOERROR;
}

fep::Result cModule::cModulePrivate::ProcessReadyExit(const fep::tState eNewState)
{
    //ATTENTION!!!
    //Never change the component order in this call unless you know
    // what you are doing!

    if (FS_RUNNING != eNewState)
    {
        m_poSignalRegistry->AllowSignalRegistration();
    }

    return ERR_NOERROR;
}

fep::Result cModule::cModulePrivate::ProcessRunningExit(const fep::tState eNewState)
{
    //ATTENTION!!!
    //Never change the component order in this call unless you know
    // what you are doing!

    if (m_poSignalMapping)
    {
        m_poSignalMapping->ResetMappingEngine();
    }

    if (FS_READY != eNewState)
    {
        m_poSignalRegistry->AllowSignalRegistration();
    }
    return ERR_NOERROR;
}

fep::Result cModule::cModulePrivate::ProcessErrorExit(const fep::tState eNewState)
{
    //ATTENTION!!!
    //Never change the component order in this call unless you know
    // what you are doing!

    return ERR_NOERROR;
}

fep::Result cModule::cModulePrivate::ProcessReadyRequest(const fep::tState eOldState)
{
    fep::Result nResult = ERR_NOERROR;
    if (FS_INITIALIZING == eOldState)
    {
        auto res = _component_registry.ready();
        if (isFailed(res))
        {
            _p->GetIncidentHandler()->InvokeIncident(FSI_GENERAL_CRITICAL,
                fep::SL_Critical,
                a_util::strings::format("Participant %s can not proceed ready request: %s",
                    _p->GetName(),
                    res.getDescription()).c_str(),
                _p->GetName(),
                res.getLine(),
                res.getFunction());
        }
        return res;
    }

    return nResult;
}

fep::Result cModule::cModulePrivate::Update(const fep::INameChangeCommand* poCommand)
{
    ChangeName(poCommand->GetNewName());
    return ERR_NOERROR;
}

IModule* cModule::cModulePrivate::GetModule()
{
    return m_poModule;
}

cTransmissionAdapter* cModule::cModulePrivate::GetTransmissionAdapter()
{
    return m_poBusAdapter;
}

cSignalRegistry* cModule::cModulePrivate::GetSignalRegistry()
{
    return m_poSignalRegistry;
}

cSignalMapping* cModule::cModulePrivate::GetSignalMapping()
{
    return m_poSignalMapping;
}

cDataAccess* cModule::cModulePrivate::GetDataAccess()
{
    return m_poDataAccess;
}

fep::Result cModule::cModulePrivate::ChangeName(const char* strNewName)
{
    if (a_util::strings::getLength(strNewName) == 0)
    {
        return ERR_FAILED;
    }
    else if (a_util::strings::isEqual(strNewName, m_strModuleName.c_str()))
    {
        return ERR_NOERROR;
    }

    cModuleOptions oModuleOptions(strNewName);
    if (isFailed(oModuleOptions.CheckValidity()))
    {
        return ERR_INVALID_ARG;
    }

    // apply new name and save old name
    std::string strOldModuleName = m_strModuleName;
    m_strModuleName = strNewName;
    
    //apply to the RPC ObjectRegistry to React on the right name, to send the right name
    IRPCInternal* rpc_internal = _component_registry.getComponent<IRPCInternal>();
    if (rpc_internal)
    {
         rpc_internal->setLocalName(m_strModuleName);
    }

    // set participant header
    setProperty(*this, g_strElementHeaderPath_strElementName, m_strModuleName.c_str());

    // transmit name changed notification
    cNameChangedNotification oNotification(strOldModuleName.c_str(), strNewName, "*",
        GetTimeStampMicrosecondsUTC(), m_poModule->GetTimingInterface()->GetTime());
    m_poModule->GetNotificationAccess()->TransmitNotification(&oNotification);
    return ERR_NOERROR;
}

fep::Result cModule::SetStandAloneModeEnabled(bool bEnable)
{
    fep::Result nResult = ERR_NOERROR;

    if (fep::isOk(nResult))
    {
        IPropertyTree *pPropertyTree = cModule::GetPropertyTree();
        if (!pPropertyTree) { return ERR_POINTER; }

        nResult = pPropertyTree->SetPropertyValue(
            component_config::g_strStateMachineStandAloneModePath_bEnable, bEnable);

        /* Incident for stand alone mode should be invoked by the STM, but not here */
    }

    return nResult;
}

fep::Result cModule::HandleLocalIncident(const int16_t nIncident,
    const fep::tSeverityLevel eSeverity,
    const char *strOrigin,
    int nLine,
    const char *strFile,
    const timestamp_t tmSimTime,
    const char *strDescription)
{
    // empty implementation - to be specified by the user
    return ERR_NOERROR;
}

fep::Result cModule::CreateDefaultProperties(const char* strElementName,
                                             fep::eTimingSupportDefault timingsupport)
{
    fep::Result nResult = ERR_NOERROR;

    // Generate UUID for setting the header property
    std::string strUUID = a_util::system::generateUUIDv4();
    
    IPropertyTree* tree = GetPropertyTree();

    // Set ElementHeader
    nResult |= setPropertyIfNotExists(*tree, g_strElementHeaderPath_fElementVersion, s_fPropertyElementVersionDefault);
    nResult |= setPropertyIfNotExists(*tree, g_strElementHeaderPath_strElementName, std::string(strElementName));
    nResult |= setPropertyIfNotExists(*tree, g_strElementHeaderPath_strElementDescription, "");
    const auto fep_version = static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MAJOR) + static_cast<double>(FEP_SDK_PARTICIPANT_VERSION_MINOR) / 10;
    nResult |= setPropertyIfNotExists(*tree, g_strElementHeaderPath_fFEPVersion, fep_version);
    nResult |= setPropertyIfNotExists(*tree, g_strElementHeaderPath_strElementPlatform, FEP_SDK_PARTICIPANT_PLATFORM_STR);
    nResult |= setPropertyIfNotExists(*tree, g_strElementHeaderPath_strElementContext, "");
    nResult |= setPropertyIfNotExists(*tree, g_strElementHeaderPath_fElementContextVersion, s_fPropertyModuleContextVersionDefault);
    nResult |= setPropertyIfNotExists(*tree, g_strElementHeaderPath_strElementVendor, "");
    nResult |= setPropertyIfNotExists(*tree, g_strElementHeaderPath_strElementDisplayName, strElementName);
    nResult |= setPropertyIfNotExists(*tree, g_strElementHeaderPath_strElementCompilationDate, s_strPropertyElementCompilationDateValue.c_str());
    nResult |= setPropertyIfNotExists(*tree, g_strElementHeaderPath_strElementCurrentState, "");
    nResult |= setPropertyIfNotExists(*tree, g_strElementHeaderPath_strElementHost, a_util::system::getHostname().c_str());
    nResult |= setPropertyIfNotExists(*tree, g_strElementHeaderPath_strInstanceID, strUUID.c_str());
    nResult |= setPropertyIfNotExists(*tree, g_strElementHeaderPath_strTypeID, "");

    // Set FunctionConfig
    nResult |= setPropertyIfNotExists(*tree, function_config::g_strFunctionConfig, "");

    // Set ComponentConfig
    nResult |= setPropertyIfNotExists(*tree, component_config::g_strComponentConfig, "");

    if (timingsupport == timing_FEP_20)
    {
        setProperty(*tree, FEP_SCHEDULERSERVICE_SCHEDULER,
                           FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME);
        setProperty(*tree, FEP_CLOCKSERVICE_MAIN_CLOCK,
                           FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_MASTER_LOCKED_STEP_SIMTIME);
    }
    else
    {
        setProperty(*tree, FEP_SCHEDULERSERVICE_SCHEDULER,
                           FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED);
        setProperty(*tree, FEP_CLOCKSERVICE_MAIN_CLOCK,
                           FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME);
    }


    return nResult;
}

fep::Result fep::cModule::Rename(const char* strNewName)
{
    return _d->ChangeName(strNewName);
}

void fep::cModule::SetShutdownHandler(const std::function<void()>& oShutdownHandler)
{
    _d->m_oShutdownHandler = oShutdownHandler;
}

fep::IComponents* fep::cModule::GetComponents() const
{
    return &_d->_component_registry;
}
