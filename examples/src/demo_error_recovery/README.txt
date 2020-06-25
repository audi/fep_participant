/**
 *
 * Demo for FEP error recovery
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

namespace fep
{
 
/**
 * \page page_fep_demo_error_recovery Example: Error Recovery using a Master FEP Element 
 *
 *
 * This example demonstrates how to handle and fix a slave error inside a Master FEP Element
 * and subsequently signal any Slave FEP Elements of the fix to let them reinitialize <br><br>
 *
 * This example makes use of the incident handling mechanisms in FEP. For details see
 * \ref fep_incident_handling.
 *
 * \par Location
 * \code
 *    ./examples/src/demo_error_recovery
 * \endcode
 *
 * \par Build Environment
 *
 * This example only relies on standard C++98 and the STL.
 *
 * \par What is demonstrated
 *
 * - How to implement a typical standalone Master FEP Element
 * - How to implement a worker FEP Element that drives itself
 * - How to implement customized FEP Incident strategies
 * - How to use control commands to signal an "error fixed" event
 * - How to use the "Global Scope" for incidents to signal that a FEP Element has finished its work
 * - Using custom FEP incident codes
 *
 * \par Running the example
 *
 * The example may simply be run without any parameters.
 \code
 $> ./demo_error_recovery
 \endcode
 *
 * The output will look as follows:
 \verbatim
MasterElement@HOSTNAME  ST: 0[us]  Info 4: Startup
MasterElement@HOSTNAME  ST: 0[us]  Info 4: Idle
MasterElement@HOSTNAME  ST: 0[us]  Info 4: Initializing
MasterElement@HOSTNAME  ST: 0[us]  Info 4: Ready
MasterElement@HOSTNAME  ST: 42[us]  Info 4: Running
MasterElement@HOSTNAME  ST: 1981982[us]  [StateMachine] Info 102: STM: stand alone mode just got enabled
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: Startup
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: Idle
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: Initializing
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: Ready
SlaveElement@HOSTNAME  ST: 36[us]  Info 4: Running
SlaveElement@HOSTNAME  ST: 2000253[us]  Critical -3840: Invoking critical incident
SlaveElement@HOSTNAME  ST: 2000429[us]  Critical -3840: Invoking critical incident
MasterElement@HOSTNAME  ST: 4925762[us]  Info 4: Received error incident, fixing now
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: Error
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: Idle
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: Initializing
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: Ready
SlaveElement@HOSTNAME  ST: 51[us]  Info 4: Running
SlaveElement@HOSTNAME  ST: 2000245[us]  Critical -3840: Invoking critical incident
SlaveElement@HOSTNAME  ST: 2000399[us]  Critical -3840: Invoking critical incident
MasterElement@HOSTNAME  ST: 7950521[us]  Info 4: Received error incident, fixing now
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: Error
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: Idle
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: Initializing
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: Ready
SlaveElement@HOSTNAME  ST: 43[us]  Info 4: Running
SlaveElement@HOSTNAME  ST: 2000172[us]  Info -3841: Finished work
SlaveElement@HOSTNAME  ST: 2000317[us]  Info -3841: Finished work
MasterElement@HOSTNAME  ST: 10976510[us]  Info 4: Received finished incident, stopping now
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: Idle
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: CleanUp
SlaveElement@HOSTNAME  ST: 0[us]  Info 4: Shutdown
MasterElement@HOSTNAME  ST: 0[us]  Info 4: Idle
MasterElement@HOSTNAME  ST: 0[us]  [StateMachine] Info 102: STM: stand alone mode just got disabled
MasterElement@HOSTNAME  ST: 0[us]  Info 4: Cleanup
MasterElement@HOSTNAME  ST: 0[us]  Info 4: Shutdown
 \endverbatim
 *
 * \par The Implementation of this example
 * - \subpage page_demo_error_recovery_master
 * <br>
 * - \subpage page_demo_error_recovery_slave
 * <br>
 *
 */

/**
* \page page_demo_error_recovery_master FEP Element: Master FEP Element (code)
* <hr>
* Class declaration
* <hr>
* \include demo_error_recovery/demo_master_element.h
* <br>
* <hr>
* Class implementation
* <hr>
* \include demo_error_recovery/demo_master_element.cpp
* <br>
*/

/**
* \page page_demo_error_recovery_slave FEP Element: Slave FEP Element (code)
* <hr>
* Class declaration
* <hr>
* \include demo_error_recovery/demo_error_element.h
* <br>
* <hr>
* Class implementation
* <hr>
* \include demo_error_recovery/demo_error_element.cpp
* <br>
*/
*/

}
