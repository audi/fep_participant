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
#ifndef CONSTANTS_FOR_ELEMENTS_H
#define CONSTANTS_FOR_ELEMENTS_H
//Unique Identifier of the client fep element
#define FEP_CLIENT_IDENTIFIER "Client_FEP_Element_diagnostics_example_3A34CVKl86D"
//Unique Identifier of the server fep element
#define FEP_SERVER_IDENTIFIER "Server_FEP_Element_diagnostics_example_3A34CVKl86D"
//Maximum timeout that should be used in search for minimal necessary timeout
#define MAX_TIMEOUT 5000
//granularity of the search for minimal necessary timeout in milli seconds
#define TIMEOUT_INCREMENT 100
//Name of "remote" property that will be tested
#define ELEMENT_TEST_PROPERTY "TestPropertyString"
//Signal name of ping signal
#define PING_SIGNAL_NAME "Ping_Signal_3A34CVKl86D"
//Signal name of pong signal
#define PONG_SIGNAL_NAME "Pong_Signal_3A34CVKl86D"
//Path to ddl description file
#define DDL_DESCRIPTION "descriptions/demo_diagnostics.description"
#endif

