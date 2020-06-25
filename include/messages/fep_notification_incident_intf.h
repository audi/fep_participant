/**
 * Declaration of the Class ILogNotification.
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

#if !defined(EA_09AAF507_F39D_4cc6_BE73_219FD0594DCF__INCLUDED_)
#define EA_09AAF507_F39D_4cc6_BE73_219FD0594DCF__INCLUDED_

#include "incident_handler/fep_severity_level.h"
#include "messages/fep_notification_intf.h"

namespace fep
{
    /**
     * Base interface of incident notifications in FEP
     */
    class FEP_PARTICIPANT_EXPORT IIncidentNotification : public INotification
    {

    public:
        /**
         * DTOR
         */
        virtual ~IIncidentNotification() = default;

        /**
         * The method \c GetSeverity returns the log level of the contained log message.
         * 
         * @returns  The log level.
         */
        virtual tSeverityLevel GetSeverity() const =0;

        /**
         * The method \c GetDescription returns the incident message.
         * 
         * @returns  The message.
         */
        virtual char const * GetDescription() const =0;

        /**
         * The method \c GetIncidentCode return the supplied incident code if any
         * has been assigned to this notification. In case this notification is not
         * releated to any specific incidence, GetIncidentCode will return 0;
         *
         * @return An incidence code if available. 0 if this is not the case.
         */
        virtual int16_t GetIncidentCode() const =0;
    };
} // namespace fep
#endif // !defined(EA_09AAF507_F39D_4cc6_BE73_219FD0594DCF__INCLUDED_)
