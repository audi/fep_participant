
# FEP SDK Participant Library Changelog {#fep_sdk_participant_change_log}
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0) and this project adheres to [Semantic Versioning](https://semver.org/lang/en).


Release Notes - FEP SDK Participant Library - Version 2.6.1

## [2.6.1] - 2020-03-30

### Bug
    * [FEPSDK-2042] - Synchronization of discrete clock occupies one cpu core on slave side
    * [FEPSDK-2211] - Participant crashes after restart
    * [FEPSDK-2232] - FireupStateMachine not possible after Finalize of State Machine
    * [FEPSDK-2243] - usage of different fep::System instances and usage of const methods in parallel leads to a crash 
    * [FEPSDK-2263] - Transmission Adapter not disabled in READY->ERROR transition
    * [FEPSDK-2271] - DataRegistryFEP2::unregisterDataReceiveListener segmentation fault

### Change
    * [FEPSDK-1314] - Prepare FEP SDK code to be distributed as OSS
    * [FEPSDK-2022] - Cleanup CMake and conan as preparation for the open source distribution

Release Notes - FEP SDK Participant Library - Version 2.6.0

## [2.6.0] - 2020-02-19

### Bug
    * [FEPSDK-1268] - Property FEP_CLOCKSERVICE_SLAVE_SYNC_CYCLE_TIME influences discrete time synchronisation
    * [FEPSDK-1308] - Accessing input data with wrong type (non-matching size) is not reported as an error
    * [FEPSDK-1944] - Segmentation fault when shutting down a ParticipantFEP2 which uses DataJobs and DataWriters/DataReaders
    * [FEPSDK-2058] - It is not possible to set timing property
    * [FEPSDK-2123] - Wrong RTI license text is being published
    * [FEPSDK-2018] - Typos and incorrect unit in the FEP SDK Timing 30 Example
    * [FEPSDK-2147] - Examples only compile with warnings and VS 2017 support does not work 

### Change
    * [FEPSDK-1130] - Porting to QNX
    * [FEPSDK-1251] - Change default queue size of DataWriter objects
    * [FEPSDK-1288] - Adapt FEP DataWriter/DataWriterFEP2 to be able to resize the underlying DataItemQueue
    * [FEPSDK-1353] - [TransmissionAdapter] Make ZeroMQ (aka. Zyre) optional
    * [FEPSDK-1356] - Job max runtime duration check should not check if max_run_realtime is set to 0
    * [FEPSDK-1358] - make documentation of fep timing more precise
    * [FEPSDK-1360] - Compatibility of FEP SDK version <= 2.2 and FEP SDK version > 2.2 regarding sending/receiving of samples

Release Notes - FEP SDK Participant Library - Version 2.5.1

## [2.5.1] - 2019-10-18

### Bug
    * [FEPSDK-1377] - [fep launcher] In configureVUProvider the property ScenarioPath is set with the wrong path
    * [FEPSDK-1381] - Memory Leaks when accessing samples using LockData/UnlockData

### Change
    * [FEPSDK-1087] - [PropertyTree] Add To System Lib - Rework public interface to IRPCPropertyTree 
    * [FEPSDK-1324] - Create Participant Library (Repo + Package + Jobs) and rename it as participant library with an own delivery
    * [FEPSDK-1332] - Add new FEP SDK Package which collects the others: this package is delivered as 
                      fep_sdk_participant/2.5.1@aev25/stable => fep_sdk/2.5.1@aev25/stable will collect all FEPSDK Packages. Have a look on 
