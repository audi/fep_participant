
Copyright @ 2019 Audi AG. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

DDS Comparison (Project only_dds_test)
======================================

About
-----
The task of the program is to compare different DDS implementation concerning performance and scalability.

------------------------                                     ------------------------
|                      |            Request                  |                      |    
| test_client_...      | ----------------------------------> | test_server_...      |
|                      | t0                               t1 |                      |
|                      |            Response                 |                      |
|     Client           | <---------------------------------- |     Server 0..N      |
|                      | t3                               t2 |                      |
------------------------                                     ------------------------

The communication is between:
* Client (Left side): Is producing packets and sending requests and receiving responses.
* Server (Right side): Is receiving request and sends a response for each request

Following limitations apply:
* It is possible to run multiple servers, but always only one client.
* The servers must be started before starting the client.
* The servers must get sequential Ids (0..N-1)
* The client needs to know the number of servers (N)

Note on the terms:
* During this document the terms client and servers are used.
* This convention was chosen as they are always publishers and subscribers (publish and subscribers are typical DDS-Terms) or writers and readers.

The program is using timestamps:
* t0: Request Send time (set by Client)
* t1: Request Receive time (set by Server)
* t2: Response Send time (set by Server)
* t3: Response Receive time (set by Client)

Please note: As client and server might be in different time domains (e.g. different hosts) it is not valid to compare server and client timestamps directly. This is the reason why round trip times are used.

These timestamps can be used to calculate performance relevant results:
* RTT (Round-Trip-Time): Calculate as "RTT = t3 - t0"

Command Line
------------
Note: Always start matching combinations of client/server (Same file names except client/server)

* Client and server: common parts of command line (The first for arguments apply to client and server and must match.)
  <size> (mandatory) sample size to use. default is 1024.
  <mode> (mandatory) is 0 (BEST_EFFORT) or 1 (RELIABLE)
  <multicast> (mandatory) is 0 (UNICAST) or 1 (MULTICAST)
  <async> (mandatory) is 0 (SYNC) or 1 (ASYNC)

* Server only: This argument only applies to the server
  <server-id> (optional) is a server-id from 0 to 31

* Client only: This argument only applies to the client
  <number-of-servers> (mandatory) number of servers expected 1 to 32
  <frequency> (mandatory) is number 1 ... 1000000
  <name-of-measurement> (optional) append this text to measure file

Note: The <server-id> starts at zero. The minimal value of <number-of-servers> is one.  
  
1. Start servers
Start two servers (sample size is 1024 bytes, mode is reliable, multicast and async are disabled) with id 0 and 1
$ test_server_rtisdds_queue 1024 1 0 0 0
$ test_server_rtisdds_queue 1024 1 0 0 1

2. Start client
Always use the same values for the first four arguments. The number of servers in this example is 2. Frequency used is 100Hz.
$ test_client_rtisdds_queue 1024 1 0 0 2 100

The measurement time (run time of the client) is hard coded to 10s. The client will terminate. The server needs to be terminated manually.

The output is formatted as CSV. The first line is the header line and should be used to interpret the output.

Measurement script
------------------

The measurement script is using "python 3". It is recommended to download python 3 from https://anaconda.org/ as this distribution contains all required modules.

Note: Script will not run with "python 2".

To run the script open a console and switch to the directory containing the executables (test client and server). Alos make sure that "python 3" is the python version used. The command "python --version" should state python version 3.

Now run the script using the full path to the script file, e.g.: 
$ python /<path-to-the-script>/run_only_dds_test.py

Note: The script will write error messages like "Error: Process not found". This messages should be ignored (message is produced when termination a process which is not running).

Running the script will need some hours. When finished the directory contains all measurements done as graphics (png- and pdf-files) and as comma separated values (csv-file).

