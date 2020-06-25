/**
 *
 * Demo for FEP Remote Incidents
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
 */

namespace fep
{
 
/**
 * \page page_fep_demo_incident Example: Handling FEP Incidents 
 *
 *
 * This example demonstrates how to use and handle "Remote Incidents". For this, the setup will
 * use one Slave FEP Element ("BadlyCodedElement") and one Master FEP Element ("MasterElement"). The latter of 
 * which provides a fully customized FEP Incident Strategy to handle and resolve incidents that 
 * have been issued by the Slave FEP Element. <br><br>
 *
 * For details on the FEP Incident Handling Mechanism see \ref fep_incident_handling.
 *
 * \par Location
 * \code
 *    ./examples/src/demo_incident
 * \endcode
 *
 * \par Build Environment
 *
 * This example only relies on standard C++98 and the STL.
 *
 * \par What is demonstrated
 *
 * - How to implement a customized and configurable FEP Incident Strategy
 * - How to configure the built-in FEP Notification Strategy to broadcast Incidents no matter of 
 * which severity level
 * - How to use the "Global Scope" for Incidents in the context of a Master FEP Element
 * - Using built-in FEP Incident Codes
 *
 * \par Demonstrated Use-Case
 *
 * This example does not serve a specific use-case. It may, however, be used as a guideline on how
 * to implement basic FEP incident handling routines within the scope of an entire FEP System 
 * domain. As illustrated by the the sequence diagram below, the \c main.cpp file drives two 
 * FEP Elements in the fashion of a batch-file for a functional test. The objective is a demonstration 
 * of the API and of means of communication in between Master FEP Element (cMasterElement) and Slave 
 * FEP Element (cBadlyCodedElement) through commands and incidents. For convenience, this example is 
 * based on two built-in incidents of the FEP distribution, both of which usually have the 
 * severity level of a warning message: fep::FSI_PROP_TREE_ELEMENT_HEADER_INVALID and 
 * fep::FSI_STM_STAND_ALONE_MODE. Whilst under normal circumstances, these are only logged 
 * as warnings, the implementation of cMyMasterStrategy considers these as "fatal" incidents for 
 * this particular example. <br>
 * Upon encountering fep::FSI_PROP_TREE_ELEMENT_HEADER_INVALID, the custom strategy will shut 
 * down the Slave FEP Element remotely 
 * and make the Master FEP Element instance enter fep::FS_ERROR. This will happen not matter 
 * which of the two FEP Elements reports an invalid FEP Element Header. While the 
 * FSI_STM_STAND_ALONE_MODE incident is perfectly fine (and desired) for a Master FEP Element itself, 
 * it does not make any sense when applied to the Slave FEP Element as it is naturally supposed to be 
 * driven by external commands.
 * If the Master FEP Element notices the Slave FEP Element switching into the stand-alone mode, it will
 * reset the respective property remotely to regain control over the Slave FEP Element. The incident will not
 * result in an error state in either of the two Elements but is being resolved "transparently" to
 * demonstrate a "failsafe" use-case.
 *
 * For the scope of this example, the BadlyCodedElement does not incorporate any means of local 
 * incident handling. In contrast to this, the Master FEP Element is designed to handle
 * both, local incidents concerning itself as well as remote incidents issued by the 
 * BadlyCodedElement. To ensure reception of FEP Incident Notifications, the Master FEP Element
 * has the "Global Scope" enabled for its FEP Incident Handler while the BadlyCodedElement has
 * its built-in FEP Notification Strategy activated. For details on what options the FEP Incident
 * Handler and the accompanying built-in FEP Strategies have to offer please refer to
 * \ref fep_incident_handling.
 *
 * Please note that the implemented example has no real significance for real-world use-cases.
 * Both of the mentioned incidents are rather used for informational purposes and should not
 * be seen as a reason to interrupt a Elements initialization phase in a productive environment.
 *
 * <table border="0">
 * <tr><td style="padding-right:100px;padding-left:100px;">
 \dot
digraph fep_demo_incident_setup {
    graph [rankdir="T   "];
    node [shape = point ];
    Entry
    node [shape = box, style = rounded ];
    Master      [label="Master FEP Element"];
    node [group=""]
    BadlyCodedElement           [label="BadlyCodedElement"];
    Entry -> Master            [label = "main.cpp" style="dotted" ];
    Master -> BadlyCodedElement [label = "Control     "];
    Master -> Master [label = "Incidents"];
    BadlyCodedElement -> Master [label = "Incidents"];
}
\enddot
</td><td>
\msc
Master, MyBadlyCodedElement;
|||;
Master box Master [label="STARTUP", textbgcolor="#7F7FFF"];
Master->Master [label="Set Property #Fussyness to TRUE"];
Master->Master [label="Set Property #CriticalElement to MyBadlyCodedElement"];
Master->Master [label="Set Property #EnableGlobalScope to TRUE"];
Master->Master [label="Associate and Configure Custom FEP Strategy"];
Master box Master [label="IDLE", textbgcolor="#7FFFFF"];
Master:>Master [label="Local Warning: Element Header invalid", textcolor="red"];
Master box Master [label="ERROR", textbgcolor="#FF11111"];
Master->Master [label="Set Property #Fussyness to FALSE"];
Master->Master [label="evErrorFixed", linecolor="gray", textcolor="gray"];
...;
...;
\endmsc
</td></tr>
<tr>
<td>
<center><i>Incident Demo Setup</i></center>
</td><td>
<center><i>Click \subpage page_demo_incident_global_seq_full to see the full chart</i></center>
</td>
</tr>
</table>
 *
 * \par Running the example
 *
 * The example may simply be run without any parameters.
 \code
 $> ./demo_incident_global
 \endcode
 *
 * The output will be as follows:
 \verbatim
 Startup MasterElement
 [MasterElement] Warning: Element Header has not been filled properly!
 Startup Done MasterElement
 MasterElement threw an error! Something went wrong!
 [MasterElement] Info: Property bStandAloneModeEnabled has changed.
 [MasterElement] Warning: Stand Alone Mode has been enabled
 [MasterElement] Warning: Element Header has not been filled properly!
 Stopped MasterElement
 MasterElement threw an error! Something went wrong!
 >>>> The master refused to start. Apparently the element header was not filled and its incident strategy did complain.
 [MasterElement] Warning: Element Header has not been filled properly!
 Stopped MasterElement
 Initializing MasterElement
 Ready MasterElement
 Running MasterElement
 >>>> Disabled fussiness; now the master accepts to enter running state.
 >>>> Halting the master to correct the otherwise persistent issue of an invalid FEP Element Header and starting over afterwards.
 Stopped MasterElement
 Initializing MasterElement
 Ready MasterElement
 Running MasterElement
 >>>> Now the Element Header has been filled in properly and the Master reached running state.
 Startup BadlyCodedElement
 Startup Done BadlyCodedElement
 MasterElement: Element BadlyCodedElement does not have a propery filled Element header. Preventing System Initialization.
 MasterElement threw an error! Something went wrong!
 [BadlyCodedElement] Warning: Element Header has not been filled properly!
 >>>> First of all, the slave element doesn't have its Element Header set up either. This will make the fussy master fail. So, filling the property tree for a second attempt and starting over.
 Stopped MasterElement
 Initializing MasterElement
 Ready MasterElement
 Running MasterElement
 >>>> Resolving the induced error state of the FEP Element.
 >>>> Constructing a secondary incident by setting the BadlyCoded Element into StandAloneMode - which generally is a pretty bad idea. For this demonstration, the Master Element is set up to resolve this issue remotely
 [BadlyCodedElement] Info: Property bStandAloneModeEnabled has changed.
 MasterElement: Element BadlyCodedElement has its stand-alone mode enabled. This is being corrected remotely by the Master.
 [BadlyCodedElement] Warning: Stand Alone Mode has been enabled
 [BadlyCodedElement] Info: Property bStandAloneModeEnabled has changed.
 >>>> With all pre-conditions met, the system is able to reach running mode
 Initializing BadlyCodedElement
 Ready BadlyCodedElement
 BadlyCodedElement reached running mode
 >>>> Shutting down the system to exit the demonstration
 Stopped MasterElement
 Stopped BadlyCodedElement
 >>>> MAIN::Example run complete. Exiting.
 \endverbatim
 *
 * \par The Implementation of this example
 * - \subpage page_demo_incident_global_master
 * <br>
 * - \subpage page_demo_incident_global_slave
 * <br>
 *
 */

/**
* \page page_demo_incident_global_master FEP Element: Master FEP Element with incident strategy (code)
* <hr>
* Class declaration
* <hr>
* \include demo_incident_global/demo_master_element.h
* <br>
* <hr>
* Class implementation
* <hr>
* \include demo_incident_global/demo_master_element.cpp
* <br>
*/

/**
* \page page_demo_incident_global_slave FEP Element: BadlyCodedElement Slave (code)
* <hr>
* Class declaration
* <hr>
* \include demo_incident_global/demo_incident_element.h
* <br>
* <hr>
* Class implementation
* <hr>
* \include demo_incident_global/demo_incident_element.cpp
* <br>
*/

/**
* \page page_demo_incident_global_seq_full Chart: Global Incident Sequence (enlarged)
\msc
Master, MyBadlyCodedElement;
|||;
Master box Master [label="STARTUP", textbgcolor="#7F7FFF"];
Master->Master [label="Set Property #Fussyness to TRUE"];
Master->Master [label="Set Property #CriticalElement to MyBadlyCodedElement"];
Master->Master [label="Set Property #EnableGlobalScope to TRUE"];
Master->Master [label="Associate and Configure Custom FEP Strategy"];
Master box Master [label="IDLE", textbgcolor="#7FFFFF"];
Master:>Master [label="Local Warning: Element Header invalid", textcolor="red"];
Master box Master [label="ERROR", textbgcolor="#FF11111"];
Master->Master [label="Set Property #Fussyness to FALSE"];
Master->Master [label="evErrorFixed", linecolor="gray", textcolor="gray"];
Master box Master [label="IDLE", textbgcolor="#7FFFFF"];
Master box Master [label="READY", textbgcolor="#7FFF7F"];
Master box Master [label="RUNNING", textbgcolor="#7FFF7F"];
Master note Master [label="Disabling the strategy effectively is no solution; filling the FEP Element header instead"];
Master->Master [label="Fill in FEP Element Header", textcolor="green"];
Master->Master [label="evStop", linecolor="gray", textcolor="gray"];
Master->Master [label="Set Property #Fussyness to TRUE"];
Master box Master [label="IDLE", textbgcolor="#7FFFFF"];
Master box Master [label="READY", textbgcolor="#7FFF7F"];
Master box Master [label="RUNNING", textbgcolor="#7FFF7F"];

MyBadlyCodedElement box MyBadlyCodedElement [label="STARTUP", textbgcolor="#7F7FFF"];
MyBadlyCodedElement->MyBadlyCodedElement  [label="Set Property #EnableNotificationStrategy to TRUE"];
MyBadlyCodedElement note MyBadlyCodedElement [label="Allowing the Element to broadcast incidents no matter the severity."];
MyBadlyCodedElement box MyBadlyCodedElement [label="IDLE", textbgcolor="#7FFFFF"];
MyBadlyCodedElement:>MyBadlyCodedElement [label="Global Warning: Element Header invalid", textcolor="red"],
MyBadlyCodedElement:>Master [label="", textcolor="red"];
MyBadlyCodedElement box MyBadlyCodedElement [label="ERROR", textbgcolor="#FF11111"],
Master box Master [label="ERROR", textbgcolor="#FF11111"];
Master note Master [label="The custom strategy made the Master enter Error State as soon as the BadlyCodedElement issues the incident above."];
MyBadlyCodedElement->MyBadlyCodedElement [label="Fill in FEP Element Header", textcolor="green"];
Master->Master [label="evErrorFixed", linecolor="gray", textcolor="gray"];
MyBadlyCodedElement->MyBadlyCodedElement [label="evErrorFixed", linecolor="gray", textcolor="gray"];
Master box Master [label="IDLE", textbgcolor="#7FFFFF"],
MyBadlyCodedElement box MyBadlyCodedElement [label="IDLE", textbgcolor="#7FFFFF"];
Master box Master [label="READY", textbgcolor="#7FFF7F"];
Master box Master [label="RUNNING", textbgcolor="#7FFF7F"];

Master note Master [label="The Slave FEP Element attempts to enable the stand-alone mode which is has to be treated by an error by the Master FEP Element"];
MyBadlyCodedElement->MyBadlyCodedElement [label="Set Property #EnableStandAloneMode to TRUE"];
MyBadlyCodedElement:>MyBadlyCodedElement [label="Global Warning: Stand Alone Mode enabled", textcolor="red"],
MyBadlyCodedElement:>Master [label="", textcolor="red"];
Master note Master [label="In this case, the Master is able to recover the BadlyCodedElement remotely and reset the Stand Alone Mode"];
Master->MyBadlyCodedElement  [label="Set Property #EnableStandAloneMode to FALSE", textcolor="green"];
Master->MyBadlyCodedElement [label="evInitialize", linecolor="gray", textcolor="gray"];
MyBadlyCodedElement box MyBadlyCodedElement [label="READY", textbgcolor="#7FFF7F"];
Master->MyBadlyCodedElement [label="evStart", linecolor="gray", textcolor="gray"];
MyBadlyCodedElement box MyBadlyCodedElement [label="RUNNING", textbgcolor="#7FFF7F"];
MyBadlyCodedElement note MyBadlyCodedElement [label="Element is re-enabled to be controlled through the Master and reached RUNNING state successfully"];

Master->MyBadlyCodedElement [label="evStop", linecolor="gray", textcolor="gray"];
MyBadlyCodedElement box MyBadlyCodedElement [label="IDLE", textbgcolor="#7FFFFF"];
Master->MyBadlyCodedElement [label="evShutdown", linecolor="gray", textcolor="gray"];
MyBadlyCodedElement box MyBadlyCodedElement [label="STARTUP", textbgcolor="#7F7FFF"];
Master->Master [label="evStop", linecolor="gray", textcolor="gray"];
Master->Master [label="evShutdown", linecolor="gray", textcolor="gray"];
Master box Master [label="IDLE", textbgcolor="#7FFFFF"];
Master box Master [label="STARTUP", textbgcolor="#7F7FFF"];
\endmsc
<hr>
<br>
*/

}
