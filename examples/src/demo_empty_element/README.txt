/**
 *
 * Exemplary Stand-Alone FEP Element
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
 * \page page_empty_participant Example: Empty FEP Element 
 *
 *
 * This example represents a very simplistic, empty FEP Element and may serve as a starting
 * point for further development.
 *
 * \par Location
 * \code
 *    ./examples/src/demo_empty_element
 * \endcode
 *
 * \par Build Environment
 *
 * This example only relies on standard C++ and the STL.
 *
 * \par What is demonstrated
 *
 * Creating a first FEP Element, how to configure it by commandline and switching through all its FEP States.
 *
 * \par Demonstrated Use-Case
 *
 * This FEP Element does not serve a specific use-case. It may be used as a guideline on how to 
 * implement and run a FEP Element. The state of the demo FEP Element can be controlled by using any other FEP Element or tool that 
 * can transmit state change commands (for example the FEP Control GUI). For additional information about the FEP State Machine please see \ref fep_state_machine.
 *
 * \par Running the example
 *
 * The example may simply be run by starting the scripts:
 \code
 $> ./demo_empty_element_files/demo_empty_element_launch
 // after successful launch start demo_empty_element_controller
 $> ./demo_empty_element_files/demo_empty_element_controller
 // now you can control the empty element by using fep_controller commands
 \endcode
 * 
 * For further information about the fep_controller commands have a look into the documentation
 * of @ref fep_description_tooling
 *
 *
 * \par The Implementation of the Empty FEP Element Demo
 * \subpage page_empty_element_code <br>
 *
 */

/**
* \page page_empty_element_code FEP Element: Empty Element (code)
* <hr>
* Class declaration
* <hr>
* \include empty_element.h
* <br>
* <hr>
* Class implementation
* <hr>
* \include empty_element.cpp
*/
