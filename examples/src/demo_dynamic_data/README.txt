/**
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
 * \page page_demo_dynamic_data Example: Demo Dynamic Data
 *
 *
 * This example implements two FEP Elements, one raw string sender and one raw string receiver.
 * Using the sender element raw strings can be sent and the receiver elements receives these strings.
 * When an empty string is sent by the sender, both the participants reach IDLE state and shutsdown.
 * 
 *
 * \par Location
 * \code
 *    ./examples/src/demo_dynamic_data
 * \endcode
 *
 * \par Build Environment
 *
 * This example only relies on standard C++ and the STL.
 *
 * \par What is demonstrated
 *
 * - Creating stand-alone FEP Elements that drive themselves through the FEP State Machine
 * - Registering input and output signals as well as data listeners
 * - Sending and receiving a raw strings
 *
 * \par Demonstrated Use-Case
 *
 * This FEP Element does not serve a specific use-case. It may be used as a guideline on how to 
 * implement and run FEP Elements to send and receive raw strings. The state of the demo FEP Element can be controlled by using any other FEP Element or tool that 
 * can transmit state change commands (for example the FEP Control GUI). For additional information about the FEP State Machine please see \ref fep_state_machine.
 *
 * \par Running the example
 *
 * The example may simply be run by starting the sender and receiver executables respectively:
 \code
 $> ./DynamicDataSender
 $> ./DynamicDataReceiver
 \endcode
 * <br>
 *
 * \par The Implementation of the signal mapping demo
 * \subpage page_dynamic_data_sender <br>
 * \subpage page_dynamic_data_receiver <br>
 * <br>
 *
 */

/**
 * \page page_dynamic_data_sender FEP Element: dynamic data sender
 * <hr>
 * Class implementation of the sender element
 * <hr>
 * \include dynamic_data_sender.cpp
 */
 
 /**
 * \page page_dynamic_data_receiver FEP Element: dynamic data receiver
 * <hr>
 * Class implementation of the receiver element
 * <hr>
 * \include dynamic_data_receiver.cpp
 */
