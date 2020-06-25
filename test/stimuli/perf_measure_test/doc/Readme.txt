
Copyright @ 2019 Audi AG. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

Performance Measurements
========================

About
-----
Performance measurements are done using the stimuli "perf_measure_stimuli". 
This program is used to measure the round-trip time. The following setup is
used:

------------------------                                     ------------------------
|                      |            Request                  |                      |    
| perf_measure_stimuli | ----------------------------------> | perf_measure_stimuli |
|                      | t0                               t1 |                      |
|                      |            Response                 |                      |
|     Client (-C)      | <---------------------------------- |     Server (-S)      |
|                      | t3                               t2 |                      |
------------------------                                     ------------------------
  
The "perf_measure_stimuli" is used in two different configuration:
* Client (Left side): Is producing packets and sending requests and receiving responses.
* Server (Right side): Is receiving request and sends a response for each request

The program is using timestamps:
* t0: Request Send time (set by Client)
* t1: Request Receive time (set by Server)
* t2: Response Send time (set by Server)
* t3: Response Receive time (set by Client)

Please note: As client and server might be in differnt time domains (e.g. different hosts)
it is not valid to compare server and client timestamps directly. This is the reason why
round trip times are used.

These timestamps can be used to calculate performance relevant results:
* RTT (Round-Trip-Time): Calculate as "RTT = t3 - t0"
* STT (Single-Trip-Time): 
  Calculate as "STT[Send] = t1 - t0" (Only valid if Client/Server are in same time domain)
  Calculate as "STT[Recv] = t3 - t2" (Only valid if Client/Server are in same time domain)
* WT (Work time): Calculated as "WT= t2 - t1"

The RTT is the relevant timestamp.


Arguments
---------
To list all arguemnts please use the help page of "perf_measure_stimuli".

Measurement
-----------

### Start the Server
Example Command Line:
$ ./perf_measure_stimuli.exe -v 10 --transmission dds --domain 109 --server -0 --bytes 1024 --ddbsize 0 -1 --bytes 1024 --ddbsize 0 -2 --bytes 1024 --ddbsize 0 -3 --bytes 1024 --ddbsize 0 -4 --bytes 1024 --ddbsize 0
Arguments used:
-v 10: Set verbosity to 10 (highest verbosity)
--transmission dds: Set used transmission adapter (dds is default)
--domain 109: Set a domain (Do not interfere with other clients)
--server: Server mode
-0: Following arguments are for the first signal (Number #0)
--bytes 1024: Size of the packets (Request and Response) in bytes
--ddbsize 0:  Disabled DDB
-1: Following arguments are for the second signal (Number #1)
...

### Start the Client
Example Command Line:
$ ./perf_measure_stimuli.exe -v 10 --transmission dds --domain 109 --client  -f 1000 --bytes 1024 --ddbsize 0 -1 -f 999 --bytes 1024 --ddbsize 0 -2 -f 666 --bytes 1024 --ddbsize 0 -3 --bytes 1024 --ddbsize 0 -4 --bytes 1024 --ddbsize 0
Arguments used:
-v 10: Set verbosity to 10 (highest verbosity)
--transmission dds: Set used transmission adapter (dds is default)
--domain 109: Set a domain (Do not interfere with other clients)
--client: Client mode
-0: Following arguments are for the first signal (Number #0)
-f 1000: Set the send/produce frequency
--bytes 1024: Size of the packets (Request and Response) in bytes
--ddbsize 0:  Disabled DDB
-1: Following arguments are for the second signal (Number #1)
...

Please note: Client/Server configuration must match, especially size of packets (--bytes) and DDB 
configuration (--ddbsize). The frequency is only relevent for the client.

To execute a measuremnt series a python script is available. 
For more information see "DoingPerformanceMeasure.txt".




