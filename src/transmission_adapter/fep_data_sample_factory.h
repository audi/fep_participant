/**
 * Declaration of the Class cDataSampleFactory.
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

#if !defined(EA_8E5FD5C0_0CB4_4fee_9BAA_74C312D301B3__INCLUDED_)
#define EA_8E5FD5C0_0CB4_4fee_9BAA_74C312D301B3__INCLUDED_

#include "fep_participant_export.h"
#include "fep_result_decl.h"

namespace fep
{
    class IPreparationDataSample;
    class ITransmissionDataSample;
    class IUserDataSample;

    /**
     * This class is the factory for any kind of data sample in FEP.
     * It is the only class knowing about cDataSample. This is no accident but by design.
     * Also this class explicitly is not a template class, since this would mean, that all classes
     * including cDataSampleFactory's header would also include cDataSample's header since the
     * template function would have to be not only declared but also defined here and therefore the
     * cDataSamples's header would have to be included here.
     */
    class FEP_PARTICIPANT_EXPORT cDataSampleFactory
    {

    private:
        /**
         * CTOR
         */
        cDataSampleFactory() = default;

        /**
         * DTOR
         */
        virtual ~cDataSampleFactory() = default;
    public:
        /**
         * The method \ref CreateSample creates a data sample. The caller has the ownership of the
         * created sample.
         * 
         * @param [out] ppoSample  a pointer to the created sample.
         * @returns  Standard result code.
         * @retval ERR_NOERROR  Everything went fine
         * @retval ERR_POINTER  Null pointer passed
         */
        static fep::Result CreateSample(IUserDataSample** ppoSample);

        /**
         * \overload
         */
        static fep::Result CreateSample(IPreparationDataSample** ppoSample);

        /**
         * \overload
         */
        static fep::Result CreateSample(ITransmissionDataSample** ppoSample);
    };
} // namespace fep
#endif // !defined(EA_8E5FD5C0_0CB4_4fee_9BAA_74C312D301B3__INCLUDED_)
