/**
 *
 * Exemplary Timing Master FEP Element which utilizes either a custom continuous or discrete clock
 *
 * @file
Copyright @ 2019 Audi AG. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.
 *
 */
 
/**
 * \page page_demo_timing_master_external_clock Example: Timing Master using external clocks
 *
 *
 * This example covers a FEP Timing Master Element which uses either an external continuous or
 * an external discrete clock as main clock instead of a built-in clock.
 *
 * \par Location
 * \code
 *    ./examples/src/demo_timing_master_external_clock
 * \endcode
 *
 * \par Build Environment
 *
 * This example only relies on standard C++ and the STL.
 *
 * \par What is demonstrated
 *
 * How to create a FEP Timing Master Element, how to implement a custom continuous or discrete clock and how to make the timing master utilize a custom clock.
 *
 * \par Demonstrated Use-Case
 *
 * This FEP Element serves the use-case of using an external continuous or discrete clock as main clock for a FEP Timing Master Participant.
 * The used clock and optional clock settings may be configured using the command line.
 * For additional information regarding FEP Timing 3 please see \ref page_fep_timing_3.
 *
 * \par The Implementation of the FEP Timing Master Demo
 *
 * First of all, we need to implement a custom continuous and a custom discrete clock which will be used as main clock of the FEP Timing Master Participant.
 * We do so by extending the FEP ContinuousClock or DiscreteClock classes.
 *
 * The continuous clock is a continuously running clock which provides the current time on demand.
 *
 * The discrete clock is a clock which steps in discrete time steps and actively propagates these time update events across the FEP system. 
 * 
 * The custom continuous clock provides the current system time by overriding getNewTime.
 * To provide a way to reset the current time, we implement resetTime which stores the current time as offset which will be taken into account from now on.
 * By changing the return value of getNewTime, we can use the continuous clock to provide any time.
 * \snippet timing_master_external_clock_participant.cpp CucstomContinuousClock
 *
 * To create a custom discrete clock we need a helper class which is responsible for updating the discrete clock whenever a discrete time step passes.
 * For this purpose, the system timestamp of the next discrete time step is calculated. The DiscreteClockUpdater waits until the system time reaches this point in time and updates the clock.
 * To adapt the functionality of stepping discrete time steps, we have to change the way updateTime is triggered by for example adapting or replacing the work method.
 * \snippet timing_master_external_clock_participant.cpp DiscreteClockUpdater
 *
 * The corresponding discrete clock uses an EventSink to distribute the simulated time across the FEP system every time a discrete time step is passed.
 * The length of a discrete time step may be configured by setting the clock's cycle time.
 * \snippet timing_master_external_clock_participant.cpp CucstomDiscreteClock
 *
 * Now as we created a custom continuous and a custom discrete clock, we may implement the FEP Timing Master Participant.
 *
 * First of all, we have to enable FEP Timing 3 which was introduced in FEP SDK 2.3. To do so, we specify FEP Timing 3 in the ModuleOptions and use those to create the FEP Module.
 * \snippet timing_master_external_clock_participant.cpp EnableFEPTiming25
 * \snippet timing_master_external_clock_participant.cpp CreateFEPModule
 *
 * We extend the FEP Module and override ProcessStartUpEntry to configure the FEP Participant. First we configure the FEP Participant header which contains general information according the participant. 
 * \snippet timing_master_external_clock_participant.cpp ConfigureParticipantHeader
 *
 * We register both custom clocks at the Clock Service of the FEP Participant. Clocks registered at the Clock Service may be set as main clock for the FEP Participant.
 * The Clock Service contains a continuous clock 'local_system_realtime' and a discrete clock 'local_system_simtime' by default. For additional information regarding FEP Timing 3 please see \ref page_fep_timing_3.
 * \snippet timing_master_external_clock_participant.cpp RegisterCustomClocks
 *
 * Once clocks are registered at the Clock Service we can either set the custom continuous clock or the custom discrete clock as main clock of the FEP Participant.
 * We can do so by either setting the corresponding property or by using the Clock Service. For additional information on how to set a main clock by property please see \ref page_fep_timing_3.
 * \snippet timing_master_external_clock_participant.cpp SetMainClock
 *
 * \par Running the example
 *
 * The example may simply be run by starting the executable and specifying the corresponding clock mode:
 \code
 // To start the timing master using the custom continuous clock
 $> ./demo_timing_timingmaster_external_clock.exe --mode 0
 // To start the timing master using the custom discrete clock
 $> ./demo_timing_timingmaster_external_clock.exe --mode 1
  // Optionally, configure the cycle time of the custom discrete clock
 $> ./demo_timing_timingmaster_external_clock.exe --mode 1 --cycle 200
 \endcode
 *
 * \par Source code of the FEP Timing Master Demo
 * \subpage page_timing_master_external_clock_participant_code <br>
 */

/**
* \page page_timing_master_external_clock_participant_code FEP Element: Timing Master Element (code)
* <hr>
* Class declaration
* <hr>
* \include timing_master_external_clock_participant.h
* <br>
* <hr>
* Class implementation
* <hr>
* \include timing_master_external_clock_participant.cpp
*/
