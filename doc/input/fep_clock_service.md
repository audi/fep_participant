# FEP Clock Service {#page_fep_clock_service}

The \ref page_fep_clock_service is able to provide one ore more different clocks which may represent different
times with a different timebase. A clock is described by its name and its type.

There are two ways to set a main clock for a specific participant:

1. **Programatically in the code**

    `getComponent<IClockService>(participant)->setMainClock("local_system_realtime");`

2. **Using a property**

    * PropertyTree: `"Clock.MainClock"`
    * Code Macro: @ref FEP_CLOCKSERVICE_MAIN_CLOCK


See \ref clock_service_native_implementations for the native implementations shipped with FEP. 

Furthermore, the clock service is able to use clocks provided by the participants implementation.
For an example of how to create a custom continuous or discrete clock and how to implement a FEP Timing Master which utilizes those custom clocks,
please have a look at \ref page_demo_timing_master_external_clock.


\section clock_service_clock_type Clock Type

For any clock The FEP SDK supports two types.

\subsection clock_service_clock_type_discrete Discrete

The **discrete clock** actively sets a timestamp and synchronously propagates an internal updating
event to each participant using this clock type.

see \ref local_system_simtime for an implementation.

\subsection clock_service_clock_type_continuous Continuous

The **continuous clock** will provide real time (or simulated real time) on each `getComponent<IClockService>(participant)->getTime()` call.

see \ref local_system_realtime for an implementation.

\section clock_service_usage Clock Service Usage

The \ref page_fep_clock_service may be used to register various custom continuous or discrete clocks and to set a main clock by either setting the corresponding FEP property or by using the clock service itself.

\section clock_service_configuration Clock Service Configuration

A clock has to be registered at the clock service before it can be set as the main clock of a participant.

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp RegisterContinuousClock

The clock service stores all registered clocks and provides the possibility to set the active main clock of a participant.

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp SetContinuousClock

Alternatively the main clock may be set by using the local clock service. 

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp RegisterDiscreteClock

\snippet snippets/snippet_clock_service/snippet_clock_service.cpp SetDiscreteClock

\section clock_service_native_implementations Native Implementations

You can choose among the following built-in native clock implementations:

\subsection local_system_simtime local_system_simtime

| Name | Code Macro                               | Type        | Description        | 
| ---- | ----                                     | -----       |-----               |
| "local_system_simtime" |@ref FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_SIM_TIME | Discrete    | This clock will set the time in logical time steps adjusted by a property and will (try to) follow the realtime tickcount by the adjusted factor. For an example configuration, please have a look at @ref fep_timing_25_built_in_local_sim_configuration.|

Properties of the built-in clock @ref local_system_simtime :

| Name | Code Macro                               | Description        | 
| ---- | ----                                     |-----               |
| "CycleTime_ms" |@ref FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_CYCLE_TIME   | This property defines the length of a single discrete time step. The clock will wait for this period of time until the next time update event is triggered. The **default value** is 100 ms. |
| "TimeFactor_float" |@ref FEP_CLOCKSERVICE_MAIN_CLOCK_SIM_TIME_TIME_FACTOR    | This factor stretches or shrinks the discrete time steps in relation to the system time. A factor < 1 means the discrete time step lasts longer compared to the system real time. A factor > 1 means the discrete time step passes faster compared to the system real time. A factor of 0.0 means the clock does not wait between time steps. The **default value** is 1,0.  |



\subsection local_system_realtime local_system_realtime

| Name | Code Macro                               | Type        | Description        | 
| ---- | ----                                     | -----       |-----               |
| "local_system_realtime" | @ref FEP_CLOCKSERVICE_MAIN_CLOCK_VALUE_LOCAL_SYSTEM_REAL_TIME | Continuous   | This clock will return the tickcount of the current time based by 0 (where usually 0 is the time stamp of starting the computer). The precision of the time depends on the cpu frequency. For an example configuration, please have a look at @ref fep_timing_25_built_in_local_real_configuration|

\section clock_service_details Clock Service Details

##### Participant Internal Interface

The [IClockService](classfep_1_1_i_clock_service.html) interface offers functionality to register multiple clocks and to set an active main clock for the participant.

##### Participant Service Interfaces 

* @ref fep::rpc::IRPCClockServiceDef, [clock.json](../../include/fep3/rpc_components/clock/clock.json)
* @ref fep::rpc::IRPCClockSyncMasterDef, [clock_sync_master.json](../../include/fep3/rpc_components/clock/clock_sync_master.json)
