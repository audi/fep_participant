/**
 *  Declaration of a FEP Master Element that does error recovery.
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
 */

#ifndef _FEP_DEMO_MASTER_ELEMENT_H_
#define _FEP_DEMO_MASTER_ELEMENT_H_

// The master element in this example
class cMasterElement: public fep::cModule,
    public fep::IIncidentStrategy,
    public fep::cNotificationListener
{
public:
    // Default constructor
    cMasterElement();

    // Default destructor
    virtual ~cMasterElement();

public: // IStateEntryListener interface
    fep::Result ProcessStartupEntry(const fep::tState eOldState);
    fep::Result ProcessInitializingEntry(const fep::tState eOldState);
    fep::Result ProcessIdleEntry(const fep::tState eOldState);
    fep::Result ProcessReadyEntry(const fep::tState eOldState);
    fep::Result ProcessRunningEntry(const fep::tState eOldState);
    fep::Result CleanUp(const fep::tState eOldState);
    fep::Result ProcessShutdownEntry(const fep::tState eOldState);
    fep::Result ProcessErrorEntry(const fep::tState eOldState);

public: // override cNotificationListener
    fep::Result Update(fep::IStateNotification const * pNotification);

public: // IIncidentStrategy interface
    fep::Result HandleLocalIncident(fep::IModule *pElementContext, const int16_t nIncident,
        const fep::tSeverityLevel eSeverity, const char *strOrigin, int nLine, const char *strFile,
                                const timestamp_t tmSimTime, const char *strDescription);
    fep::Result HandleGlobalIncident(const char *strSource, const int16_t nIncident,
        const fep::tSeverityLevel eSeverity,
        const timestamp_t tmSimTime, const char *strDescription);
    fep::Result RefreshConfiguration(const fep::IProperty *pStrategyProperty,
        const fep::IProperty *pAffectedProperty);

private:
    fep::Result FillElementHeader();

private:
    // Indicates that the slave has been initialized before
    bool m_bInitSlave;
};

#endif // _FEP_DEMO_MASTER_ELEMENT_H_
