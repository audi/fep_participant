# Example: Using FEP for a realtime cascade using FEP Timing 3.0 {#demo_realtime_cascade_30}

This is an example on how to use FEP to implement a simple real time cascade using new timing 3.0.

### Location

\code 
      ./src/examples/src/demo_realtime_cascade_30
\endcode
 
### What is demonstrated
This example demonstrates a simulation system that transports data in a cascade.
The "starter" element sends initial data, which is received by a sequence of "transmitter"-elements that 
forward the signals. The last "transmitter" sends back the data to the "starter".
An additional participant (Timing Master 3.0) provides a clock. The "starter" evaluate the data of each
transmission loop and start the next cascade iteration, until the maximum number of iterations is reached. Some statistics are shown after execution of the cascades. \n

### System structure
The following diagram schematically describes the system structure:
\image html Cascade_Demo.png ["Demo System Structure"]\n

### Running the example

The example can be run in three different configurations
- **System Time**: A continuous clock is used to execute the example at system time speed (demo_realtime_cascade_30_start_launcher_system_time.(cmd|sh))
The behaviour of this example may vary because this example simulates a distributed real time system.
Participants are cyclical clock-synchronized after a configurable amount of time but apart from that every participant runs on its own. Therefore, a deterministic behaviour can not be guaranteed.
- **Simulation Time**: A discrete clock is used to execute the example at configurable discrete time steps (demo_realtime_cascade_30_start_launcher_simulation_time(cmd|sh))
- **AFAP Time**: A discrete clock is used to execute the example at the fastest speed possible (demo_realtime_cascade_30_start_launcher_afap_time.(cmd|sh))

To run the example simply start one of the scripts above. This will launch all participants and configure them accordingly, using the FEP Launcher (see @ref fep_description_tooling).

To start the system, simply run the demo_realtime_cascade_30_start_controller(.sh/.cmd) script. This will start a
FEP Controller instance (see @ref fep_description_tooling) which you can then issue commands to.
(Example startup may take some time).

The simplest sequence of commands would be: "**start** *ENTER*" to start the system and then "**stop** *ENTER* **shutdown** *ENTER* when you want to stop it and shut down the participants.
