/**
 * Declaration of the Class cModule.
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

#if !defined(EA_B1FA1506_F804_4396_BF54_852D38FA05AA__INCLUDED_)
#define EA_B1FA1506_F804_4396_BF54_852D38FA05AA__INCLUDED_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <a_util/base/types.h>
#include <a_util/strings/strings_format.h>
#include "./statemachine/fep_state_entry_listener.h"

#include "data_access/fep_user_data_access_intf.h"
#include "distributed_data_buffer/fep_ddb_strategies.h"
#include "fep3/components/base/component_intf.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/base/states/fep2_state.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep3/components/legacy/timing/timing_master_intf.h"
#include "fep3/components/rpc/fep_rpc_intf.h"
#include "fep_participant_export.h"
#include "fep_result_decl.h"
#include "fep_dptr.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "mapping/fep_mapping_intf.h"
#include "messages/fep_command_access_intf.h"
#include "messages/fep_notification_access_intf.h"
#include "module/fep_module_intf.h"
#include "module/fep_module_options.h"
#include "signal_registry/fep_signal_registry_intf.h"
#include "statemachine/fep_state_entry_listener.h"
#include "statemachine/fep_statemachine_intf.h"

namespace fep
{
    class IDDBAccess;
    class ITransmissionDriver;

    /// Property base path of all participant header properties
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderBase;
    /// participant header property path for element version
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_fElementVersion;
    /// participant header property path for element name
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementName;
    /// participant header property path for element description
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementDescription;
    /// participant header property path for FEP version
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_fFEPVersion;
    /// participant header property path for element's platform
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementPlatform;
    /// participant header property path for element's context
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementContext;
    /// participant header property path for element's context's version
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_fElementContextVersion;
    /// participant header property path for element vendor
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementVendor;
    /// participant header property path for element display name
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementDisplayName;
    /// participant header property path for element compilation date
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementCompilationDate;
    /// participant header property path for current element state
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementCurrentState;
    /// participant header field name for current element state
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderField_strElementCurrentState;
    /// participant header property path for the hostname of the element
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strElementHost;
    /// participant header property path for the autogenerated uuid of the element
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strInstanceID;
    /// participant header property path for the author defined uuid of the element
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_strTypeID;
    /// participant header property path for current muting state of the element
    extern FEP_PARTICIPANT_EXPORT char const * g_strElementHeaderPath_bGlobalMute;

    /**
     * This is the base class for every FEP Element.
     * It will contain the DDBs, property tree, state machine and transmission adapter.
     * To implement your own FEP Element, inherit from this class.
     * For more information about FEP Elements, see \ref fep_general_usage.
     */
    class FEP_PARTICIPANT_EXPORT cModule : public fep::cStateEntryListener,
                                    public fep::IModule
    {
        /// D-pointer to private implementation
         FEP_UTILS_D(cModule);

    public:
        /**
         * CTOR
         */
        cModule();

        /**
         * DTOR
         */
        virtual ~cModule();

    private:
        /**
         * @brief cModule Copy Constructor
         *
         * @warning Do not copy cModule objects!
         *
         * \sa operator=(const cModule &)
         */
        cModule(const cModule &) = delete;

        /**
         * @brief cModule Assignment Operator
         *
         * @warning Do not copy cModule objects!
         *
         * @return instance of cModule
         * \sa cModule(const cModule &)
         */
        cModule &operator=(const cModule &) = delete;

    public:
        /**
         * The method \c Create will create the internal components of the FEP Module to provide 
         * them to the inheriting FEP Element. Also it will put cModule and its child classes into 
         * state STARTUP.
         * The element name \a strElementName will be used for sending and receiving commands
         * and for sending status updates.
         *
         * In case no pointer to a transmission driver instance is provided (pTransmissionDriver):
         * The method will read the environment variable FEP_TRANSMISSION_DRIVER to determine
         * the transmission driver type and default to TT_RTI_DDS if the variable is not set.
         * 
         *
         * @param [in] oModuleOptions  The Options for the element.
         * @param [in] pTransmissionDriver (optional) Pointer to an external TransmissionDriver. 
         *                                 If left empty a transmission driver has to be selected
         *                                 in the ModuleOptions oModuleOptions.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_RESOURCE_IN_USE The module is already initialized.
         * @retval ERR_EMPTY Element name \c strElementName is empty (\c "").
         * @retval ERR_INVALID_ARG Element name \c strElementName contains wildcards or other
         *                         forbidden symbols. 
         * @retval ERR_INVALID_HANDLE The environment variable is set incorrectly.
         * @retval ERR_NOT_SUPPORTED Requested unsupported transmission adapter.
         */
        virtual fep::Result Create(const cModuleOptions& oModuleOptions,
                                   ITransmissionDriver* pTransmissionDriver = NULL);

        /**
        * The method \c Rename will rename the FEP Participant.
        *
        * @param [in] strNewName   The new name for the element.
        * @returns  Standard result code.
        * @retval ERR_NOERROR  Everything went fine
        * @retval ERR_FAILED The new name is empty.
        */
        fep::Result Rename(const char* strNewName);

    public:
        /**
         * The method Destroy will destroy the internal components of the FEP Module if it was 
         * initialized properly. Note that all FEP Components will not longer be available to the 
         * inheriting FEP Element. This method is also called by the destructor, but in situations 
         * where a module/element is to be reinitialized again later, this is the method to use.
         * 
         * \note During destruction, the state machine will be shut down to the
         *   \ref fep_states_shutdown "Shutdown" state.
         *   This involves calling the \ref fep_states_cleanup "Cleanup" method of cModule.
         *   Override this method to correctly cleanup any registrations of your class before it is destroyed.
         *
         * \warning DO NOT CALL THIS METHOD FROM INSIDE THE FEP Element CONTEXT! This will very likely
         * lead to crashes and pure virtual function calls as vital information of the module is being
         * deleted while still in use!
         *
         * @returns Standard result code.
         * @retval ERR_NOERROR Everything went fine.
         * @retval ERR_NOT_INITIALISED The module is not initialized properly.
         */
        virtual fep::Result Destroy();

        /**
         * The method \c InitDDBEntry will create a new DDB entry.
         * See \ref fep_data for more information about the DDB concept in FEP.
         *
         * \note This will register a \ref fep::SD_Input "SD_Input" Signal 
         * in the background. So you should NOT register the signal yourself 
         * before you call InitDDBEntry.
         *
         * @param [in] strSignalName  The name of the signal that should be received by the DDB.
         * @param [in] strSignalType  The type of the signal that should be received by the DDB.
         * @param [in] szMaxDepth     The maximum amount of instances the DDB should be able to 
         *                            hold.
           @param [in] eDDBDeliveryStrategy The \ref tDDBDeliveryStrategy. By default, incomplete
        *                                   frames are delivered to the user.
         * @param [out] hSignal       Handle of the signal registered by the DDB / this DDB is 
         *                            is used for.
         * @param [out] ppoDDBAccess  An object for accessing the DDB. It is owned by this class.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        fep::Result InitDDBEntry(char const * strSignalName,
            char const * strSignalType,
            size_t const szMaxDepth,
            fep::tDDBDeliveryStrategy const eDDBDeliveryStrategy,
            handle_t & hSignal, fep::IDDBAccess** ppoDDBAccess);

        /**
         * The method \c DestroyDDBEntry will remove an previously created DDBEntry.
         *
         * \note The corresponding signal will be unregistered at the ITransmissionAdapter; 
         *       the signal handle will not be valid any longer.
         *
         * @param [in] hSignal   handle of the signal whose DDB should be destroyed 
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         */
        fep::Result DestroyDDBEntry(handle_t const hSignal);

        /**
         * If \p bEnable is true, the \c cModule ignores control commands sent from a master
         * FEP Element within the FEP network and preserves its state.
         *
         * By default, \p bEnable is false.
         *
         * @param [in] bEnable
         *
         * @warning    This method should never be called before the FEP Element has been created.
         *
         * @returns    Standard result code.
         * @retval     ERR_NOERROR if everything went fine.
         *
         * @see        cModule::Create()
         */
        fep::Result SetStandAloneModeEnabled(bool bEnable);

        
        /**
         * @brief Checks whether a module/element is ready to switch off (= is in final state 
         * SHUTDOWN or has already been Destroy()'ed)
         *
         * @return bool true if module/element is in (final) state SHUTDOWN and can be deleted, 
         * false otherwise
         */
        bool IsShutdown() const; 

        /**
         * !WARNING: not implemented - just a placeholder for future implementation!
         * Set the domain prefix for this FEP Element.
         *
         * @param [in] strDomainPrefix The domain prefix.
         * @return Standard result code.
         * @retval ERR_NOT_IMPL Currently not implemented!
         */
        fep::Result SetDomainPrefix(const char* strDomainPrefix);

        /**
         * @brief Waits for the module/element to reach the final state SHUTDOWN
         *
         * @param [in] tmTimeout Time (ms) to wait before returning ERR_TIMEOUT, -1 for infinite duration
        *
         * @retval ERR_NOERROR Everything went fine, the module/element has reached SHUTDOWN
         * @retval ERR_TIMEOUT The module/element did not reach SHUTDOWN within the specified timespan
         * @retval ERR_INVALID_ARG Invalid timeout parameter (< -1)
         */
        fep::Result WaitForShutdown(const timestamp_t tmTimeout = -1) const;

        /**
         * @brief The method \ref WaitForState waits for the module to reach a certain state
         *
         * @param [in] eState This is the state to be waited for
         * @param [in] tmTimeout Time (ms) to wait before returning ERR_TIMEOUT,
         *             -1 for infinite duration
         * 
         * @retval ERR_NOERROR Everything went fine, the module/element has reached the expected state
         * @retval ERR_TIMEOUT The module/element did not reach the expected state within the
         * specified timespan
         * @retval ERR_INVALID_ARG Invalid timeout parameter (< -1) or invalid state parameter
         */
        fep::Result WaitForState(const tState eState, const timestamp_t tmTimeout = -1) const;

        /**
        * Set a shutdown handler that gets called when the Module enters FS_SHUTDOWN
        *
        * \note The callback function must be threadsafe!
        * \note If the lifetime of the Module exceeds the lifetime of any
        *       element used in the callback function, the shutdown handler must be reset beforehand
        *       (e.g. to an empty function object)
        * \warning only ONE shutdown handler can be registered. A second 
        *          registration will automatically unregister the first shutdown handler. 
        *          Thus you can unregister on purpose by registering nullptr as shutdown handler.
        *
        * @param [in] oShutdownHandler Callback function
        */
        void SetShutdownHandler(const std::function<void()>& oShutdownHandler);

    public: // implements IModule
        const char* GetName() const;
        fep::IPropertyTree* GetPropertyTree() const;
        fep::IStateMachine* GetStateMachine() const;
        fep::INotificationAccess* GetNotificationAccess() const;
        fep::IUserDataAccess* GetUserDataAccess() const;
        fep::ICommandAccess* GetCommandAccess() const;
        fep::IIncidentHandler* GetIncidentHandler() const;
        fep::ISignalRegistry* GetSignalRegistry() const;
        fep::ISignalMapping* GetSignalMapping() const;
        fep::ITiming* GetTimingInterface() const;
        fep::ITimingMaster* GetTimingMaster() const;
        fep::IRPC* GetRPC() const;
        uint16_t GetDomainId() const;
        const char* GetDomainPrefix() const;

        fep::IComponents* GetComponents() const;

    protected:
        /**
         * Module-internal callback to handle local incidents of this particular module instance 
         * and the inheriting FEP Element, including its components such as the FEP Transmission 
         * Layer subsystem or the FEP State Machine.
         *
         * \note The FEP Incident Handler has to have been enabled for this callback to be
         * triggered.
         *
         * \see fep_incident_handling
         *
         * @param [in] nIncident The incident code that has been invoked.
         * @param [in] eSeverity The severity level of the recorded incident
         * @param [in] strDescription An optional description which has been supplied by the
         * invoking component.
         * @param [in] strOrigin The origin-component of the incident (e.g StateMachine, Signal-Registry...)
         * @param [in] nLine Line-number where the incident occurred. (only relevant in Debug build)
         * @param [in] strFile File in which the incident was invoked. (only relevant in Debug build)
         * @param [in] tmSimTime Time stamp of the simulation time at the incident invokation.
         *
         * @return User-defined return value. The return value is not evaluated by the
         * FEP Incident Handler.
         */
        virtual fep::Result HandleLocalIncident(const int16_t nIncident,
                                            const fep::tSeverityLevel eSeverity,
                                            const char *strOrigin,
                                            int nLine,
                                            const char *strFile,
                                            const timestamp_t tmSimTime,
                                            const char* strDescription = NULL);

    private:
        ///@cond nodoc
        fep::Result CreateDefaultProperties(const char* strElementName, 
                                            fep::eTimingSupportDefault timingsupport);
        ///@endcond
        
    };

    /**
     * @brief helper macro to invoke an error within incident handler
     * 
     * @param handler the incident handler to invoke to
     * @param severity the level
     * @param additional_desc additional description at the beginngin of the message
     * @param err the error to invoke
     */
    void inline invokeErrorAsIncident(IIncidentHandler& handler,
        fep::tSeverityLevel severity,
        const char* additional_desc,
        fep::Result err)
    {
        //additional_desc
        const char* desc = err.getDescription();
        if (desc == nullptr)
        {
            desc = "";
        }
        const char* label = err.getErrorLabel();
        if (label == nullptr)
        {
            label = "";
        }
        handler.InvokeIncident(1,
            severity,
            a_util::strings::format("%s - code(%d), label(%s) : %s",
                additional_desc,
                err.getErrorCode(),
                label,
                desc).c_str(),
            "",
            err.getLine(),
            err.getFile());
    }

    /**
     * @brief helper macro to invoke an error as information within incident handler
     * @param handler the incident handler
     * @param additional_desc description added at the beginning of the invoke message
     * @param err the result error 
     */
    #define INVOKE_ERROR_AS_INFO(handler, additional_desc, err) \
             invokeErrorAsIncident(handler, fep::tSeverityLevel::SL_Info, additional_desc, err);
    /**
     * @brief helper macro to invoke an error as warning within incident handler
     * @param handler the incident handler
     * @param additional_desc description added at the beginning of the invoke message
     * @param err the result error 
     */
    #define INVOKE_ERROR_AS_WARNING(handler, additional_desc, err) \
            invokeErrorAsIncident(handler, fep::tSeverityLevel::SL_Warning, additional_desc, err);
    /**
     * @brief helper macro to invoke an error as critical within incident handler
     * @param handler the incident handler
     * @param additional_desc description added at the beginning of the invoke message
     * @param err the result error 
     */
    #define INVOKE_ERROR_AS_CRITICAL(handler, additional_desc, err) \
            invokeErrorAsIncident(handler, fep::tSeverityLevel::SL_Critical, additional_desc, err);

}
#endif // !defined(EA_B1FA1506_F804_4396_BF54_852D38FA05AA__INCLUDED_)