/**
* Implementation of a mock incident invocation handler implementing the IIncidentInvocationHandler interface
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

#include "incident_handler/fep_incident_handler.h"

class  cMockIncidentInvocationHandler : public IIncidentInvocationHandler
{
public:
    /**
    * CTOR
    */
    cMockIncidentInvocationHandler()
    {
    }
    /**
    * DTOR
    */
    virtual ~cMockIncidentInvocationHandler()
    {
    }

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
    virtual fep::Result InvokeIncident(int16_t nIncidentCode, tSeverityLevel eSeverity,
        const char* strDescription, const char* strOrigin,
        int nLine, const char* strFile)
    {
        a_util::concurrency::unique_lock<a_util::concurrency::recursive_mutex> oSync(m_mtProtect);
        m_nIncidentCode = nIncidentCode;
        m_eSeverity = eSeverity;
        if (NULL != strFile)
        {
            m_strFile = strFile;
        }
        if (NULL != strDescription)
        {
            m_strDescription = strDescription;
        }
        if (NULL != strOrigin)
        {
            m_strOrigin = strOrigin;
        }
        m_nLine = nLine;
        if (NULL != strFile)
        {
            m_strFile = strFile;
        }

        return fep::ERR_NOERROR;
    }

public:
    int16_t m_nIncidentCode;
    tSeverityLevel m_eSeverity;
    std::string m_strDescription;
    std::string m_strOrigin;
    int m_nLine;
    std::string m_strFile;
    a_util::concurrency::recursive_mutex m_mtProtect;
};
