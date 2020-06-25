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

#ifdef SEVERITY_ERROR
    #undef SEVERITY_ERROR
#endif 

namespace fep
{
    namespace logging
    {
        enum Category : std::uint32_t
        {
            ///any category for logging events that does not fit into any of the other
            CATEGORY_NONE = 0,
            ///Marks log event event that occured on system level.
            ///There might be log events on participant category before
            CATEGORY_SYSTEM = 1,
            ///Marks log event event that occured on participant level.
            /// there might be log events on component or element category before
            CATEGORY_PARTICIPANT = 2,
            ///Marks log event event that occured on component level
            ///these components are StateMachine, DataRegistry, SimulationBus ... see @ref fep_components
            CATEGORY_COMPONENT = 3,
            ///Marks log event event that occured on element functionality level
            ///the element category is for user level implemented within the element functionality
            ///one log may be: "ESP did only found 3 wheels, where is the fourth?" (Decide by your own which fep::logging::Severity this will be.)
            CATEGORY_ELEMENT = 4
        };

        /// Filter for logging events, the smaller the number the less events will be given
        enum Severity : std::uint32_t
        {
            ///any logging severity for logging events which are usually not valid
            SEVERITY_NONE = 0,
            ///marks very severe error log events that will presumably lead the application to abort
            SEVERITY_FATAL = 1,
            ///marks error events that might still allow the application to continue running
            SEVERITY_ERROR = 2,
            ///Marks log events that may potentially harmful situations
            SEVERITY_WARNING = 3,
            ///Marks log informational messages that highlight the progress of the application
            SEVERITY_INFO = 4,
            ///Marks log informational events that are most useful for debugging
            ///usually these messages will only appear within debug builds.
            SEVERITY_DEBUG = 5
        };
    }
}
