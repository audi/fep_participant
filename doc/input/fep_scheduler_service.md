
# FEP Scheduler Service {#page_fep_scheduler_service}

The @ref page_fep_scheduler_service is responsible for the scheduling of jobs within a *FEP System*. It is used to register jobs (@ref fep::IScheduler::IJob) and one or more schedulers (@ref fep::IScheduler). One of the schedulers is the so called *active scheduler*, which will trigger the jobs.

Every participant has one @ref page_fep_scheduler_service. Various participants in a *FEP system* may use different schedulers depending on the corresponding use case. 

The *FEP SDK* offers a native implementation of an @ref fep::IScheduler, the @ref scheduler_service_local_clock_based_scheduler, which will be used by default.

\section scheduler_service_usage Scheduler Service Usage

Similar to the registration and configuration of clocks at the @ref page_fep_clock_service, the @ref page_fep_scheduler_service  may be used to register various custom schedulers and jobs and set an active scheduler which triggers the jobs.

\subsection scheduler_service_registration Registration of Jobs

Jobs are cyclic processing units which can be registered at the scheduler service. A job can read data from @ref fep::DataReader s, process received data samples and send data to @ref fep::DataWriter s. The configured active scheduler triggers all jobs which are registered at the scheduler service.

\snippet snippets/snippet_scheduler_service/snippet_scheduler_service.cpp RegisterJob


\section scheduler_service_configuration Scheduler Service Configuration

A scheduler has to be registered at the scheduler service before it can be set as the active scheduler of a participant.

\snippet snippets/snippet_scheduler_service/snippet_scheduler_service.cpp RegisterScheduler

The scheduler service stores all schedulers which are registered for a participant and can set the active scheduler which is responsible for triggering jobs.

\snippet snippets/snippet_scheduler_service/snippet_scheduler_service.cpp SetScheduler


\section scheduler_service_native Native Implementations

You can choose among the following built-in native implementations:

\subsection scheduler_service_local_clock_based_scheduler local_clock_based_scheduler

The @ref scheduler_service_local_clock_based_scheduler triggers jobs based on the local time of the participant and the jobs cycle time. The local time is provided by the @ref page_fep_clock_service.
The jobs will cyclicly be triggered every time the corresponding job's cycle time passes.

Also for every job execution an execution time check is performed. The check is configured with the jobs @ref fep::JobConfiguration (see class documentation for details).

\note The parameter *delay_sim_time_us* is currently not evaluated

If you read samples from a @ref fep::DataReader created with a @ref fep::DataJob only samples with a timestamp smaller than the current simulation time will be provided.

\section scheduler_service_custom_implementation Custom Implementations

A custom scheduler implementation has to implement the @ref fep::IScheduler interface and provide functionality to trigger the jobs registered at the scheduler service.

\section scheduler_service_details Scheduler Service Details

#### Participant Internal Interface

The @ref fep::ISchedulerService interface offers functionality to register multiple schedulers and jobs and to set an active scheduler for the participant.

#### Participant Service Interfaces 

* still experimental: [scheduler.json](../../include/fep3/rpc_components/scheduler/scheduler.json)