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

#ifndef __SNIPPET_INCIDENT_ELEMENT_H_
#define __SNIPPET_INCIDENT_ELEMENT_H_

/// just to avoid using the utils.
class cSnippetSocket
{
public:
    fep::Result Close() { return fep::ERR_NOERROR; }
};

/// Simple forward declaration
class cMyElement;

/**
 * @brief The cMyMasterStrategy class
 * Exemplary implementation of a FEP Incident Strategy to deal with specific
 * incidents (remote and local incidents) which require intervention. Plain and simple
 * logging is already provided by FEP built-in Strategies.
 */
class cMyStrategy : public fep::IIncidentStrategy
{
public:
    /// Default constructor
    cMyStrategy();
    /// Default constructor with context
    cMyStrategy(cMyElement* pMasterInstance);
    /// Default destructor
    ~cMyStrategy();

public: // IIncidentStrategy interface
    fep::Result HandleLocalIncident(fep::IModule *pElementContext, const int16_t nIncident,
                                const tSeverityLevel eSeverity,
                                const char *strOrigin,
                                int nLine,
                                const char *strFile,
                                const timestamp_t tmSimTime,
                                const char *strDescription);
    fep::Result HandleGlobalIncident(const char *strSource, const int16_t nIncident,
                                 const tSeverityLevel eSeverity,
                                 const timestamp_t tmSimTime,
                                 const char *strDescription);
    fep::Result RefreshConfiguration(const fep::IProperty *pStrategyProperty,
                                 const fep::IProperty *pAffectedProperty);

private:
    /// local switch to enable general fussiness of this strategy.
    /// It mostly demonstrates the FEP Property Tree integration of Incident Strategies.
    bool m_bBoolOpt;
    /// Keep track of a specific FEP Participant which *must* work reliably. Others on the network
    /// are less relevant as to care for it...
    std::string m_strStringOpt;
    /// The Master FEP Participant instance - used to access the transmission layer to be able
    /// to transmit commands.
    cMyElement* m_pElementContext;
    /// Any command access - from any arbitrary FEP Participants.
    fep::ICommandAccess* m_pCommandAccess;
};

class cMyElement: public fep::cModule
{
    friend class cMyStrategy;

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
    fep::Result ProcessShutdownEntry(const fep::tState eOldState);

private: // overrides cModule
    fep::Result HandleLocalIncident(const int16_t nIncident,
                                const  fep::tSeverityLevel eSeverity,
                                const char *strOrigin,
                                int nLine,
                                const char *strFile,
                                const timestamp_t tmSimTime,
                                const  char* strDescription = NULL);

public:
    fep::Result GetLastIncidentCode(int &nLastIncidentCode);
    fep::Result DumpCompleteIncidentHistory();
    fep::Result SettingOptions();

private:
    /// Any kind of socket
    cSnippetSocket m_oUDPSocket;
    /// Custom strategy
    cMyStrategy m_oMyStrategy;
};

#endif //__SNIPPET_INCIDENT_ELEMENT_H_
