
# FEP Data Transmission Best Practices {#page_fep_participant_data_transmission}

This page contains information regarding configuration best practices when using the @ref fep::IDataRegistry::IDataWriter and @ref fep::IDataRegistry::IDataReader API for data exchange within a FEP system.

For further information regarding @ref fep::IDataRegistry::IDataSample and how to use @ref fep::IDataRegistry::IDataWriter and @ref fep::IDataRegistry::IDataReader to exchange data in a FEP system please have a look at @ref page_fep_data_registry and the @ref demo_timing_30.

@ref fep::IDataRegistry::IDataWriter objects are used to register output signals at the @ref page_fep_data_registry. Signals will be sent when the @ref fep::Job::executeDataOut method of a @ref fep::Job is called. Data may be received using @ref fep::IDataRegistry::IDataReader instances which register input signals at the @ref page_fep_data_registry and will receive data samples during the @ref fep::Job::executeDataIn method of a @ref fep::Job.

While the @ref fep::IDataRegistry::IDataWriter and @ref fep::IDataRegistry::IDataReader objects provide an easy way to send and receive data, some use cases require further configuration effort to avoid data loss and to ensure performance optimization.

The following sections describe use cases which may require additional configuration. 

\section transmission_multiple_participants Transmission of data between multiple participants

Currently, FEP SDK does not guarantee in which order participants and corresponding @ref fep::Job are executed. This may lead to data loss if a participant receives data samples before the data of the current simulation step was processed.

A @ref fep::IDataRegistry::IDataReader uses an internal item queue to hold data samples until they are being used. If data samples are being received while the item queue is full, because some samples were not processed yet, the old data samples are dropped and therefore never processed within the simulation.

To avoid data loss, a @ref fep::IDataRegistry::IDataReader and the internal item queue may be configured using the @ref fep::DataJob::addDataIn method. The default size of an internal item queue is 1 which means the queue contains a single data sample. Increasing the data item queue allows the @ref fep::IDataRegistry::IDataReader to hold multiple data samples until they are relevant for the current simulation step.

\section transmission_multiple_samples Transmission of multiple data samples during a single simulation step

A @ref fep::IDataRegistry::IDataWriter uses an internal item queue to hold data samples until the @ref fep::Job::executeDataOut method of the corresponding @ref fep::Job flushes all @ref fep::IDataRegistry::IDataWriter. If a fep::IDataRegistry::IDataWriter is used to send an amount of data samples which is higher than the size of the corresponding item queue, data samples may be dropped from the queue without being sent.

If a @ref fep::IDataRegistry::IDataReader receives more data samples during a single simulation step than its item queue is able to hold, data samples are dropped without being processed.

To avoid loss of data when sending and receiving data samples, the item queue size of corresponding @ref fep::IDataRegistry::IDataWriter and @ref fep::IDataRegistry::IDataReader objects may be configured by using the corresponding overload of @ref fep::DataJob::addDataOut and @ref fep::DataJob::addDataIn methods.

Alternatively, @ref fep::DataJob::addDynamicDataOut may be used to create a @ref fep::IDataRegistry::IDataWriter which uses an item queue with dynamic size and theoretically infinite capacity. Using this dynamic item queue may result in problems due to memory shortage and a decrease in performance compared to item queues with fixed size.

\section transmission_multiple_cycle_times Transmission between jobs with divergent cycle times

If a FEP system consists of multiple participants which have divergent cycle times and therefore are triggered at different timestamps of the simulation, incorrect @ref fep::IDataRegistry::IDataReader configuration may lead to data loss.

This may occur if a participant having a short cycle time sends data samples and another participant having a longer cycle time (compared to the sending participant) receives those data samples using a @ref fep::IDataRegistry::IDataReader and a item queue which is not configured to hold enough data samples.

To avoid data loss, configure the @ref fep::IDataRegistry::IDataReader to have a data item queue which provides the required capacity.