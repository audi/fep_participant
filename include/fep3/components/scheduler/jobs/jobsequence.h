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

#ifndef __FEP_JOB_SEQUENCE_H
#define __FEP_JOB_SEQUENCE_H

#include <vector>
#include "fep3/components/scheduler/jobs/job.h"

namespace fep
{
    class FEP_PARTICIPANT_EXPORT JobSequence : public Job
    {
        public: 
            JobSequence(std::string name, timestamp_t cycle_time);
            JobSequence(std::string name, timestamp_t cycle_time, std::vector<std::pair<std::string, IJob*>> jobs_to_add);
        public:
            fep::Result addJob(std::string name, IJob& job_reference);
            void clearJobs();
            fep::Result execute(timestamp_t time_of_execution) override;
        private:
            std::vector<std::pair<std::string, IJob*>> _sequence;
    };
}

#endif // __FEP_JOB_SEQUENCE_H