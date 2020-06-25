/**
 * Declaration of the Class IIncidentHandler.
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

#if !defined(EA_834FCDEE_84A2_4145_8DCD_75DED40C96E8__INCLUDED_)
#define EA_834FCDEE_84A2_4145_8DCD_75DED40C96E8__INCLUDED_

#include <deque>
#include "fep_types.h"
#include "fep_severity_level.h"
#include "fep_incident_strategy_intf.h"

/// Definition of the maximum length of description strings in serialized FEP Incidents
#define ENTRY_MESSAGE_LENGTH 512
/// Definition of the maximum length of source name strings in serialized FEP Incidents
#define SOURCE_NAME_LENGTH 64
/// Definition of the maximum length of the origin-component name in serialized FEP Incidents
#define ENTRY_ORIGIN_LENGTH 128

namespace fep
{
    /// Enumeration of available association options for the Incident Handler interface.
    typedef enum eStrategyAssociation
    /* see "rules for changing an enumeration" (#27200) before doing any change! */
    {
        /// Flag to replace all strategies associated with a specific
        /// incident code by a new one.
        SA_REPLACE = 0,
        /// Flag to append a strategy to an incident code
        SA_APPEND  = 1
    } tStrategyAssociation;

    /// Structured description of a FEP Incident data set. This structure
    /// is used for serialization / storage and primarily used by the History Log
    /// feature. See IIncidentHandler::RetrieveIncidentHistory() and the corresponding
    /// SDK documentation for more details.
    typedef struct sIncidentEntry
    {
        /// The (Unix compatible) timestamp of when the incident has been recorded.
        timestamp_t  nTimeStamp;
        /// The incident code.
        int16_t      nIncident;
        /// The incident's severity level
        fep::tSeverityLevel eSeverity;
        /// Fixed-length source identifier of the incident's origin;
        /// usually a FEP Element instance.
        char strSource[SOURCE_NAME_LENGTH];
        /// Fixed-length description of the recorder incident if any is available.
        /// For RT-compliance the length is capped for serialization.
        char strMessage[ENTRY_MESSAGE_LENGTH];
        /// Fixed-length string describing the origin-component of the incident.
        char strOrigin[ENTRY_ORIGIN_LENGTH];
        /// Time stamp of the simulation time at the incident invokation.
        timestamp_t tmSimTime;
    } tIncidentEntry;

    /// Enumeration enlisting shortcut references to built-in Incident Strategies.
    typedef enum eIncidentStrategy
    /* see "rules for changing an enumeration" (#27200) before doing any change! */
    {
        /// References generalized File Log Strategy
        ES_LogFile = 0,
        /// References generalized Console Log Strategy
        ES_LogConsole = 1,
        /// References generalized History Log Strategy
        ES_LogHistory = 2,
        /// References generalized Notification Log Strategy
        ES_LogNotification = 3
    } tIncidentStrategy;

    /// Definition of a non-constant iterator access to Incident History entries.
    typedef std::deque<tIncidentEntry>::iterator tIncidentListIter;
    /// Definition of a constant iterator access to Incident History entries.
    typedef std::deque<tIncidentEntry>::const_iterator tIncidentListConstIter;

    /**
     * @brief The IIncidentHandler class
     * Interface for the FEP Incident Handler implementation. For details refer to
     * page @ref fep_incident_handling.
     */
    class FEP_PARTICIPANT_EXPORT IIncidentHandler
    {
    public:
        /// Virtual Destructor
        virtual ~IIncidentHandler() {}

        /**
         * Association of any of the built-in FEP Incident Strategy to an incident code.
         *
         * @param [in] nFEPIncident An arbitrary incident code to associate the given
         * strategy with.
         * @param [in] eStrategyDelegate The shortcut-reference to a built-in strategy
         * @param [in] eAssociation The way the strategy is to be associated.
         *
         * @retval ERR_NOERROR Everything went as expected.
         * @retval ERR_NOT_SUPPORTED Given shortcut-reference is unknown.
         * @retval ERR_INVALID_INDEX The incident code was '0' (which is reserved and unused)
         * @return @see AssociateStrategy(const int16_t, IIncidentStrategy*,
         *                                const char*, const tStrategyAssociation);
         */
        virtual fep::Result AssociateStrategy(const int16_t nFEPIncident,
                            const fep::tIncidentStrategy eStrategyDelegate,
                            const fep::tStrategyAssociation eAssociation) =0;

        /**
         * Association of a customized FEP Incident Strategy delegate to an incident code.
         *
         * @param [in] nFEPIncident An arbitrary incident code to associate the given
         * strategy with.
         * @param [in] pStrategyDelegate An IIncidentStrategy* delegate to be associated.
         * @param [in] strConfigurationPath An optional configuration path through which
         * the strategy may be configured by the user during runtime. The Configuration
         * is stored in the FEP Property Tree of the hosting module/element. The strategy delegate
         * in pStrategyDelegate is being notified of any changes made to this path:
         * See IIncidentStrategy::RefreshConfiguration().
         * @param [in] eAssociation The way the strategy is to be associated.
         *
         * \note The same Incident Strategy may be registered multiple times. Association
         * as catch-all strategy does not impair association to a specific incident code.
         * Regardless of how and when a strategy is being associated by the user, the
         * FEP Incident Handler will ensure that a strategy delegate is only being called
         * once per incident.
         *
         * * \note The FEP Incident Handler does NOT take ownership of associated strategies.
         * It is within the responsibility of the developer to disassociate and delete
         * self-associated delegates.
         *
         * @retval ERR_NOERROR Everything went as expected.
         * @retval ERR_NOT_INITIALISED The FEP Element Context is not available (e.g. NULL)
         * @retval ERR_INVALID_INDEX The incident code was '0' (which is reserved and unused)
         * @retval ERR_POINTER The strategy delegate is NULL;
         * @retval ERR_INVALID_ARG The configuration path is NULL.
         * @retval ERR_RESOURCE_IN_USE The strategy delegate has already been associated
         * with the given incident code.
         */
        virtual fep::Result AssociateStrategy(
                const int16_t nFEPIncident,
                IIncidentStrategy* pStrategyDelegate,
                const char* strConfigurationPath = "",
                const fep::tStrategyAssociation eAssociation = SA_REPLACE) = 0;

        /**
         * Association of a customized FEP Incident Strategy as "catch-all" delegate to any
         * encountered incident.
         *
         * @param [in] pStrategyDelegate An IIncidentStrategy* delegate to be associated.
         * @param [in] strConfigurationPath An optional configuration path through which
         * the strategy may be configured by the user during runtime. The Configuration is
         * stored in the FEP Property Tree of the hosting module/element. The strategy delegate in
         * pStrategyDelegate is being notified of any changes made to this path: See
         * IIncidentStrategy::RefreshConfiguration().
         * @param [in] eAssociation The way the strategy is to be associated.
         *
         * \note The same Incident Strategy may be registered multiple times. Association
         * as catch-all strategy does not impair association to a specific incident code.
         * Regardless of how and when a strategy is being associated by the user, the
         * FEP Incident Handler will ensure that a strategy delegate is only being called
         * once per incident.
         *
         * \note The FEP Incident Handler does NOT take ownership of associated strategies.
         * It is within the responsibility of the developer to disassociate and delete
         * self-associated delegates.
         *
         * @retval ERR_NOERROR Everything went as expected.
         * @retval ERR_NOT_INITIALISED The FEP Element Context is not available (e.g. NULL)
         * @retval ERR_POINTER The strategy delegate is NULL;
         * @retval ERR_INVALID_ARG The configuration path is NULL.
         * @retval ERR_RESOURCE_IN_USE The strategy delegate has already been associated
         * as catch-all strategy.
         */
        virtual fep::Result AssociateCatchAllStrategy(
                IIncidentStrategy* pStrategyDelegate,
                const char* strConfigurationPath,
                const fep::tStrategyAssociation eAssociation = SA_APPEND) =0;

        /**
         * Disassociation of a customized FEP Incident Strategy.
         *
         * @param [in] nFEPIncident The incident code the pStrategyDelegate had previously
         * been associated with.
         * @param [in] pStrategyDelegate The strategy delegate that is to be disassociated.
         *
         * \note This method does NOT delete the strategy; it merely removes its internal
         * reference to it. To avoid memory leaks, the user must delete the instance manually.
         *
         * @retval ERR_NOERROR Everything went as expected.
         * @retval ERR_POINTER The strategy delegate is NULL;
         * @retval ERR_NOT_FOUND The strategy is not found to have been associated with
         * the given incident code.
         */
        virtual fep::Result DisassociateStrategy(
                const int16_t nFEPIncident,
                IIncidentStrategy* pStrategyDelegate) =0;

        /**
         * Disassociation of a built-in FEP Incident Strategy.
         *
         * @param [in] nFEPIncident The incident code the pStrategyDelegate had previously
         * been associated with.
         * @param [in] eStrategyDelegate Shortcut-reference to a built-in strategy that is
         * to be disassociated.
         *
         * @retval ERR_NOERROR Everything went as expected.
         * @retval ERR_INVALID_ARG The shortcut-reference is unknown.
         * @retval ERR_NOT_FOUND The strategy is not found to have been associated with
         * the given incident code.
         */
        virtual fep::Result DisassociateStrategy(
                const int16_t nFEPIncident,
                const fep::tIncidentStrategy eStrategyDelegate) =0;

        /**
         * Disassociation of a customized FEP Incident Strategy as catch-all delegate.
         *
         * @param [in] pStrategyDelegate Reference to the strategy delegate that is to be
         * disassociated.
         *
         * @retval ERR_NOERROR Everything went as expected.
         * @retval ERR_POINTER The strategy delegate is NULL;
         * @retval ERR_NOT_FOUND The strategy is not found to have been associated.
         */
        virtual fep::Result DisassociateCatchAllStrategy(
                IIncidentStrategy* pStrategyDelegate) =0;

        /**
         * Invocation of a FEP Incident to be processed and handled by registered / associated
         * FEP Incident Strategies. A call to this method may as well be regarded as
         * if throwing an exception or error but can also be used for logging purposes.
         *
         * @param [in] nFEPIncident The incident code that is to be invoked.
         * @param [in] eSeverity The severity of the incident at hand.
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
        virtual fep::Result InvokeIncident(int16_t nFEPIncident,
                                       fep::tSeverityLevel eSeverity,
                                       const char* strDescription,
                                       const char* strOrigin,
                                       int nLine,
                                       const char* strFile) =0;

        /**
         * Access method to retrieve the latest incident recorded by the internal
         * FEP Incident History Strategy.
         *
         * \note To use this feature, please make sure the FEP Incident History has
         * been enabled and configured according to @ref sec_history_log_strategy on page
         * @ref fep_incident_handling.
         *
         * @param [in,out] ppIncidentEntry Reference to a pointer to a serialized FEP
         * Incident.
         *
         * \note This method is RT-safe but NOT MT-safe. The retrieved reference is
         * always valid but the contents are only valid up until the next call to
         * GetLastIncident().
         *
         * @retval ERR_NOERROR Everything went as expected.
         * @retval ERR_POINTER Invalid pointer given.
         * @retval ERR_NOT_READY The FEP Incident History Strategy is disabled.
         * @retval ERR_EMPTY History Log Strategy either deactivated or no incidents
         * on record.
         */
        virtual fep::Result GetLastIncident(
                const fep::tIncidentEntry** ppIncidentEntry) =0;

        /**
         * Access method to retrieve the full set of incident records from the
         * internal FEP Incident History.
         *
         * To "scroll" or iterate through the current history records, simply use common
         * c++ stl techniques. The first iterator is holding the <i>oldest</i> entry on record.
         *
         * Example:
         * \snippet snippet_incident_element.cpp HistoryStratRetrieveHistory
         *
         * \note History records can generally only be retrieved once! By calling this method
         * the memory is locked for exclusive access by the calling context. To finish
         * processing through the history, FreeIncidentHistory() must be called. This will
         * release the lock and <b>purges</b> the collected history up until io_iterHistEnd.
         *
         * \warning Failing to call FreeIncidentHistory() can lead to crashes on destruction!
         *
         * \note Locking the history for processing does NOT impair the continued recording
         * of incidents! To retrieve and collect history recorded while the lock had been imposed,
         * simply free the incident history and call RetrieveIncidentHistory again.
         *
         * \note This method is RT-safe AND MT-safe due to an internal locking mechanism.
         *
         * @param [in,out] io_iterHistBegin Iterator pointing to the <i>oldest</i>
         * entry in the current set of records.
         * @param [in,out] io_iterHistEnd Iterator pointing to the end of the current
         * set or records.
         *
         * @retval ERR_NOERROR Everything went as expected; History records are available.
         * @retval ERR_NOT_READY The FEP Incident History Strategy is disabled.
         * @retval ERR_DEVICE_IN_USE The history is (still) locked by a previous call.
         * Call FreeIncidentHistory() to release the lock.
         * @retval ERR_ACCESS_DENIED Something else went wrong.
         */
        virtual fep::Result RetrieveIncidentHistory(
                fep::tIncidentListConstIter& io_iterHistBegin,
                fep::tIncidentListConstIter& io_iterHistEnd) =0;

        /**
         * Releasing the lock by a previous call to RetrieveIncidentHistory and purging
         * the history up until the last entry which has been supplied by
         * RetrieveIncidentHistory.
         *
         * @retval ERR_NOERROR Everything went as expected.
         * @retval ERR_NOACCESS Unable to unlock; The lock does NOT belong to the
         * current thread context.
         */
        virtual fep::Result FreeIncidentHistory() = 0;
    };

}
#endif // !defined(EA_834FCDEE_84A2_4145_8DCD_75DED40C96E8__INCLUDED_)
