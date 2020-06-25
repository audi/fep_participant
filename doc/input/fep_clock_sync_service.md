
# FEP Clock Sync Service {#page_fep_clock_sync_service}

The \ref page_fep_clock_sync_service provides functionality to synchronize the local time of a participant with a configured timing master. For this purpose, two native timing slave clock implementations are provided. To synchronize a timing slave with a timing master, the timing master and the corresponding main clock have to be configured using FEP properties. Depending on the timing master's main clock, the following slave clock implementations can be configured for a timing client.

\section clock_sync_native Native Implementations

You can choose among the following built-in native implementations.

\subsection clock_sync_continuous slave_master_on_demand 

The \ref clock_sync_continuous is a continuous timing slave. It cyclically requests the current simulation time from the timing master's *continuous clock* and synchronizes it's local time with the time of the master using the [Christian's algorithm](https://de.wikipedia.org/wiki/Algorithmus_von_Cristian).

Properties the @ref clock_sync_continuous uses.

| Name               | Code Macro                                    | Description                                                                                    | 
| ----               | ----                                          |-----                                                                                           |
| "SyncCycleTime_ms" | @ref FEP_CLOCKSERVICE_SLAVE_SYNC_CYCLE_TIME   | Frequency for which the service will request the time of the master. |
| "strMasterElement" | @ref FEP_TIMING_MASTER_PARTICIPANT            | Name of the timing master. |

**A concrete setup could look like that:**

- The timing master has to use a *continuous* main clock (e.g. \ref local_system_realtime).
- The timing slave uses the \ref clock_sync_continuous to cyclically (here every 100ms) synchronize its local time with the timing master.

|                                                | Master Participant                    | Client Participant      |
|-                                               | ------------                          | ------------            |
| Clock Service      - clock type - clock name   | Continuous Clock                      | "slave_master_on_demand"|
| Clock Sync Service - sync cycle time           |                                       | 100ms                   |
| Clock Sync Service - master name               |                                       | "Master Participant"    | 

\subsection clock_sync_discrete slave_master_on_demand_discrete 

The \ref clock_sync_discrete is a Discrete Timing Slave. After registration it receives *update time event* s from the timing master's *discrete* clock.

Properties the @ref clock_sync_discrete uses.

| Name               | Code Macro                               | Description                                             | 
| ----               | ----                                     |-----                                                    |
| "strMasterElement" | @ref FEP_TIMING_MASTER_PARTICIPANT       | Name of the timing master the service will register to. |

**A concrete setup could look like that:**

- The timing master has to use a *discrete* main clock (e.g. \ref local_system_simtime)
- The timing slave uses the \ref clock_sync_discrete to receive *update time event* s from the timing master and sets it local time accordingly.

|                                                | Master Participant                    | Client Participant                 |
|-                                               | ------------                          | ------------                       |
| Clock Service      - clock type - clock name   | Discrete Clock                        | "slave_master_on_demand_discrete"  |
| Clock Sync Service - master name               |                                       | "Master Participant"               |

The @ref clock_sync_discrete will try to register to the timing master (\ref FEP_TIMING_MASTER_PARTICIPANT) when going to \ref fep::FS_RUNNING. If the timing master is not available by that time he will cyclically try to register again.

\section clock_sync_details Clock Sync Service Details

##### Participant Internal Interface

The [IClockSyncService](classfep_1_1_i_clock_sync_service.html) interface offers functionality to register multiple schedulers and jobs and to set an active scheduler for the participant.

##### Participant Service Interfaces 

* @ref fep::rpc::IRPCClockSyncSlaveDef, [clock_sync_slave.json](../../include/fep3/rpc_components/clock/clock_sync_slave.json)