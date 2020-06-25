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
 * \page page_demo_signal_mapping Example: Signal Mapping
 *
 *
 * This example implements two FEP Elements, one signal producer and one signal consumer.
 * The producer writes two signals 'LightOrientation' and 'LightPos' while the consumer expects
 * an incompatible signal 'LightSource'.
 * 
 * To integrate the consumer element into this signal environment, signal mapping is used to map
 * the expected input signal from the existing output signals and some constants.
 *
 * The mapping configuration used in the example is stored in the mapping_example.map file alongside
 * the code. You can try experimenting with the configuration, for example by assigning different constants
 * to the mapped target signal. Simply run the demo again to see the changes in action.
 *
 * \par Location
 * \code
 *    ./examples/src/demo_signal_mapping
 * \endcode
 *
 * \par Build Environment
 *
 * This example only relies on standard C++ and the STL.
 *
 * \par What is demonstrated
 *
 * - Creating stand-alone FEP Elements that drive themselves through the FEP State Machine
 * - Registering signal description
 * - Registering mapping configuration and therefore using signal mapping
 * - Registering input and output signals as well as data- and cyclic listeners
 *
 * \par Demonstrated Use-Case
 *
 * - Integrating a FEP Element into a foreign FEP System with an incompatible signal environment
 *
 * \par Running the example
 *
 * The example may simply be run by starting the executable:
 \code
 $> ./demo_signal_mapping
 \endcode
 * <br>
 *
 * \par The Implementation of the signal mapping demo
 * \subpage page_signal_mapping_demo_producer <br>
 * \subpage page_signal_mapping_demo_consumer <br>
 * <br>
 *
 */

/**
 * \page page_signal_mapping_demo_producer FEP Element: Signal Mapping Producer
 * <hr>
 * Class declaration of the producer element
 * <hr>
 * \include signal_producer.h
 * <br>
 * <hr>
 * Class implementation of the producer element
 * <hr>
 * \include signal_producer.cpp
 */
 
 /**
 * \page page_signal_mapping_demo_consumer FEP Element: Signal Mapping Consumer
 * <hr>
 * Class declaration of the consumer element
 * <hr>
 * \include signal_consumer.h
 * <br>
 * <hr>
 * Class implementation of the consumer element
 * <hr>
 * \include signal_consumer.cpp
 */
