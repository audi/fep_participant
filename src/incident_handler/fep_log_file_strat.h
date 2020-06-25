/**
 * Declaration of the Class cLogFileStrategy.
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

#if !defined(EA_67B6F12A_01E2_47c0_888F_D4FBAA9BE452__INCLUDED_)
#define EA_67B6F12A_01E2_47c0_888F_D4FBAA9BE452__INCLUDED_

#include <cstdint>
#include <cstdio>
#include <mutex>
#include <string>
#include <a_util/base/types.h>
#include <a_util/filesystem/path.h>

#include "fep_result_decl.h"
#include "incident_handler/fep_incident_strategy_intf.h"
#include "incident_handler/fep_severity_level.h"

namespace fep
{
    class IModule;
    class IProperty;

    /**
     * Delegate handling the logging of incidents to a specified log file
     */
    class cLogFileStrategy : public fep::IIncidentStrategy
    {

    public:
        /// Default constructor
        cLogFileStrategy();
        /// Default destructor
        virtual ~cLogFileStrategy();

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
        /**
         * Opening of a log file specified by strPath. The opening mode depends
         * on the strategy configuration (FEP Property Tree)
         * @param [in] strPath Relative or absolute path to log file.
         * @retval ERR_NOERROR Everything went as expected.
         */
        fep::Result OpenLogFile(const a_util::filesystem::Path& strPath);

    private:
        /// @cond nodoc
        FILE* m_pLogFile;
        a_util::filesystem::Path m_strLogFilePath;
        bool m_bEnabled;
        bool m_bEnableCSVFormat;
        bool m_bOverwriteExisting;
        std::recursive_mutex m_oConfigGuard;
        std::string m_strLogString;
        std::string m_strHostname;
        /// @endcond nodoc
    };
}
#endif // !defined(EA_67B6F12A_01E2_47c0_888F_D4FBAA9BE452__INCLUDED_)
