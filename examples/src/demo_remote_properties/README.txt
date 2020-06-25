/**
 *
 * Stand-Alone FEP Element to access remote properties
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
 * \page page_remote_properties_participant Example: Reading Remote Properties 
 *
 *
 * This example implements a small utility FEP Element that can print (parts of) the property tree 
 * of other FEP Elements in the current FEP System. Packaged as a reusable console application, it 
 * can be used to easily diagnose the property tree in a remote FEP Element.
 *
 *
 * \par Location
 * \code
 *    ./examples/src/demo_remote_properties
 * \endcode
 *
 * \par Build Environment
 *
 * This example only relies on standard C++ and the STL.
 *
 * \par What is demonstrated
 *
 * Creating a stand-alone FEP Element that drives itself through the FEP State
 * Machine and remotely accesses the FEP Property Tree of another FEP Element.
 *
 * \par Demonstrated Use-Case
 *
 * This FEP Element serves the use case of diagnosing and debugging the configuration
 * of a FEP Element by dumping its FEP Property Tree (or parts of it) into a readable format.
 *
 * \par Running the example
 *
 * The example may simply be run by starting the scripts:
 \code
 $> ./demo_remote_properties_files/demo_remote_properties_dummy
 // and after successful start of the dummy participant
 $> ./demo_remote_properties_files/demo_remote_properties
 
 \endcode
 *
 * \par The Implementation of the remote properties demo
 * \subpage page_remote_properties_code <br>
 * <br>
 *
 */

/**
 * \page page_remote_properties_code FEP Element: Remote Property (code)
 * <hr>
 * Class declaration
 * <hr>
 * \include remote_properties.h
 * <br>
 * <hr>
 * Class implementation
 * <hr>
 * \include remote_properties.cpp
 */
