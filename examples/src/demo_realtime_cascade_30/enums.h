/**

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
 */
#pragma once
#include <cstdint>
/**
 * Timing client modes
 * Available timing client modes of the FEP Participant
 */
enum class ClientMode
{
    undefined_mode,
    local_realtime_mode,
    fixed_time_step_mode
};

/**
 * convert user input from command line to ClientMode
 */
inline ClientMode convertToClientMode(int mode_Input)
{
    switch (mode_Input)
    {
        case 0:
            return ClientMode::local_realtime_mode;
        case 1:
            return ClientMode::fixed_time_step_mode;
        default:
            return ClientMode::undefined_mode;
    }
}

/**
 * Timing master modes
 * Available timing master modes of the FEP Participant
 */
enum class MasterMode
{
    undefined_mode,
    local_realtime_mode,
    fixed_time_step_mode
};

/**
 * convert user input from command line to MasterMode
 */
inline MasterMode convertToMasterMode(int mode_input)
{
    switch (mode_input)
    {
        case 0:
            return MasterMode::local_realtime_mode;
        case 1:
            return MasterMode::fixed_time_step_mode;
        default:
            return MasterMode::undefined_mode;
    }
}

/**
 * Here exists two ways to get access to a signal, it is used to choose the right algorithm
 */
enum class SignalAccessVariant
{
    variant_1,
    variant_2
};

/**
 * the number based on predefined example_ddl_types
 */
enum class NumberOfElementsPerSignal : uint16_t
{
    one = 1,
    ten = 10,
    twenty = 20,
    hundred = 100,
    thousand = 1000
};
