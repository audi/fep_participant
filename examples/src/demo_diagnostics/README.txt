/**
 *
 * Exemplary Diagnostics FEP Element
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
 * \page page_diagnostics_participant Example: Diagnostics FEP Element 
 *
 *
 * This example is ment to serve as starting point for various diagnostic tasks.
 * The different tasks are meant to be implemented in a modular way.
 * Currently implemented tasks:
 *   - selftest to test communication within the fep system
 *
 *
 * \par Location
 * \code
 *    ./examples/src/demo_diagnostics
 * \endcode
 *
 * \par Build Environment
 *
 * This example only relies on standard C++ and the STL.
 *
 * \par What is demonstrated
 *
 * Measuring and analysing various aspects of FEP.
 *
 * \par Demonstrated Use-Case
 *
 * This FEP Element should be extended if new measurement tasks are arising.
 * It serves as a starting point and template therefore.
 * 
 *
 * \par Running the example
 * 
 * Available modes:
 *  --selftest
 *
 *
 * These modes are described in the following sections.
 *
 * Selftest:
 *---------------
 * Test process:
 *        1. Test minimal timeout necessary to receive state of remote fep element
 *        2. Measure time it takes a remote fep element to receive a signal and respond
 *        3. Try to accquire an example property of a remote fep element
 *
 * Typical usage of selftest:
 \code
 $> ./demo_diagnostics_files/demo_diagnostics_selftest_mixed
 \endcode
 * This performs the entire selftest on one machine. It therefore starts a server fep element (element_server.cpp) and a corresponding client fep element (element_client.cpp) that performs all 
 * required test steps and prints the results. The client application will ask you to confirm that a server is running before starting the tests/measurements.
 * To perform the selftest in a distributed system start the server element on one machine
 \code
 $> ./demo_diagnostics_files/demo_diagnostics_selftest_server
 \endcode
 * and a Client elment on another machine:
 \code
 $> ./demo_diagnostics_files/demo_diagnostics_selftest_client
 \endcode
 * The client machine will receive the measurement and test results. Always make sure that only one instance of server and client ist running
 * at one point in time. Everything else will most probably result in errors! For a more detailed description run:
 \code
 $> ./demo_diagnostics --selftest --help
 \endcode
 * Besides some information on the current state of the selftest, the client application will print a summary when all tests have finished. This will look somewhat like the following:
 \code
_______________________________________________
____________________SUMMARY____________________
Minimal timeout: 100 ms                                #1
Signal roundtrip time: 0.001707 s                      #2
Retrival of remote property: successful                #3
_______________________________________________

Exiting ...
 \endcode
 * The line marked with '#1' (not in the actual output) displays the minimal time out that was required to access the state of a remote fep element (the server). For this the timeout will be
 * increased until the remote state could be received or the maximum timeout is reached. The tested timeouts are 100, 250, 500, 1000, 2500 and 5000 ms. 
 * The signal roundtrip time '#2' is the time it takes to send a signal from client to server and receive the servers answer signal. It is measured with micro-second resolution.
 * Output '#3' informs you whether or not the client element could read a remote property of the server element. It displays 'successful' on success and 'failed' on failure.
 
 *
 * \par The Implementation of the selftest capabilities
 * \subpage page_element_selftest_code <br>
 * \subpage page_element_client_code <br>
 * \subpage page_element_server_code <br>
 * \subpage page_selftest_common_code <br>
 *
 */

/**
* \page page_element_selftest_code Selftest functions: selftest (code)
* <hr>
* Class declaration
* <hr>
* \include selftest.h
* <br>
* <hr>
* Class implementation
* <hr>
* \include selftest.cpp
*/

/**
* \page page_element_client_code FEP Element: Element Client (code)
* <hr>
* Class declaration
* <hr>
* \include element_client.h
* <br>
* <hr>
* Class implementation
* <hr>
* \include element_client.cpp
*/

/**
* \page page_element_server_code FEP Element: Element Server (code)
* <hr>
* Class declaration
* <hr>
* \include element_server.h
* <br>
* <hr>
* Class implementation
* <hr>
* \include element_server.cpp
*/

/**
* \page page_selftest_common_code Functionality that server and client have in Common (code)
* <hr>
* Class declaration
* <hr>
* \include common.h
*/
