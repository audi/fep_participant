/**
 * Declaration of the Class IModule.
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

#if !defined(EA_E4D6E119_67FB_4cfe_A4BD_744871A82D28__INCLUDED_)
#define EA_E4D6E119_67FB_4cfe_A4BD_744871A82D28__INCLUDED_

#include "fep_types.h"
#include "fep3/components/base/component_intf.h"
#include "statemachine/fep_statemachine_intf.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "messages/fep_notification_access_intf.h"
#include "data_access/fep_user_data_access_intf.h"
#include "messages/fep_command_access_intf.h"
#include "mapping/fep_mapping_intf.h"
#include "signal_registry/fep_signal_registry_intf.h"
#include "fep3/components/legacy/timing/timing_client_intf.h"
#include "fep3/components/legacy/timing/timing_master_intf.h"
#include "fep3/components/rpc/fep_rpc_intf.h"

namespace fep
{
    /**
     * Basic interface for a FEP Module, that is inherited to every FEP Element.
     */ 
    class FEP_PARTICIPANT_EXPORT IModule
    {
    public:
        /**
         * DTOR
         */
        virtual ~IModule() = default;

    public:
        /**
         * The method \c GetName provides returns a elements (intentionally unique) identifier
         * which has previously been set through cModule::Create()
         *
         * @return The FEP Element's name.
         */
        virtual const char* GetName() const  = 0;

        /**
         * The method \c GetStateMachine provides access to the state machine.
         * See \ref fep_state_machine for more information about the FEP state machine.
         *
         * @returns  The state machine.
         */
        virtual fep::IStateMachine* GetStateMachine() const = 0;

        /**
         * The method \c GetPropertyTree provides access to the Property Tree interface
         * of the transmission layer.
         *
         * @returns  The transmission adapter.
         */
        virtual fep::IPropertyTree* GetPropertyTree() const = 0;

        /**
         * The method \c GetIncidentHandler provides access to the Incident Handler interface
         * of a particular FEP Element.
         *
         * @returns  The Incident Handler.
         */
        virtual fep::IIncidentHandler* GetIncidentHandler() const = 0;

        /**
         * The method \c GetNotificationAccess provides access to the INotificationAccess interface
         * of the transmission layer.
         *
         * @returns  The status access.
         */
        virtual fep::INotificationAccess* GetNotificationAccess() const = 0;

        /**
         * The method \c GetUserDataAccess provides access to the IDataAccess interface of the
         * transmission layer.
         *
         * @returns  The data access.
         */
        virtual fep::IUserDataAccess* GetUserDataAccess() const = 0;

        /**
         * The method \c GetCommandAccess provides access to the ICommandAccess interface of the
         * transmission layer.
         *
         * @returns  The command access.
         */
        virtual fep::ICommandAccess* GetCommandAccess() const = 0;

        /**
         * The method \c GetSignalRegistry provides access to the Central Signal Registry (CSR) 
         * of this FEP Element.
         *
         * @return Pointer to the CSR
         */
        virtual fep::ISignalRegistry* GetSignalRegistry() const = 0;

        /**
        * The method \c GetSignalMapping provides access to the Signal Mapping interface
        * of this FEP Element.
        * 
        * @return Pointer to the mapping component
        */
        virtual fep::ISignalMapping* GetSignalMapping() const = 0;

        /**
        * The method \c GetTimingInterface provides access to the Timing interface
        * of this FEP Element.
        * 
        * @return Pointer to the timing component interface
        */
        virtual fep::ITiming* GetTimingInterface() const = 0;

        /**
        * The method \c GetTimingMaster provides access to the Timing Master interface
        * of this FEP Element.
        *
        * @return Pointer to the timing master interface
        */
        virtual fep::ITimingMaster* GetTimingMaster() const = 0;

        /**
         * The method \c GetRPC provides the remote procedure call access interface
         * of this FEP Element.
         *
         * It is to register a object rpc server or to send rpc messages as a client.
         *
         * @return Pointer to the RPC component interface
         */
        virtual fep::IRPC* GetRPC() const = 0;

        /**
         * The method \c GetDomainId returns the currently used domain id used for 
         * this FEP Element. Only FEP Elements within the same domain id are able
         * to communicate/interact with each other.
         *
         * @return The current domain id value.
         */
        virtual uint16_t GetDomainId() const = 0;

        /**
         * !WARNING: currently not implemented - just a placeholder for future implementation!
         * The method \c GetDomainPrefix returns the currently used domain prefix used for
         * this FEP Element. Only FEP Elements with the same domain id are able
         * to communicate/interact with each other.
         *
         * @return The domain prefix
         * @retval Currently only an empty string!
         */
        virtual const char* GetDomainPrefix() const = 0;

        /**
         * @brief Get the Component registry of the module
         * 
         * @return fep::IComponents* 
         */
        virtual fep::IComponents* GetComponents() const = 0;
    };

    /**
     * @brief helper function to set the property within the module
     * 
     * @tparam T Type of the property 
     * @param module the module where the property tree is part of
     * @param path named path to the porperty
     * @param value the value to set
     * @return fep::Result 
     */
    template<typename T>
    fep::Result setProperty(IModule& module, const char* path, const T& value)
    {
        return setProperty<T>(*module.GetPropertyTree(), path, value);
    }

    /**
     * @brief helper function to get a property 
     * 
     * @tparam T Type of the property 
     * @param module the module 
     * @param path named path to the porperty
     * @param default_value defeault value if property is not found
     * @return T 
     */
    template<typename T>
    T getProperty(const IModule& module, const char* path, const T& default_value=T())
    {
        return getProperty<T>(*module.GetPropertyTree(), path, default_value);
    }

    /**
     * @brief helper function to retrieve a component from the modules components registry
     * 
     * @tparam INTERFACE the interface of the component
     * @param module teh module
     * @return INTERFACE*
     * @retval nullptr the interface was not found
     */
    template <class INTERFACE>
    INTERFACE* getComponent(const IModule& module)
    {
        return module.GetComponents()->getComponent<INTERFACE>();
    }

}
#endif // !defined(EA_E4D6E119_67FB_4cfe_A4BD_744871A82D28__INCLUDED_)
