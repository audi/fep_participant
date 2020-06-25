/**
 * Declaration of the Class cIncidentHandler.
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

#if !defined(EA_846ABDF4_4417_4886_A322_3C9B73C9D92C__INCLUDED_)
#define EA_846ABDF4_4417_4886_A322_3C9B73C9D92C__INCLUDED_

#include <cassert>
#include <cstdint>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <a_util/base/types.h>
#include <a_util/regex/regularexpression.h>
#include <a_util/result/result_type.h>

#include "fep_result_decl.h"
#include "fep3/components/legacy/property_tree/property_intf.h"
#include "fep3/components/legacy/property_tree/property_listener_intf.h"
#include "fep_participant_export.h"
#include "fep_errors.h"
#include "incident_handler/fep_incident_handler_intf.h"
#include "incident_handler/fep_incident_strategy_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "messages/fep_notification_listener.h"

namespace fep
{
    class IIncidentNotification;
    class IModule;
    class IPropertyTree;
    class cIncidentHistoryStrategy;

    //################################################################################
    /**
     * @brief The cStrategyConfigWrapper class
     * Intermediate class used as a "proxy" by the FEP Incident Handler to manage
     * individual FEP Incident Strategies. The only purpose of this class is transparent
     * Property Listener Registration and De-Registration to and from their respective
     * configuration paths.
     */
    class cStrategyConfigWrapper : IPropertyListener
    {
    public:
    /// @cond nodoc
        cStrategyConfigWrapper(fep::IIncidentStrategy* pStrategyDelegate,
                               fep::IProperty* pProperty)
        {
            assert(NULL != pStrategyDelegate);
            assert(NULL != pProperty);

            m_pStrategyDelegate = pStrategyDelegate;
            m_pStratConfigBase = pProperty;
            m_pStratConfigBase->RegisterListener(this);
        }

        ~cStrategyConfigWrapper()
        {
            m_pStratConfigBase->UnregisterListener(this);
        }

        fep::IIncidentStrategy* GetWrappedStrategy()
        {
            return m_pStrategyDelegate;
        }

        fep::IProperty* GetStrategyConfig()
        {
            return m_pStratConfigBase;
        }

    /// @endcond nodoc

    public: // IPropertyListener interface
        /// implements interface from property tree package
        /// @copydoc IPropertyListener::ProcessPropertyAdd
        inline fep::Result ProcessPropertyAdd(const IProperty *poProperty,
            IProperty const * poAffectedProperty, char const * strRelativePath)
        {
            return ProcessPropertyChange(poProperty, poAffectedProperty, strRelativePath);
        }
        /// implements interface from property tree package
        inline fep::Result ProcessPropertyChange(const IProperty *poProperty,
            IProperty const * poAffectedProperty, char const * strRelativePath)
        {
            return m_pStrategyDelegate->RefreshConfiguration(m_pStratConfigBase, poAffectedProperty);
        }
        /// implements interface from property tree package
        inline fep::Result ProcessPropertyDelete(const IProperty *poProperty,
            IProperty const * poAffectedProperty, char const * strRelativePath)
        {
            // note: this will spam to the console when the property tree is being deleted...
            //LOG_ERROR("Deleting a configuration for FEP Incident Strategies is NOT supported!");
            return ERR_NOERROR; // has no significance here.
        }

    private:
        /// The respective base path within the FEP Property Tree on which to listen
        /// to for changes.
        fep::IProperty* m_pStratConfigBase;
        /// The wrapped incident strategy.
        fep::IIncidentStrategy* m_pStrategyDelegate;
    };

    //################################################################################
    
    /**
    * \brief The \ref IIncidentInvocationHandler interface provides a minimalistic interface
    * to the incident handler, providing only the incident invocation method. This is for interal
    * use and testing only.
    */
    class FEP_PARTICIPANT_EXPORT IIncidentInvocationHandler
    {
    public:

        /**
        * DTOR
        */
        virtual ~IIncidentInvocationHandler() = default;

        /**
        * Invocation of a FEP Incident to be processed and handled by registered / associated
        * FEP Incident Strategies. A call to this method may as well be regarded as
        * if throwing an exception or error but can also be used for logging purposes.
        *
        * @param [in] nFEPIncident The incident code that is to be invoked.
        * @param [in] severity The severity of the incident at hand.
        * @param [in] strDescription An description / log message for the
        * incident at hand.
        * @param [in] strOrigin The module that invoked the incident.
        * @param [in] nLine The line from where the incident was invoked
        * @param [in] strFile The filepath of the incident invoking file
        *
        * @warning Keep in mind that the 16bit range of the FEP Incident Code -
        * by definition -  is split into two ranges: Range -32768 to -1 may be used
        * for custom incidents whilst the range from 0 to 32767 is <b> exclusively</b>
        * reserved for FEP system-related incidents! Not respecting this convention may
        * impair the behavior of the <b>ENTIRE</b> FEP System including remote FEP Elements!
        *
        * @retval ERR_NOERROR Everything went as expected.
        * @retval ERR_INVALID_ARG Invalid incident code provided - most likely 0.
        * @retval ERR_NOT_INITIALISED No element context available (e.g. no cModule)
        * @retval ERR_NOT_READY The FEP Incident Handler has been disabled through its
        * configuration.
        */
        virtual fep::Result InvokeIncident(int16_t nIncidentCode, tSeverityLevel severity,
            const char* strDescription, const char* strOrigin,
            int nLine, const char* strFile) =0;
    };


    /**
     * @brief The cIncidentHandler class
     * The actual implementation of the FEP Incident Handler interface.
     */
    class FEP_PARTICIPANT_EXPORT cIncidentHandler : public fep::IIncidentHandler,
                                public fep::IPropertyListener,
                                public fep::cNotificationListener,
                                public fep::IIncidentInvocationHandler
    {

    public:
        /// CTOR
        cIncidentHandler();
        /// Default destructor
        ~cIncidentHandler();

        /**
         * The method \ref SetModule sets the associated module
         * 
         * @param [in] pModule  the module
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_FAILED
         */
        fep::Result SetModule(fep::IModule * pModule);

    public: // IIncidentHandler interface
        fep::Result AssociateStrategy(const int16_t nFEPIncident,
                                  const tIncidentStrategy eStrategyDelegate,
                                  const tStrategyAssociation eAssociation = SA_REPLACE);

        fep::Result AssociateStrategy(const int16_t nFEPIncident, IIncidentStrategy* pStrategyDelegate,
                                  const char* strConfigurationPath,
                                  const tStrategyAssociation eAssociation = SA_REPLACE);

        fep::Result AssociateCatchAllStrategy(IIncidentStrategy* pStrategyDelegate,
                                  const char* strConfigurationPath,
                                  const tStrategyAssociation eAssociation = SA_APPEND);

        fep::Result DisassociateStrategy(const int16_t nFEPIncident,
                                     IIncidentStrategy* pStrategyDelegate);
        fep::Result DisassociateCatchAllStrategy(IIncidentStrategy* pStrategyDelegate);

        fep::Result DisassociateStrategy(const int16_t nFEPIncident,
                                     const tIncidentStrategy eStrategyDelegate);

        public: //implements IIncidentInvocationHandler
        fep::Result InvokeIncident(int16_t nIncidentCode, tSeverityLevel severity,
                               const char* strDescription, const char* strOrigin,
                               int nLine, const char* strFile);

    public: // override cNotificationListener
        virtual fep::Result Update(fep::IIncidentNotification const * pIncidentNotification);

    public: // implements IPropertyListener
        fep::Result ProcessPropertyAdd(IProperty const * poProperty,
            IProperty const * poAffectedProperty, char const * strRelativePath);
        fep::Result ProcessPropertyChange(IProperty const * poProperty,
            IProperty const * poAffectedProperty, char const * strRelativePath);
        fep::Result ProcessPropertyDelete(IProperty const * poProperty,
            IProperty const * poAffectedProperty, char const * strRelativePath);

    public:
        // IIncidentHandler interface - continued
        fep::Result GetLastIncident(const fep::tIncidentEntry** ppIncidentEntry);
        fep::Result RetrieveIncidentHistory(tIncidentListConstIter& io_iterHistBegin,
                                        tIncidentListConstIter& io_iterHistEnd);
        fep::Result FreeIncidentHistory();
    
    public:
        /// Updates the configuration of all known global strategies
        fep::Result RefreshConfiguration();

    public: //Used in CleanUp to reset to default properties
        fep::Result SetDefaultProperties();

    /// @cond nodoc
    private:
        // only used internally to process received global incidents
        fep::Result ForwardIncident(int16_t nIncidentCode, tSeverityLevel severity,
                               const char* strSource, const timestamp_t tmSimTime, const char* strDescription);
        bool StrategyIsAssociatedAnywhere(IIncidentStrategy* pStrategyHandle,
                                   cStrategyConfigWrapper** pConfigWrapper);
        fep::Result GetConfigWrapperReference(IIncidentStrategy* pStrategyHandle,
                                          cStrategyConfigWrapper** pConfigWrapper);

    private:
        // catch-all strategy map
        typedef std::map<IIncidentStrategy*, bool> tCAStrategyMap;
        typedef tCAStrategyMap::iterator tCAStrategyMapIter;
        tCAStrategyMap m_mapCatchAllDelegates;

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic ignored "-Wattributes" // standard type attributes are ignored when used in templates
#endif

        // individually associated strategies
        typedef std::multimap<const int16_t, IIncidentStrategy*> tStrategyMap;
        typedef tStrategyMap::iterator tStrategyMapIter;
        tStrategyMap m_mapIncidentDelegates;

#if defined(__GNUC__) && (__GNUC__ == 5) && defined(__QNX__)
#pragma GCC diagnostic warning "-Wattributes" // standard type attributes are ignored when used in templates
#endif

        // collection of all known strategies. Also handles configuration management
        // and uniqueness of strategies.
        typedef std::set<cStrategyConfigWrapper*> tGlobalStrategyMap;
        typedef tGlobalStrategyMap::iterator tGlobalStrategyIter;
        tGlobalStrategyMap m_setGlobalStrategies;

        fep::IModule* m_pFEPModule;
        fep::IPropertyTree* m_pPropertyTree;
        std::recursive_mutex m_oAssocMutex;
        std::recursive_mutex m_oPropConfigMutex;

        IIncidentStrategy* m_pLogStratRef;
        IIncidentStrategy* m_pFileStratRef;
        IIncidentStrategy* m_pNotifStratRef;

        // The incident handler acts as a proxy when it comes to retrieving the recorded
        // history of the historystrategy; IIncidentStrategy cannot provide this so
        // this particular strategy is referenced directly
        cIncidentHistoryStrategy* m_pHistoryStratRef;
        tIncidentListConstIter m_oCurrentHistoryIter;

        bool m_bEnabled;
        bool m_bEnableGlobalScope;
        std::string m_strFilterGlobalSource;
        a_util::regex::RegularExpression m_oSourceMatcher;

    /// @endcond nodoc
    };
}
#endif // !defined(EA_846ABDF4_4417_4886_A322_3C9B73C9D92C__INCLUDED_)
