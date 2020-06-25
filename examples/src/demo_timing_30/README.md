# Example: Distributed simulation using FEP 3 Timing {#demo_timing_30}

This is the FEP 3 Timing version of the FEP 2 Timing Example.
Instead of using the FEP Legacy Timing, this examples utilizes the FEP 3 Timing.

This is an example on how to use FEP to implement, describe, deploy and run a distributed simulation.

Location: **examples/src/demo_timing_30**

### What is demonstrated

This example system implements a distributed system simulating three cars driving in a row on one lane,
in which the middle car is accelerated and decelerated until reaching fixed distances to the front and
rear car while the outer cars are accelerating and decelerating simultaneously.

The simulation system is implemented in multiple participants. The following diagram schematically describes the system structure:
\image html Timing_Demo.png

The goal of this example is to show the user how to implement a time synchronized simulation containing
multiple interdependent participants by means of FEP Timing.

This version of the FEP Timing example uses the new FEP 3 Timing and the new FEP Participant. Another version of this example using the
old FEP 2 Legacy Timing is available at @ref demo_timing.

### Running the example

The example can be run in three different configurations
- **System Time**: A continuous clock is used to execute the example at system time speed (demo_timing_30_start_system_time.(cmd|sh))
The behaviour of this example may vary because this example simulates a distributed real time system.
Participants are cyclical clock-synchronized after a configurable amount of time but apart from that every participant runs on its own. Therefore, a deterministic behaviour can not be guaranteed.
- **Simulation Time**: A discrete clock is used to execute the example at configurable discrete time steps (demo_timing_30_start_simulation_time.(cmd|sh))
- **AFAP Time**: A discrete clock is used to execute the example at the fastest speed possible (demo_timing_30_start_afap_time.(cmd|sh))

To run the example simply start one of the scripts above. This will launch all participants and configure them accordingly, using the FEP Launcher (see @ref fep_description_tooling).

To start the system, simply run the demo_timing_30_start_controller (.sh/.cmd) script. This will start a
FEP Controller instance (see @ref fep_description_tooling) which you can then issue commands to.
(Example startup may take some time).

The simplest sequence of commands would be: "**start** *ENTER*" to start the system and then "**stop** *ENTER* **shutdown** *ENTER* when you want to stop it and shut down the participants.


### The visualization in the Observer Participant

When the system is running, the observer participant prints the positions of the three cars to the command line each step.

The positions are symbolized by B for the rear car, A for the car in front and O for the EGO car.


### Timing Configurations

While the **Simulation Time** and **AFAP Time** configurations both utilize a discrete clock and differ only in Timing Master configuration,
**System Time** uses a continuous clock and differs in Timing Master configuration as well as in Timing Client configuration (see @ref page_fep_timing_3)

#### Note: 
The structure of the source code in this example differs from the one found in the FEP 2 example (examples/src/demo_timing). This is due to the use of DataJob (@ref fep::DataJob) and 
ParticipantFEP2 (@ref fep::ParticipantFEP2).
