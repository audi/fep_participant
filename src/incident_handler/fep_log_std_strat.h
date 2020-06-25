/**
 * Declaration of the Class cLogConsoleStrategy.
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

#if !defined(EA_A23F4E6B_A181_4868_9FB4_411E7774A87C__INCLUDED_)
#define EA_A23F4E6B_A181_4868_9FB4_411E7774A87C__INCLUDED_

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <a_util/base/types.h>

#include "fep_result_decl.h"
#include "incident_handler/fep_incident_strategy_intf.h"
#include "incident_handler/fep_severity_level.h"
#include "incident_handler/logmanager.h"

namespace fep
{
    class IModule;
    class IProperty;

    /**
     * RT Compliant Console Log strategy dumping a string representation of
     * encountered incidents to stdout / stderr.
     */
    class cLogConsoleStrategy : public fep::IIncidentStrategy
    {
    public:
        /// Default constructor
        cLogConsoleStrategy();
        /// Default destructor
        virtual ~cLogConsoleStrategy();

    public: // IIncidentStrategy interface
        virtual fep::Result HandleLocalIncident(fep::IModule* pModuleContext, const int16_t nIncident,
                                       const tSeverityLevel severity,
                                       const char* strOrigin,
                                       int nLine,
                                       const char* strLine,
                                       const timestamp_t tmSimTime,
                                       const char* strDescription = NULL);
        virtual fep::Result HandleGlobalIncident(const char *strSource, const int16_t nIncident,
                                             const tSeverityLevel severity,
                                             const timestamp_t tmSimTime,
                                             const char *strDescription);
        virtual fep::Result RefreshConfiguration(const fep::IProperty* pStrategyProperty,
                                             const fep::IProperty* pAffectedProperty);

    private:
        /// @cond nodoc
        fep::RT::cLogMessageMgr m_oLogMessageMgr;
        std::string m_strLogString;
        std::string m_strHostname;
        std::recursive_mutex m_oLogGuard;
        bool m_bEnabled;
        /// @endcond nodoc
    };
}
#endif // !defined(EA_A23F4E6B_A181_4868_9FB4_411E7774A87C__INCLUDED_)
