/**

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
#ifndef _HEADER_SERVER_ELEMENT_H_
#define _HEADER_SERVER_ELEMENT_H_
#include <fep_participant_sdk.h>
/*
* Server FEP element used in selftest mode of 'Diagnostics Demo'. The FEP-elements will be queried
* for its current state by a client FEP element. After that it will wait for a 'ping' signal and
* respond with a 'pong' signal. This is used for roundtrip time measurements. This element has a
* property called 'TestPropertyString' which the client fep element will attempt to retrieve. 
*/
class cElementServer: public fep::cModule,
    public fep::cStateRequestListener,
    public fep::IUserDataListener
{
public:
    cElementServer();
    virtual ~cElementServer();
public:
    virtual fep::Result Create(const fep::cModuleOptions& oModuleOptions);

    const char* GetParticipantName() const;

    //Implements cModule
public:
    virtual fep::Result ProcessInitializingEntry(const fep::tState eOldState);
    virtual fep::Result ProcessIdleEntry(const fep::tState eOldState);
    virtual fep::Result ProcessReadyEntry(const fep::tState eOldState);
    virtual fep::Result ProcessRunningEntry(const fep::tState eOldState);
    virtual fep::Result ProcessStartupEntry(const fep::tState eOldState);
    virtual fep::Result ProcessErrorEntry(const fep::tState eOldState);

    //Implements IUserDataListener
public:
    virtual fep::Result Update(const fep::IUserDataSample* poSample);

public:
    virtual fep::Result TerminateServer();
    /*
    * Returns flag that indicates whether the server element has received ping signal and
    * responded with pong signal.
    */
    virtual bool IsDone();

private:
    std::string m_strParticipantName;
    handle_t m_hPing;
    handle_t m_hPong;
    fep::IUserDataSample* m_pSamplePong;
    const char* m_strTestProperty;
    // Is set true when the server did his job and responded with his pong signal
    bool m_bTestDone;
};
#endif
