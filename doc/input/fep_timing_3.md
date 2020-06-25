
# FEP 3 Timing {#page_fep_timing_3}

To support the clear separation of features into FEP Components the new timing concept of FEP SDK
2.3 will introduce several configurable ways to synchronize the FEP %System. The relevant participant
components are:

1. \ref page_fep_clock_service
    - to register and set a dedicated clock by name
2. \ref page_fep_clock_sync_service
    - to provide different clocks by registration at the Clock Service for clock synchronization modes
3. \ref page_fep_scheduler_service
    - to register jobs (execution-points-for-things-to-do) at a single component to provide information
    - to register and set a dedicated scheduling mode

\section fep_timing_usage FEP Timing Usage

### FEP 3 Timing

Every participant uses the old FEP 2 Legacy Timing by default, unless it is configured otherwise.
To actively configure a participant to use the new FEP 3 Timing, we just have to create a cModuleOptions object
and set the default timing accordingly. Afterwards we can use the module options to create a FEP Participant.

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp SetupFEPTiming25

Setting FEP 3 Timing as default timing is equal to setting the Participant's @ref FEP_CLOCKSERVICE_MAIN_CLOCK
and @ref FEP_SCHEDULERSERVICE_SCHEDULER properties to @ref FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME
and @ref FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_CLOCK_BASED as shown in the
[No time synchronization (default) configuration](#fep_timing_25_default_configuration).

### FEP 2 Timing (Legacy)

To configure a participant to use the old @ref fep_timing_2, we have to create a cModuleOptions object
and set the default timing accordingly. Afterwards we can use the module options to create a FEP Participant.

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp SetupFEPTiming20

Setting FEP 2 Timing as default timing is equal to setting the Participant's @ref FEP_CLOCKSERVICE_MAIN_CLOCK
and @ref FEP_SCHEDULERSERVICE_SCHEDULER properties to @ref FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_MASTER_LOCKED_STEP_SIMTIME
and @ref FEP_SCHEDULERSERVICE_SCHEDULER_VALUE_MASTER_LOCKED_STEP_SIMTIME as shown in the
[Master/Slave locked fixed step distributed scheduling configuration](#fep_timing_20_default_configuration).

\section time_synchronization_and_distributed_scheduling Time Synchronization and Distributed Scheduling

The following section lists various timing and scheduling use cases and the corresponding configuration.

\subsection fep_timing_25_default_configuration No time synchronization

**Use Cases:**
- Development of new participants
- Non time dependent functional testing of participants

**Configuration:**

|                                    | Participant 1         | Participant 2         | Participant 3     |
|-                                   | ------------          | ------------          | --------------    |
| Clock Service - clock name         | local_system_realtime     | local_system_realtime     | local_system_realtime |
| Scheduler Service - scheduler name | clock_based_scheduler | clock_based_scheduler | clock_based_scheduler |

**This configuration means:**
- *No* time synchronization is used.
- Each local system time will *continue* by its system clock.
- The scheduler will continuously look at the time and will execute configured jobs (timers) at the corresponding point in time.
- The scheduler will watch runtime execution times and will raise configured runtime violation strategy incidents as configured.

**Code example**
- Every FEP participant

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp SetupNoTimeSynchronization

\subsection master_slave_synchronization Master/Slave time synchronization

\subsubsection fep_timing_25_built_in_local_real_configuration Built-in continuous real-time


**Use Cases:**
- Realtime mode within a realtime environment like vehicle prototypes or other distributed systems
- No fixed distributed scheduling
- Data driven systems

**Configuration:**

|                                                | Participant 1         | Participant 2           | Participant 3          |
|-                                               | ------------          | ------------            | --------------         |
| Clock Service      - clock name                | local_system_realtime     | slave_master_on_demand  | slave_master_on_demand |
| Clock Sync Service - update time/master_set    |                       | 100ms/"Participant 1"   | 100ms/"Participant 1"  |
| Scheduler Service - scheduler name             | clock_based_scheduler | clock_based_scheduler   | clock_based_scheduler  |

**This configuration means:** 
- *Only* time synchronization is used.
- *Participant 2* and *Participant 3* are timing slaves, the *slave_master_on_demand* clock will
  synchronize every 100ms with the configured master *Participant 1*.
- The slave times are straightened by the *Christians Algorithm* with round trip.
- The *slave_master_on_demand* clock will interpolate the time between the 100ms synchronization
  step.
- *Participant 1* will provide the time by a *continuous* local system real time.
- The scheduler will continuously look at the time and will execute the configured jobs (timers) at the corresponding point in time.
- The scheduler will watch runtime execution times and will raise the configured runtime violation strategy incidents as configured.

**Code example**
- Timing master

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp SetupNoTimeSynchronization

- Timing clients

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp ClientSetupLocalRealTimeSynchronization

\subsubsection external_realtime_clock External real-time clock

**Use Cases:**
- Realtime mode within a realtime environment like vehicle prototypes or other distributed systems
- No fixed distributed scheduling
- The time source clock *my_fancy_realtime* is provided by an external implementation

**Configuration:**

|                                                | Participant 1         | Participant 2           | Participant 3          |
|-                                               | ------------          | ------------            | --------------         |
| Clock Service      - clock name                | my_fancy_realtime     | slave_master_on_demand  | slave_master_on_demand |
| Clock Sync Service - update time/master_set    |                       | 100ms/"Participant 1"   | 100ms/"Participant 1"  |
| Scheduler Service  - scheduler name            | clock_based_scheduler | clock_based_scheduler   | clock_based_scheduler  |

**This configuration means:** 
- *Only* time synchronization is used.
- *Participant 2* and *Participant 3* are timing slaves, the *slave_master_on_demand* clock will synchronize every 100ms with the configured master *Participant 1*
- The slave times are straighten by the *Christians Algorithm* with round trip.
- The *slave_master_on_demand* clock will interpolate the time between the 100ms synchronization
  steps.
- *Participant 1* will provide the time by a *continuous* external time called *my_fancy_realtime*
- The scheduler will continuously look at the time and will execute the configured jobs (timers) at the corresponding point in time.
- The scheduler will watch runtime execution times and will raise the configured runtime violation strategy incidents as configured.

**Code example**
- Timing master

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp MasterSetupExternalRealTimeSynchronization

- Timing clients

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp ClientSetupLocalRealTimeSynchronization

For an example of how to create a custom continuous or discrete clock and how to implement a FEP Timing Master which utilizes those custom clocks,
please have a look at \ref page_demo_timing_master_external_clock.


\subsection master_slave_clock_based  Master/Slave time synchronization with clock based distributed scheduling

\subsubsection fep_timing_25_built_in_local_sim_configuration Built-in fixed time steps

**Use Cases:**
- Simulated deterministic test environment
- Fixed distributed scheduling with *discrete* logical time (usually no realtime!)

**Configuration:**

|                                                | Participant 1                         | Participant 2           | Participant 3          |
|-                                               | ------------                          | ------------            | --------------         |
| Clock Service      - clock name                | local_system_simtime (100ms)    | slave_master_on_demand_discrete  | slave_master_on_demand_discrete |
| Clock Sync Service - update time/master_set    |                                       | 100ms/"Participant 1"   | 100ms/"Participant 1"  |
| Scheduler Service  - scheduler name            | clock_based_scheduler                 | clock_based_scheduler   | clock_based_scheduler  |

**This configuration means:** 

- Time synchronization is used by connecting to the *Clock Service* of the configured master.
- Each *slave_master_on_demand_discrete* clock will receive time update events from the configured master.
- The *Clock Service* of the master will wait at each time step for the *slaves confirmation event* that the time has been reached.
- The *clock_based_scheduler* will be informed about each logical time step, will execute the
  configured jobs (timers)-
- The scheduler will watch runtime execution times in real-time and will raise the configured
  runtime violation strategy incidents as configured.

**Code example**

- Timing master

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp MasterSetupLocalDiscreteTimeSynchronization

- Timing clients

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp ClientSetupLocalDiscreteTimeSynchronization

\subsubsection external_lock External clock (i.e. player)

**Use Cases:**
- Simulated deterministic test environment
- Fixed distributed scheduling by a *discrete* logical time from the external clock (i.e. an
  external player!)

**Configuration:**

|                                                | Participant 1                         | Participant 2           | Participant 3          |
|-                                               | ------------                          | ------------            | --------------         |
| Clock Service      - clock name                | external_player                       | slave_master_on_demand_discrete  | slave_master_on_demand_discrete |
| Clock Sync Service - update time/master_set    |                                       | 100ms/"Participant 1"   | 100ms/"Participant 1"  |
| Scheduler Service  - scheduler name            | clock_based_scheduler                 | clock_based_scheduler   | clock_based_scheduler  |

**This configuration means:**

- Time synchronization is used by connecting to the *Clock Service* of the configured master.
- Each *slave_master_on_demand_discrete* clock will receive time update events from the configured
  master.
- The *Clock Service* of the master will wait at each time step for the *slaves confirmation event*
  that the time has been reached.
- The *clock_based_scheduler* will be informed about each logical time step and will execute the
  configured jobs (timers).
- The scheduler will watch runtime execution times in real-time and will raise the configured
  runtime violation strategy incidents as configured.

**Code example**
- Timing master

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp MasterSetupExternalDiscreteTimeSynchronization

- Timing clients

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp ClientSetupExternalDiscreteTimeSynchronization

\subsubsection fep_timing_20_default_configuration Master/Slave locked fixed step distributed scheduling (FEP 2.1 Timing)

**Use Cases:**
- Simulated deterministic test environment
- Locked fixed step scheduling by a *discrete* logical simulation time where the steps are calculated by a timing master
- **This is the 2.1 legacy timing**

**Configuration:**

|                                                | Participant 1                         | Participant 2           | Participant 3          |
|-                                               | ------------                          | ------------            | --------------         |
| Clock Service      - clock name                | locked_step_simtime                   | locked_step_simtime     | locked_step_simtime    |
| Timing Legacy      - update time/master_set    | 100ms/"Participant 1"                 | 100ms/"Participant 1"   | 100ms/"Participant 1"  |
| Scheduler Service  - scheduler name            | locked_step_simtime_scheduler         | locked_step_simtime_scheduler   | locked_step_simtime_scheduler  |

**This configuration means:** 

- The time is set through the scheduler implementation.
- The master *Participant 1* will calculate the logical time steps and execution times of the
  registered jobs of each *slave*.
- The *slaves* are triggered separately by the *master* and confirm the job executions.

**Code example**

- Every FEP Participant

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp SetupLegacyTiming

For more information regarding the FEP Timing Legacy, please have a look at @ref fep_timing_2.


## Example

- @ref demo_timing_30


\section time_synchronization_explained Time Synchronization and Distributed Scheduling explained

The section @ref realtime_synchronization refers to the setups in \ref master_slave_synchronization.
The section @ref discrete_time_synchronization refers to the setups in \ref master_slave_clock_based excluding the setup @ref fep_timing_20_default_configuration.


\subsection realtime_synchronization Continuous Synchronization

In case of a continuous synchronization the master and the client will each run with their own clock.
In an interval of @ref FEP_CLOCKSERVICE_SLAVE_SYNC_CYCLE_TIME the client will ask the master for it's current time and synchronize it's clock to that time. The @ref page_fep_scheduler_service of the clients will schedule jobs based on that time.

**For this Use Case the following components are used:**

* \ref page_fep_clock_service (@ref local_system_realtime)
* \ref page_fep_clock_sync_service (@ref clock_sync_continuous)
* \ref page_fep_scheduler_service

\subsubsection continuous_a_short_example An Example

The following demonstrates such a time synchronization.

**We have the following setup**

* A **timing master**  (@ref local_system_realtime).
* A **timing client1** which has one job with a cycle time of 5ms (@ref clock_sync_continuous (100ms)).
* A **timing client2** which has one job with a cycle time of 10ms ( @ref clock_sync_continuous (50ms)).

Due to the @ref FEP_CLOCKSERVICE_SLAVE_SYNC_CYCLE_TIME the client1 will ask for the master's time every 100ms and synchronize his clock to that time. The client2 will do the same but every 50ms. 
Client1 will then, based on his synchronized clock, schedule a job every 5ms. Client2 will schedule a job every 10ms.

If using this kind of time synchronization the clocks could drift apart in between the synchronization intervals.

\subsection discrete_time_synchronization Discrete Time Synchronization

The master emits *update time event* s whenever it's time is changing.
The event is sent to all timing clients. If a timing client receives such an event it might schedule one or more jobs.
When a timing master is emitting _update time events_ is dependent on his implementation. 

The @ref local_system_simtime clock for example sends *update time event* in the resolution of the property @ref FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME based on a local operation system clock.
An *external_player* might send *update time event* s whenever it processes data with a new timestamp (of course that is dependent on the concrete implementation). The @ref clock_sync_discrete sync clock on the client side will receive the *update time event* and update the local clock of the client. The @ref page_fep_scheduler_service might then schedule jobs based on the new time.

For a timing master to send *update time event* the clients have to register with the master. To do so the clients have to specify the property \ref FEP_TIMING_MASTER_PARTICIPANT. 

\note be aware that the discrete synchronization of one participant over the network currently takes at least 4ms. Depending on your @ref FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME this will lead to a simulation that runs slower than wall clock.


**For this Use Case the following components are used:**

* \ref page_fep_clock_service (@ref local_system_simtime)
* \ref page_fep_clock_sync_service (@ref clock_sync_discrete)
* \ref page_fep_scheduler_service

\subsubsection discrete_a_short_example An Example

The following demonstrates such a time synchronization.

**We have the following setup**

* A **timing master** emitting _update time events_ for every ms (@ref local_system_simtime (1ms)).
* A **timing client1** which has one job with a cycle time of 5ms (@ref clock_sync_discrete).
* A **timing client2** which has one job with a cycle time of 10ms ( @ref clock_sync_discrete).

Due to the @ref FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME of 1ms the master sends a *update time event* for every ms it simulates. The event is sent to all timing clients in non determined order.
For every call it sends, it waits for the timing client to process the event. Processing the *update time event* means that the client checks, based on the received time, whether it needs to trigger a job.
If jobs need to be triggered the timing client will wait with his response until all jobs are executed. This ways the timing master will also wait until completion of the job executions.

**In detail this will lead to the following behaviour**

\image html img/discrete_time_synchronisation.png

The following table outlines the behaviour for a simulation duration of 10ms.

| Time      | What happens                                                                                                      |
| ----      | ----                                                                                                              | 
| -         | When going to @ref fep::FS_RUNNING client1 and client2 will register themselves as timing clients.                |
| 0ms - 4ms | For the first 4ms the clients will immediately return the call executing no job.                                  |
| 5ms       | After 5ms the client1 will trigger it's 5ms Job. The Timing master waits until the job is processed.              |
| 6ms -9 ms | Again no job is scheduled.                                                                                        |
| 10ms      | After 10ms the client1 will trigger it's 5ms Job again. The client2 will trigger his 10ms job for the first time. |