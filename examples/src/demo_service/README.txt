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
 * \page page_demo_service Example: Services
 *
 *
 * This example implements two FEP Participants, one service provider and one service consumer.
 * 
 * The service provider participant serves a simple interface described in the accompanying service description.
 *
 * The service consumer participant connects to the provider participant and calls into the service interface.
 *
 * \par Location
 * \code
 *    ./examples/src/demo_service
 * \endcode
 *
 * \par Build Environment
 *
 * This example only relies on standard C++ and the STL.
 *
 * \par What is demonstrated
 *
 * - Registering an object server serving an interface
 * - Calling into an interface served by another FEP Participant
 * - Generating support code from a service description (see the CMakeLists.txt of the example)
 *
 * \par Demonstrated Use-Case
 *
 * - Providing and consuming services in FEP
 *
 * \par Running the example
 *
 * The service provider participant needs to be started first:
 \code
 $> ./demo_service_provider
 \endcode
 * Then, any number of service consumer participants can be started (they have random names)
 \code
 $> ./demo_service_consumer
 \endcode
 * <br>
 *
 * \par The implementation of the service demo
 * \subpage page_demo_service_provider <br>
 * \subpage page_demo_service_consumer <br>
 * <br>
 *
 */

/**
 * \page page_demo_service_provider FEP Participant
 * <hr>
 * Implementation of the provider participant
 * <hr>
 * \include service_provider.cpp
 */
 
/**
 * \page page_demo_service_consumer FEP Participant
 * <hr>
 * Implementation of the consumer participant
 * <hr>
 * \include service_consumer.cpp
 */
