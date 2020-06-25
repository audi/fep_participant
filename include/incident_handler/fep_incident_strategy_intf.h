/**
 * Declaration of the Class IIncidentStrategy.
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

#if !defined(EA_99D236D5_4790_45b2_8F51_FE5CCD3639BC__INCLUDED_)
#define EA_99D236D5_4790_45b2_8F51_FE5CCD3639BC__INCLUDED_

#include "fep_types.h"
#include "incident_handler/fep_severity_level.h"
#include "fep3/components/legacy/property_tree/property_intf.h"

namespace fep
{
    /// forward declaration of IModule
    class IModule;

    /**
     * @brief The IIncidentStrategy class
     * Interface for FEP Incident Strategy implementations. For details refer to
     * page @ref fep_incident_handling, ff.
     */
    class FEP_PARTICIPANT_EXPORT IIncidentStrategy
    {
    public:
        /// Virtual default destructor
        virtual ~IIncidentStrategy() = default;

    public:
        /**
         * Central method which is being called by the FEP Incident Handler
         * (IIncidentHandler) upon associating a strategy delegate implementing this
         * interface. This callback is limited to <b> local </b> incidents only! To handle
         * incidents issued by remote FEP Elements, implement HandleGlobalIncident()
         *
         * @param [in] pElementContext The FEP Element context in which the given incident
         * has been invoked. Depending on the incident code nIncident, the strategy may
         * chose to drive the IModules State Machine as well as to reset the Element's
         * configuration as needed.
         *
         * @param [in] nIncident The invoked incident code.
         * Range -1 to 32768 may be used for custom incidents whilst the
         * range from 0 to 32767 is <b> exclusively</b> reserved for FEP system-related
         * incidents!
         * @param [in] eSeverity The severity level of the received incident.
         * @param [in] strDescription An optional description of the invoked incident.
         * @param [in] strOrigin The module that invoked the incident. This is NOT the fep element.
         * @param [in] nLine The line from where the incident was invoked
         * @param [in] strFile The filepath of the incident invoking file
         * @param [in] tmSimTime Time stamp of the simulation time at the incident invokation.
         *
         * @return User-defined return code; the value is not evaluated by the
         * IIncidentHandler!
         */
        virtual fep::Result HandleLocalIncident(
                IModule* pElementContext, const int16_t nIncident,
                const fep::tSeverityLevel eSeverity,
                const char* strOrigin,
                int nLine,
                const char* strFile,
                const timestamp_t tmSimTime,
                const char* strDescription = NULL) =0;

        /**
         * Central method which is being used by the FEP Incident Handler
         * (IIncidentHandler) upon associating a strategy delegate implementing this
         * interface. This callback is limited to <b> global </b> incidents only! To handle
         * incidents issued the own FEP Element only, implement HandleLocalIncident()
         *
         * @param [in] strSource The name of the issuing FEP Element.
         * @param [in] nIncident The invoked incident code.
         * Range -1 to 32768 may be used for custom incidents whilst the
         * range from 0 to 32767 is <b> exclusively</b> reserved for FEP system-related
         * incidents!
         * @param [in] eSeverity The severity level of the received incident.
         * @param [in] strDescription An optional description of the invoked incident.
         * @param [in] tmSimTime Time stamp of the simulation time at the incident invokation.
         *
         * @return User-defined return code; the value is not evaluated by the
         * IIncidentHandler!
         */
        virtual fep::Result HandleGlobalIncident(const char* strSource, const int16_t nIncident,
                                             const fep::tSeverityLevel eSeverity,
                                             const timestamp_t tmSimTime,
                                             const char* strDescription = NULL
                                             ) =0;

        /**
         * Optional configuration management for a custom FEP Incident Strategy.
         * The FEP Incident Handler expects a configuration path when associating a
         * FEP Incident Strategy for the first time. This path is being monitored by
         * the FEP Incident Handler and changes are being forwarded to the respective
         * Incident Delegate. <br>
         * This mechanism is optional and is intended to provide means to allow end-user
         * configuration of custom Incident Strategies.
         *
         * @param [in] pStrategyProperty The root property which has been specified
         * during IIncidentHandler::AssociateStrategy(). This will serve as a reference.
         * @param pAffectedProperty The property along the path of the pStrategyProperty root
         * property which has actually been affected by a change or has been added to the
         * branch of pStrategyProperty.
         *
         * \note: In case the root property of the Strategy has been altered,
         * pStrategyProperty and pAffectedProperty reference the very same property.
         *
         * @return User-defined return code; the value is (!) evaluated by the
         * IIncidentHandler and will be returned by the corresponding call to
         * GetPropertyTree()->SetPropertyValue("MyStrat.Prop.Entry", "testvalue")!
         */
        virtual fep::Result RefreshConfiguration(
                const fep::IProperty* pStrategyProperty,
                const fep::IProperty* pAffectedProperty) =0;
    };
}
#endif // !defined(EA_99D236D5_4790_45b2_8F51_FE5CCD3639BC__INCLUDED_)
