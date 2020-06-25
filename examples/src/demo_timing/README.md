# Example: Distributed simulation using FEP 2 Timing  {#demo_timing}


This is an example on how to use FEP to implement, describe, deploy and run a distributed simulation

Location: **examples/src/demo_timing**

### What is demonstrated

This example system implements a distributed system simulating three cars driving in a row on one lane,
in which the middle car is accelerated and decelerated until reaching fixed distances to the front and
rear car while the outer cars are accelerating and decelerating simultaneously.

The simulation system is implemented in multiple participants. The following diagram schematically describes the system structure:
\image html Timing_Demo.png

The goal of this example is to show the user how to implement a time synchronized simulation containing
multiple interdependent participants by means of FEP 2 Timing.

This version of the FEP Timing example uses the FEP 2 Legacy Timing. Another version of this example using the
FEP 3 Timing is available at @ref demo_timing_30.


### Running the example

The example can be run in three different configurations
- **Default**: The execution speed will try to match the system time (demo_timing_start_default.(cmd|sh))
- **Default (faster)**: The execution speed will increase up to twice as fast as system time progresses (demo_timing_start_default_faster.(cmd|sh))
- **Strict**: Same as default, but timing violations will be detected and the simulation halted (demo_timing_start_strict.(cmd|sh))

To run the example simply start one of the scripts above. This will launch all participants and configure them accordingly, using the FEP Launcher (see @ref fep_description_tooling).

To start the system, simply run the demo_timing_start_controller (.sh/.cmd) script. This will start a
FEP Controller instance (see @ref fep_description_tooling) which you can then issue commands to.

The simplest sequence of commands would be: "**start** *ENTER*" to start the system and then "**stop** *ENTER* **shutdown** *ENTER* when you want to stop it and shut down the participants.


### The visualization in the Observer Participant

When the system is running, the observer participant prints the positions of the three cars to the command line each step.

The positions are symbolized by B for the rear car, A for the car in front and O for the EGO car.


### Timing Configurations

While the **Default** and **Default (faster)** configurations differ only in Timing Master configuration,
**Strict** uses a separate Timing Configuration (see @ref fep_timing_2)

The Timing Configurations are located in demo_timing_files/timing_configurations.
