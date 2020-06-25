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
#ifndef _HEADER_CLIENT_ELEMENT_H_
#define _HEADER_CLIENT_ELEMENT_H_

#include <fep_participant_sdk.h>
#include <a_util/system/timer.h>

/**
* Client FEP element used in selftest mode of 'Diagnostics Demo'. The FEP-elements will querie
* the current state of a server FEP element. After that it will send a 'ping' signal and wait for
* a response in form of a 'pong' signal. This is used for roundtrip time measurements.
* This element will also attempt to retrieve a remote property called 'TestPropertyString' from
* the server fep element. 
*/
class cElementClient: public fep::cModule, private fep::IUserDataListener
{
public:
    cElementClient();
    virtual ~cElementClient();

    /*
    * Performs the Signal roundtriptime measurement and the retrieval of the remote property.
    *
    * It sends out a ping signal and will declare the test completed if the response signal
    * (pong) wasn't received within 200 cycles.
    *
    * When the propertytest flag is set to true the function will try to get the specified
    * remote property from the server element. It will make a total of five attempts in five
    * cycles.
    *
    */
    void RunCyclic();

    //Implements cModule
public:
    virtual fep::Result Create(const fep::cModuleOptions& oModuleOptions);

    const char* GetParticipantName() const;
    virtual fep::Result ProcessInitializingEntry(const fep::tState eOldState);
    virtual fep::Result ProcessIdleEntry(const fep::tState eOldState);
    virtual fep::Result ProcessReadyEntry(const fep::tState eOldState);
    virtual fep::Result ProcessRunningEntry(const fep::tState eOldState);
    virtual fep::Result ProcessStartupEntry(const fep::tState eOldState);
    virtual fep::Result ProcessErrorEntry(const fep::tState eOldState);

    //Implements IUserDataListener
public:
    fep::Result Update(const fep::IUserDataSample* poSample);

    //Demo Diagnostics 
public:
    fep::Result TerminateClient();
    fep::Result SetSignaltest(bool signaltest);
    fep::Result SetPropertytest(bool propertytest, timestamp_t tmTimeout);
    bool GetSignaltestDone();
    bool GetPropertytestDone();
    bool GetRemotePropertyReceived();
    timestamp_t GetRoundTripTime();

private:
    std::string m_strParticipant;
    // Flag allowing the signaltest (measuring of the roundtrip time) to be started when set to true
    bool m_bSignaltest;
    // Flag allowing the propertytest (reading a remote property from server) to be started when set to true
    bool m_bPropertytest;
    // Flag indicating that the signaltest completed and the propertytest can be started
    bool m_bSignaltestDone;
    // Flag indicating that the propertytest completed and the application can finish.
    bool m_bPropertytestDone;
    // Flag indicating the signaltest was started. Is used to limit the attempts of receiving the
    // server's response signal (pong).
    bool m_bSignaltestWasStarted;
    a_util::system::Timer m_oTimer;

    // Measured roundtrip time
    timestamp_t m_tmRoundtripTime;
    // Flag indicating that the remote property could be read successfully.
    bool m_bRemotePropertyReceived;

    handle_t m_hPing;
    handle_t m_hPong;
    // Stores the sample that is send in the ping signal
    fep::IUserDataSample* m_pSamplePing;
    // used to measure roundtrip time
    timestamp_t m_tmSend;
    // used to measure roundtrip time
    timestamp_t m_tmReceived;
    // limits the number of attemts :=5
    int16_t m_nAttemptsPropertyTest;
    // limits the number of attempts:=100
    int16_t m_nAttemptsSignalTest;
    timestamp_t m_tmTimeout;
};
#endif
