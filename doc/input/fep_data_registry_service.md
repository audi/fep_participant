
# FEP Data Registry {#page_fep_data_registry}

The DataRegistry is responsible for registering data in access and data out access of the element implementation.
The container to write and read data is @ref fep::IDataRegistry::IDataSample. 
These data are classified via the @ref fep::IStreamType.

The main functionality of the data registry is to add the data ins and outs to the simulation bus at the correct initialization time.
The latest state your are able to add data ins and outs is @ref fep::FS_INITIALIZING.
Usually each data in and data out is named and pre-classified (see fep::IDataRegistry::registerDataIn, fep::IDataRegistry::registerDataOut).


## Data Samples

Data Samples (see also @ref fep::IDataRegistry::IDataSample) are plain raw memory containers with 3 important information:
\li time (in micro seconds)    - see @ref fep::IDataRegistry::IDataSample::getTime
\li size (in bytes)   - see @ref fep::IDataRegistry::IDataSample::getSize
\li counter (counting at writers time) - see @ref fep::IDataRegistry::IDataSample::getCounter

These samples are written to a writer queue using simulation bus memory @ref fep::IDataRegistry::IDataWriter (see @ref fep::IDataRegistry::getWriter). 
Samples are read from a queue through a instance of @ref fep::IDataRegistry::IDataReader (see @ref fep::IDataRegistry::getReader).

### Data Writer / Data Reader

Usually you do not need to handle the DataRegistry by yourself. 
Consider @ref fep::DataWriter and fep::DataReader convenience classes. 

See also @ref demo_timing_30 how to use them.

## Default Streamtypes

\par Current FEP 2 support 
Through to the FEP 2 limitations of the fep::ISignalRegistry the current data registry implementation of fep 2 does only supports 2 
different stream meta types:
\li @ref fep::meta_type_raw
\li @ref fep::meta_type_ddl

\par Further FEP 3 support 
The further data registry of FEP 3 will just forward following Streamtypes to the simulation bus: 
\li @ref fep::meta_type_ddl - as main use case
\li @ref fep::meta_type_raw - for unspecified data only the elements itself know the content
\li @ref fep::meta_type_plain - for plain c-type based samples
\li @ref fep::meta_type_plain_array - for plain c-type arrays
\li @ref fep::meta_type_string - for dynamic string data
\li @ref fep::meta_type_video - for video data
\li other @ref fep::StreamMetaType - user types which are described by the user but are unspecified for the simulation bus.

It depends on the simulation bus implementation used which meta type is supported.


#### Participant Internal Interface

The [IDataRegistry](classfep_1_1_i_data_registry.html) interface offers functionality to register data in and data out points.

#### Participant Service Interfaces 

* @ref fep::rpc::IRPCDataRegistry
* still experimental: [data_registry.json](../../include/fep3/components/data_registry/data_registry.json)