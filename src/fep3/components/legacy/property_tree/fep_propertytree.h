/**
 * Declaration of the Class cPropertyTree.
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

#if !defined(EA_DC0D26F7_6442_49e1_980D_44C4881009EE__INCLUDED_)
#define EA_DC0D26F7_6442_49e1_980D_44C4881009EE__INCLUDED_

#include <cstddef>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <memory>
#include <a_util/base/types.h>
#include <a_util/result/result_type.h>

#include "fep3/components/base/component_base_legacy.h"
#include "fep3/components/base/component_intf.h"
#include "fep3/components/legacy/property_tree/fep_propertytree_intf.h"
#include "fep3/components/legacy/property_tree/property_listener_intf.h"
#include "fep_participant_export.h"
#include "fep_dptr.h"
#include "fep_result_decl.h"
#include "messages/fep_command_listener.h"
#include "messages/fep_notification_listener.h"
#include "messages/fep_notification_prop_changed_intf.h"

#include "fep3/components/rpc/fep_rpc.h"
#include "fep3/rpc_components/configuration/configuration_service.h"
#include "fep3/rpc_components/configuration/configuration_rpc_intf_def.h"

namespace fep
{
    class IDeletePropertyCommand;
    class IGetPropertyCommand;
    class IModule;
    class INameChangedNotification;
    class IProperty;
    class IRegPropListenerCommand;
    class ISetPropertyCommand;
    class IUnregPropListenerCommand;

    /**
     * This class is the implementation of the property tree.
     */
    class FEP_PARTICIPANT_EXPORT IPropertyTreePrivate
    {
    public:
        /**
        * DTOR
        */
        virtual ~IPropertyTreePrivate() = default;
        /// @copydoc IPropertyTree::SetRemotePropertyValue
        virtual fep::Result SetPropertyValue(char const * strPropPath, char const * strValue) = 0;
        /// @copydoc IPropertyTree::SetRemotePropertyValue
        virtual fep::Result SetPropertyValue(char const * strPropPath, double const f64Value) = 0;
        /// @copydoc IPropertyTree::SetRemotePropertyValue
        virtual fep::Result SetPropertyValue(char const * strPropPath, int32_t const n32Value) = 0;
        /// @copydoc IPropertyTree::SetRemotePropertyValue
        virtual fep::Result SetPropertyValue(char const * strPropPath, bool const bValue) = 0;
        /// @copydoc IPropertyTree::GetRemotePropertyValue
        virtual fep::Result GetPropertyValue(const char * strPropPath, const char *& strValue) const = 0;
        /// @copydoc IPropertyTree::GetRemotePropertyValue
        virtual fep::Result GetPropertyValue(const char * strPropPath, double & fValue) const = 0;
        /// @copydoc IPropertyTree::GetRemotePropertyValue
        virtual fep::Result GetPropertyValue(const char * strPropPath, int32_t & nValue) const = 0;
        /// @copydoc IPropertyTree::GetRemotePropertyValue
        virtual fep::Result GetPropertyValue(const char * strPropPath, bool & bValue) const = 0;
    };

    class FEP_PARTICIPANT_EXPORT cPropertyTree : public fep::IPropertyTree,
        public fep::cCommandListener, public fep::cNotificationListener,
        public fep::IPropertyTreePrivate
    {
        /// D-pointer to private implementation
        FEP_UTILS_D(cPropertyTree);

    public:
        /**
         * CTOR
         */
        cPropertyTree();

        /**
         * DTOR
         */
        virtual ~cPropertyTree();

        /**
         * The method \ref SetModule sets the associated module
         *
         * @param [in] pModule  the module
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_FAILED  Failed to set the module
         */
        fep::Result SetModule(const IModule* module);

    public: // implements IPropertyTree
        fep::Result SetPropertyValues(char const * strPropPath,
            char const *const * strFirstValue, size_t szArraySize);
        fep::Result SetPropertyValues(char const * strPropPath,
            double const * f64FirstValue, size_t szArraySize);
        fep::Result SetPropertyValues(char const * strPropPath,
            int32_t const * n32FirstValue, size_t szArraySize);
        fep::Result SetPropertyValues(char const * strPropPath,
            bool const * bFirstValue, size_t szArraySize);
        fep::Result SetRemotePropertyValue(char const * strElementName,
            char const * strPropPath, char const * strValue, const timestamp_t tmTimeout = 0);
        fep::Result SetRemotePropertyValue(char const * strElementName,
            char const * strPropPath, double const f64Value, const timestamp_t tmTimeout = 0);
        fep::Result SetRemotePropertyValue(char const * strElementName,
            char const * strPropPath, int32_t const n32Value, const timestamp_t tmTimeout = 0);
        fep::Result SetRemotePropertyValue(char const * strElementName,
            char const * strPropPath, bool const bValue, const timestamp_t tmTimeout = 0);
        fep::Result SetRemotePropertyValues(char const * strElementName,
            char const * strPropPath, char const *const * strFirstValue, size_t szArraySize,
            const timestamp_t tmTimeout = 0);
        fep::Result SetRemotePropertyValues(char const * strElementName,
            char const * strPropPath, double const * f64FirstValue, size_t szArraySize, 
            const timestamp_t tmTimeout = 0);
        fep::Result SetRemotePropertyValues(char const * strElementName,
            char const * strPropPath, int32_t const * n32FirstValue, size_t szArraySize, 
            const timestamp_t tmTimeout = 0);
        fep::Result SetRemotePropertyValues(char const * strElementName,
            char const * strPropPath, bool const * bFirstValue, size_t szArraySize, 
            const timestamp_t tmTimeout = 0);
        IProperty const * GetLocalProperty(char const * strPropPath) const;
        IProperty * GetLocalProperty(char const * strPropPath);
        fep::Result GetRemoteProperty(char const * strElementName, char const * strPropPath,
            IProperty ** pProperty, timestamp_t const tmTimeout);
        fep::Result MirrorRemoteProperty(char const * strElementName, char const * strRemotePath,
            char const * strLocalPath, timestamp_t const tmTimeout);
        fep::Result UnmirrorRemoteProperty(char const * strElementName, char const * strRemotePath,
            char const * strLocalPath, timestamp_t const tmTimeout);
        fep::Result DeleteRemoteProperty(
            char const * strElementName, char const * strPropPath);

    public: // implements IPropertyTree & IPropertyTreePrivate
        IProperty const * GetProperty(char const * strPropPath) const;
        IProperty * GetProperty(char const * strPropPath);
        fep::Result SetPropertyValue(char const * strPropPath, char const * strValue);
        fep::Result SetPropertyValue(char const * strPropPath, double const f64Value);
        fep::Result SetPropertyValue(char const * strPropPath, int32_t const n32Value);
        fep::Result SetPropertyValue(char const * strPropPath, bool const bValue);
        fep::Result GetPropertyValue(const char * strPropPath, const char *& strValue) const;
        fep::Result GetPropertyValue(const char * strPropPath, double & fValue) const;
        fep::Result GetPropertyValue(const char * strPropPath, int32_t & nValue) const;
        fep::Result GetPropertyValue(const char * strPropPath, bool & bValue) const;
        fep::Result RegisterListener(char const * strPropertyPath,
            IPropertyListener* const poListener);
        fep::Result UnregisterListener(char const * strPropertyPath,
            IPropertyListener* const poListener);
        fep::Result ClearProperty(char const * strPropertyPath);
        fep::Result DeleteProperty(char const * strPropertyPath);


    public: // override cCommandListener
        fep::Result Update(ISetPropertyCommand const * poCommand);
        fep::Result Update(IGetPropertyCommand const * poCommand);
        fep::Result Update(IDeletePropertyCommand const * poCommand);
        fep::Result Update(IRegPropListenerCommand const * poCommand);
        fep::Result Update(IUnregPropListenerCommand const * poCommand);

    public: // override cNotificationListener
         fep::Result Update(INameChangedNotification const * pNotification);

    private:
        /// this property listener proxies remote property changed notifications to other modules
        class cProxyPropertyChangedListener : public IPropertyListener
        {
        private:
            friend class cPropertyTree;
            /// name of receiver module
            std::string m_strRemoteModule;
            /// path of the property this one is listening to
            std::string m_strPath;
            /// Module access pointer
            const fep::IModule *  m_pModule;
            /// Local property pointer
            IProperty *  m_poProperty;
            /// holds the ref count (i.e how many subscriptions are active on this one)
            int m_nRefCount;

        private:
            /**
             * \c SendPropertyChangedNotification is a helper function that sends
             * the property changed notification in the event of a change.
             * @param [in] poProperty The property that was changed.
             * @param [in] ceEvent The event.
             * @retval Returns standard result.
             */
            fep::Result SendPropertyChangedNotification(const IProperty * poProperty,
                fep::IPropertyChangedNotification::tChangeEvent ceEvent);

        public:
            /// CTOR
            cProxyPropertyChangedListener(const char * strRemoteModule,
                const char * strPath, const fep::IModule * pModule, IProperty * poProperty);

            /// Increment refcount
            void Ref();
            /// Decrement refcount - returns false when refcunt reaches zero
            bool Unref();

            /**
             * \c SendFinalDelete sends the delete notification for this property
             */
            fep::Result SendFinalDelete();


        public: // implements IPropertyListener
            fep::Result ProcessPropertyAdd(IProperty const * poProperty,
                IProperty const * poAffectedProperty, char const * strRelativePath);
            fep::Result ProcessPropertyChange(IProperty const * poProperty,
                IProperty const * poAffectedProperty, char const * strRelativePath);
            fep::Result ProcessPropertyDelete(IProperty const * poProperty,
                IProperty const * poAffectedProperty, char const * strRelativePath);
        };

        /// holds information about a mirrored property
        struct sMirroredRemoteProperty
        {
            /// the module name
            std::string strElementName;
            /// the remote path of the property
            std::string strRemotePath;
            /// the local path of the property
            std::string strLocalPath;

            /**
             * Compares the mirrored property to another one for std::map.
             * @param [in] oOther The other one.
             * @retval Returns whether the other one is less than this one.
             */
            bool operator<(const sMirroredRemoteProperty & oOther) const
            {
                if (strElementName < oOther.strElementName)
                {
                    return true;
                }
                if (oOther.strElementName < strElementName)
                {
                    return false;
                }

                if (strRemotePath < oOther.strRemotePath)
                {
                    return true;
                }
                if (oOther.strRemotePath < strRemotePath)
                {
                    return false;
                }

                if (strLocalPath < oOther.strLocalPath)
                {
                    return true;
                }

                return false;
            }

            /**
             * Compares the mirrored property to another one.
             * @param [in] oOther The other one.
             * @retval Returns whether the other one is equal to this one.
             */
            bool operator==(const sMirroredRemoteProperty & oOther) const
            {
                return strElementName == oOther.strElementName &&
                    strRemotePath == oOther.strRemotePath &&
                    strLocalPath == oOther.strLocalPath;
            }
        };

        /// Each mirrored property has one of these listeners associated that listens
        /// for property changed notifications concerning this property.
        class cMirroredPropertyChangedNotificationListener :
            public fep::cNotificationListener
        {
        private:
            friend class cPropertyTree;
            /// holds information about the mirrored property
            const sMirroredRemoteProperty * m_poInfo;
            /// holds the mirrored property
            IProperty * m_poProperty;
            /// holds a reference to the property tree
            cPropertyTree * m_pTree;
            /// mutex to implement locking
            std::recursive_mutex m_mtxUpdates;

        private:
            /// finds the correct subproperty belonging to the path of the notification
            IProperty * FindAffectedProperty(const std::string & strPath,
                bool bEmptyRemPath);

        public:
            /// CTOR
            cMirroredPropertyChangedNotificationListener(const sMirroredRemoteProperty * poInfo,
                IProperty * poProperty, cPropertyTree * pTree);

            /// Locks the listener from doing any updates to the property tree
            fep::Result LockUpdates();

            /// Unlocks the listener from doing any updates to the property tree
            fep::Result UnlockUpdates();

        public: // override cNotificationListener
            fep::Result Update(fep::IPropertyChangedNotification const *
                pPropertyChangedNotification);
        };

        /// there can be multiple subscriptions on the same property in a provider module,
        /// hence the multimap
        typedef std::multimap<IProperty *, cProxyPropertyChangedListener*> tProxyMap;

        /// there can only be one subscription per property in the consumer module though
        typedef std::map<IProperty *, cMirroredPropertyChangedNotificationListener*>
            tMirroredChangeListener;

        /// The associated module
        const fep::IModule *  m_pModule;

        /// Holds the mirrored properties of this property tree
        std::map<sMirroredRemoteProperty, IProperty *> m_mapMirroredProperties;

        /// Mutex to secure m_mapProxyChangeListeners
        std::mutex m_oMapProxyChangeMutex;

        /// Mutex to secure have a workarund for a wrong implementation of *Remote* function
        /// these function usually needed to be const, but they are not!
        std::recursive_mutex m_oLockDamagedRemoteAccess;

        /// Holds the proxy listeners for remote change notification
        tProxyMap m_mapProxyChangeListeners;

        /// Holds the local listeners for remote change notification
        tMirroredChangeListener m_mapRemoteChangeListeners;

    private:
        /**
         * \c FindExistingProxyListener finds an existing proxy listener
         */
        tProxyMap::iterator FindExistingProxyListener(IProperty * pProperty,
            const char * strModule, const char * strPath);

        /**
        * \c DeletePropertyFromMap deletes the given property from m_mapProxyChangeListeners
        */
        void DeletePropertyFromMap(IProperty * pProperty);

        /**
        * \c RemoveMirroredPropertySubscription removes any subscription data
        * associated with the property.
        * @param [in] poProperty The property.
        * @param [in] poInfo Information about the subscription
        * @return Returns standard result.
        */
        fep::Result RemoveMirroredPropertyLocalSubscription(IProperty * poProperty,
            const sMirroredRemoteProperty & poInfo);

        /**
        * \c ElementNameChanged changes the name of the FEP Element during
        * runtime. Modify of the m_mapRemoteChangeListeners and
        * m_mapMirroredProperties to update element Name
        * @param [in] strOldElementName the recent Element Name.
        * @param [in] strNewElementName the new name
        * @return Returns standard result.
        */
        fep::Result ElementNameChanged(char const * strOldElementName,
            char const * strNewElementName);
    };

    namespace detail
    {
        class RPCConfigServer : public ::fep::rpc_object_server<::fep::rpc_stubs::RPCConfigurationServer, ::fep::rpc::IRPCConfigurationDef>
        {
            public:
                RPCConfigServer(cPropertyTree& tree);
                void normalizePathForTree(std::string& internal_property_path);
                std::string getProperties(const std::string& property_path);

                Json::Value getProperty(const std::string& property_path);
                int setProperty(const std::string& property_path,
                                const std::string& type,
                                const std::string& value);
                bool exists(const std::string& property_path);

            private:
                cPropertyTree& _tree;
        };

        class PropertyTreeComponent : public fep::ComponentBaseLegacy
        {
           public:
               PropertyTreeComponent(const IModule& module) : ComponentBaseLegacy(module) {}
               ~PropertyTreeComponent() = default;

           private:
                void* getInterface(const char* iid) override
                {                    
                    if (fep::getComponentIID<IPropertyTree>() == iid)
                    {
                        return static_cast<IPropertyTree*>(&_tree);
                    }
                    else
                    {
                        return nullptr;
                    }
                }
                fep::Result create() override
                {
                    _rpc_server.reset(new RPCConfigServer(_tree));
                    _module->GetRPC()->GetRegistry()->RegisterObjectServer(
                        fep::rpc::IRPCConfigurationDef::getRPCDefaultName(),
                        *_rpc_server.get());
                    return _tree.SetModule(_module);
                }
                fep::Result destroy() override
                {
                    auto rpc = _module->GetRPC();
                    if (rpc != nullptr)
                    {
                        rpc->GetRegistry()->UnregisterObjectServer(fep::rpc::IRPCConfigurationDef::getRPCDefaultName());
                    }
                    return _tree.SetModule(nullptr);
                }
                cPropertyTree _tree;
                std::unique_ptr<RPCConfigServer> _rpc_server;
        };
    }
}
#endif // !defined(EA_DC0D26F7_6442_49e1_980D_44C4881009EE__INCLUDED_)

