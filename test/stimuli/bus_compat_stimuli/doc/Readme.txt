
Copyright @ 2019 Audi AG. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

Bus Compatibility Stimuli
=========================

The "Bus Compatibility Stimuli" serves as a stimuli for the "Bus Compatibility Test". 
The "Bus Compatibility Test" was introduced to check and assert compatibility between different 
versions of FEP SDK. FEP participants built using different minor versions of FEP SDK should be 
able to run and interact in a single FEP system without any restrictions with respect to the 
feature set of the lower version.

Task of Bus Compatibility 
-------------------------

* Bus comatibility allows FEP participant using different MINOR versions of the FEP SDK to interact. 
* Currently FEP SDK version 1.x (MAJOR version 1) is subject of the bus compatibility tests
* "System Bus" and "Data Bus" must be fully compatible. Naturally, this relates to the feature set 
  of the lower version.

Bus Compatibility Check
----------------------

The "Bus Compatibility Stimuli" is a single executable running in two modes.

* The server mode: Serves a a responder and counterpart for the client
* The client mode: Executes requests and checks results

Running the comatibility test is done:

* First start the server
```shell
bus_compat_stimuli server
```

* In another shell start the client
```shell
bus_compat_stimuli client
```

The client should report sucess statements for all tests:
```text
* SUCCESS: Control Command requesting Initialize (0)
...
* SUCCESS: ResultCode Notification with some value (0)
```

There should not be any failure reports:
```text
...
* FAILURE: SetProperty Command with Integer Value (0) (-3)
** Unexpected: Update IStateNotification```
...
```

In case of failures, please check the following conditions:
* Are you using a dedicated domain for the tests, if not use the "-domain" argument
* Are timeouts occuring? Are unexpected failures occuring? The system load might be to high. 
  Stop other processes and rerun the test.

Extending Bus Compatibility Check
--------------------------------

The "Bus Compatibility Check" consists of a bunch of single compatibility tests, each encapsulated 
in its own class and dedicated to a special topic of bus compatibility. 

Extending the "Bus Compatibility Check" can be easily achieved by creating a new check class. Start 
with a copy of an existing test class. 

The newly created test needs to be integrated into the test execution (main.cpp) by adding it to the 
function "client_main".  

If additional or adjusted responses are needed, the server part (module_server.cpp) needs to be adjusted
as well. This might be the case if you want to send back the Command Message. Note that notifications 
corresponding to an incoming command will be issued by the underlying FEP Element by default.





