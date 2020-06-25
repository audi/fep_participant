#
# Copyright @ 2019 Audi AG. All rights reserved.
# 
#     This Source Code Form is subject to the terms of the Mozilla
#     Public License, v. 2.0. If a copy of the MPL was not distributed
#     with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
# 
# If it is not possible or desirable to put the notice in a particular file, then
# You may include the notice in a location (such as a LICENSE file in a
# relevant directory) where a recipient would be likely to look for such a notice.
# 
# You may add additional accurate notices of copyright ownership.
#
set(SOURCES
    fastrtps_ll_wrap.h
    Config.h
    ReliabilityMode.h
    Helper.h                Helper.cpp
    Sample.h
    Queue.h
    Topic.h                    Topic.cpp
    Participant.h            Participant.cpp
    Publisher.h                Publisher.cpp
    Subscriber.h            Subscriber.cpp
    )


# Variant 2: Queue/Unicast -> fastrtps_ll_wrap_queue
add_library(fastrtps_ll_wrap_queue STATIC
    ${SOURCES}
)
target_link_libraries(fastrtps_ll_wrap_queue
    ${FASTRTPS_LIBS}
)
set_target_properties(fastrtps_ll_wrap_queue
    PROPERTIES
        INCLUDE_DIRECTORIES "${PARENT_SOURCE_DIRECTORY};${FASTRTPS_INCLUDE_DIRS}"
        INTERFACE_INCLUDE_DIRECTORIES "${PARENT_SOURCE_DIRECTORY};${FASTRTPS_INCLUDE_DIRS}"
        COMPILE_DEFINITIONS "DDS_VARIANT_IS_FASTRTPS_LL=1"
        INTERFACE_COMPILE_DEFINITIONS "DDS_VARIANT_IS_FASTRTPS_LL=1"
   )
set_property(TARGET fastrtps_ll_wrap_queue PROPERTY FOLDER test/stimuli/only_dds_sample)


