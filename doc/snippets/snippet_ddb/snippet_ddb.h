/**
 *  Declaration of an exemplary FEP Base Participant
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
 * @file
 *
 */

#ifndef _FEP_SNIPPET_MODULE_H_
#define _FEP_SNIPPET_MODULE_H_

class cMyElement: public fep::cModule,
                 public fep::ISyncListener
{

public:
    /// Default constructor
    cMyElement();
    /// Default destructor
    virtual ~cMyElement();

public: // IStateEntryListener interface
    fep::Result ProcessInitializingEntry(const fep::tState eOldState);
    fep::Result ProcessIdleEntry(const fep::tState eOldState);
    fep::Result ProcessReadyEntry(const fep::tState eOldState);
    fep::Result ProcessRunningEntry(const fep::tState eOldState);
    fep::Result ProcessStartupEntry(const fep::tState eOldState);
    fep::Result ProcessErrorEntry(const fep::tState eOldState);

public: // ISyncListener interface
    fep::Result ProcessDDBSync(handle_t const hSignal,
                           const fep::IDDBFrame& oDDBFrame);

public: // IIncidentStrategy interface
    fep::Result HandleLocalIncident(const int16_t nIncident,
                                                const fep::tSeverityLevel eSeverity,
                                                const char *strOrigin,
                                                int nLine,
                                                const char *strFile,
                                                const timestamp_t tmSimTime,
                                                const char* strDescription = NULL);
private:
    /// @cond nodoc
    handle_t m_hMySignalHandle;
    IDDBAccess* m_pDDBAccess;
    bool m_bDDBFrameInconsistent;
    /// @endcond
};

#endif //_FEP_SNIPPET_MODULE_H_
