/**
* 
*
* @file

   @copyright
   @verbatim
   Copyright @ 2019 Audi AG. All rights reserved.
   
       This Source Code Form is subject to the terms of the Mozilla
       Public License, v. 2.0. If a copy of the MPL was not distributed
       with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
   
   If it is not possible or desirable to put the notice in a particular file, then
   You may include the notice in a location (such as a LICENSE file in a
   relevant directory) where a recipient would be likely to look for such a notice.
   
   You may add additional accurate notices of copyright ownership.
   @endverbatim
*
*/

#ifndef __FEP_DATAJOB_H
#define __FEP_DATAJOB_H

#include <list>
#include <string>

#include "fep3/components/scheduler/jobs/job.h"
#include "fep3/components/base/component_intf.h"
#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/components/data_registry/data_reader.h"
#include "fep3/components/data_registry/data_writer.h"

namespace fep
{
    /**
     * A DataJob will automatically register on initialization time the data to the fep::IDataRegistry
     * It will also set up the default timing behaviour of its process method
     * 
     */
    class FEP_PARTICIPANT_EXPORT DataJob : public Job
    {
        public: 
            DataJob(std::string name, timestamp_t cycle_time);
            DataJob(std::string name, JobConfiguration job_config);
            DataJob(std::string name, timestamp_t cycle_time, Job::Func fc);
            DataJob(std::string name, JobConfiguration job_config, Job::Func fc);

            /**
            * Creates a @ref DataReader with a default queue capacity of 1
            * If you receive more than one sample before reading, these samples will be lost.
            * @param name name of signal
            * @param type type of signal
            * @return created DataReader
            */
            DataReader* addDataIn(const char* name, const IStreamType& type);

            /**
            * Creates a @ref DataReader with a specific queue capacity
            * If you receive more samples than specified by @param queue_size before reading, these samples will be lost.
            * @param name name of signal
            * @param type type of signal
            * @param queue_size capacity of the internal DataReader item queue
            * @return created DataReader
            */
            DataReader* addDataIn(const char* name, const IStreamType& type, size_t queue_size);

            /**
            * Creates a @ref DataWriter with a default queue capacity of 1
            * If you queue more than one sample before flushing these samples will be lost.
            * @param name name of signal 
            * @param type type of signal
            * @return created DataWriter
            */
            DataWriter* addDataOut(const char* name, const IStreamType& type);            

            /**
            * Creates a @ref DataWriter with a fixed maximum queue capacity. 
            * If you queue more than @param queue_size samples before flushing these samples will be lost.
            * @param name name of signal
            * @param type type of signal
            * @param queue_size maximum capacity of the queue
            * @throw throws if queue_size parameter is <= 0
            * @return created DataWriter
            */
            DataWriter* addDataOut(const char* name, const IStreamType& type, size_t queue_size);

            /**
            * Creates a @ref DataWriter with infinite queue capacity.
            * Pushing a sample into the queue extends the queue.
            * Every sample that is queued will be written on flush.
            * @param name name of signal
            * @param type type of signal            
            * @return created DataWriter
            * @remark pushing big numbers of samples into the queue might lead to out-of-memory situations
            */
            DataWriter* addDynamicDataOut(const char* name, const IStreamType& type);

            /**
            * Resize the DataReaderBacklog of a DataReader.
            * @param name name of the DataReader to be resized.
            * @param queue_size size of the DataReaderBacklog after resizing.
            * @return fep::Result 
            * @return ERR_NOT_FOUND found no DataReader with the given @param name.
            */
            fep::Result reconfigureDataIn(const char* name, size_t queue_size);

            /**
             * The process method to override.
             * implement your functionality here.
             * you do not need to lock something in here!
             * @param time_of_execution current time of execution from the ClockService
             *                          this is the beginning time of the execution in simulation time
             * @return fep::Result If you return en error here the scheduler might stop execution immediatelly!
             */
            virtual fep::Result process(timestamp_t time_of_execution);

        public: //Job
            /**
             * The reset method to override.
             * implement your reset functionality here.
             * This method is called each time before the \p process method is called
             * @return fep::Result If you return en error here the scheduler might stop execution immediatelly!
             */
            fep::Result reset() override;
            fep::Result addToComponents(const fep::IComponents& components) override;
            fep::Result removeFromComponents(const fep::IComponents& components) override;

        private:
            fep::Result executeDataIn(timestamp_t time_of_execution) override;
            fep::Result executeDataOut(timestamp_t time_of_execution) override;
            fep::Result addToDataRegistry(IDataRegistry& data_registry);
            fep::Result removeFromDataRegistry();

        private:
            std::list<DataReader> _readers;
            std::list<DataWriter> _writers;
    };
}

#endif // __FEP_DATAJOB_H
