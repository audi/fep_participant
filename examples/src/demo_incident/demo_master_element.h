/**
 *  Declaration of an exemplary FEP Master Element
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

// The base path for the "Master Strategy" configuration
#define MASTER_STRAT_ROOT_CONFIG "MyMasterStrategy"
// The path to the option whether the "Master Strategy" is set to be fussy or not.
#define MASTER_STRAT_PROP_BE_FUSSY MASTER_STRAT_ROOT_CONFIG".bBeFussyAboutEveryting"
// The path to the option of the "Master Strategy" which element is to be monitored more
// precisely.
#define MASTER_STRAT_CRITICAL_ELEMENT MASTER_STRAT_ROOT_CONFIG".strCriticalModule"

/// Simple forward declaration
class cMasterElement;

/*
 * Exemplary implementation of a FEP Incident Strategy to deal with specific
 * incidents (remote and local incidents) which require intervention. Plain and simple
 * logging is already provided by FEP built-in Strategies.
 */
class cMyMasterStrategy : public fep::IIncidentStrategy
{
public:
    cMyMasterStrategy(cMasterElement* pMasterInstance);
    ~cMyMasterStrategy();

public: // IIncidentStrategy interface
    fep::Result HandleLocalIncident(fep::IModule* pElementContext, const int16_t nIncident,
                                 const fep::tSeverityLevel eSeverity, const char* strOrigin,
                                 int nLine, const char* strFile,
                                 const timestamp_t tmSimTime, 
                                 const char* strDescription = NULL) override;
    fep::Result HandleGlobalIncident(const char *strSource, const int16_t nIncident,
                                 const fep::tSeverityLevel eSeverity,
                                 const timestamp_t tmSimTime,
                                 const char *strDescription) override;
    fep::Result RefreshConfiguration(const fep::IProperty *pStrategyProperty,
                                 const fep::IProperty *pAffectedProperty) override;

private:
    // local switch to enable general fussiness of this strategy.
    // It mostly demonstrates the FEP Property Tree integration of Incident Strategies.
    bool m_bBeFussy;
    // Keep track of a specific element which *must* work reliably. Others on the network
    // are less relevant as to care for it...
    std::string m_strCriticalElement;
    // The Master Element instance - used to access the transmission layer to be able
    // to transmit commands.
    cMasterElement* m_pMasterInstance;
};

/*
 * The cMasterElement class
 * An typical FEP Master Element responsible for controlling any attached FEP Slave Elements
 * and monitoring thereof.
 * For this example, its major purpose is to issue commands to remote elements and to implement
 * a custom strategy to handle remote incidents issued by a remote slave element.
 */
class cMasterElement: public fep::cModule
{
public:
    /// Default constructor
    cMasterElement();
    /// Default destructor
    virtual ~cMasterElement();

public: // IStateEntryListener interface
    fep::Result ProcessInitializingEntry(const fep::tState eOldState);
    fep::Result ProcessIdleEntry(const fep::tState eOldState);
    fep::Result ProcessReadyEntry(const fep::tState eOldState);
    fep::Result ProcessRunningEntry(const fep::tState eOldState);
    fep::Result ProcessStartupEntry(const fep::tState eOldState);
    fep::Result ProcessErrorEntry(const fep::tState eOldState);
    fep::Result ProcessShutdownEntry(const fep::tState eOldState);
    fep::Result CleanUp(const fep::tState eOldState);

public:
    fep::Result FireUpMaster();

    fep::Result HaltMaster();

    fep::Result InitializeSystem();

    fep::Result StartSystem();

    fep::Result HaltSystem();

    fep::Result FillInElementHeader();

private:
    // Our custom strategy implementation driving
    // a systems error handling.
    cMyMasterStrategy m_oCustomStrategy;
};

#endif // _FEP_DEMO_MASTER_ELEMENT_H_
