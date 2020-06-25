/**
 * Implementation of the Class cDataSample.
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
#include <a_util/result/result_type.h>

#include "fep_errors.h"
#include "transmission_adapter/fep_data_sample.h"
#include "transmission_adapter/fep_data_sample_factory.h"

using namespace fep;

namespace fep
{
    template <typename T>
    fep::Result CreateSample(T** ppoSample)
    {
        if (!ppoSample) { return ERR_POINTER; }
        *ppoSample = new cDataSample();
        return ERR_NOERROR;
    }
}

fep::Result fep::cDataSampleFactory::CreateSample(fep::IUserDataSample** ppoSample)
{
    return fep::CreateSample(ppoSample);
}

fep::Result fep::cDataSampleFactory::CreateSample(fep::IPreparationDataSample** ppoSample)
{
    return fep::CreateSample(ppoSample);
}

fep::Result fep::cDataSampleFactory::CreateSample(
    fep::ITransmissionDataSample** ppoSample)
{
    return fep::CreateSample(ppoSample);
}
