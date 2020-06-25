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
set(SOURCES_COMMON
    rti_wrap.h
    Config.h
    ReliabilityMode.h
    Helper.h                Helper.cpp
    Sample.h
    Queue.h
    Topic.h                    Topic.cpp
    Participant.h            Participant.cpp
    Publisher.h                Publisher.cpp
    Subscriber.h
    )

set(SOURCES_WAITSET 
    SubscriberWithWaitset.h SubscriberWithWaitset.cpp
)
set(SOURCES_QUEUE
    SubscriberWithQueue.h SubscriberWithQueue.cpp
)


# Variant 1: Waitset -> rti_wrap_waitset
add_library(rti_wrap_waitset STATIC
    ${SOURCES_COMMON}
    ${SOURCES_WAITSET}
)
target_link_libraries(rti_wrap_waitset
    ${NDDS_LIBS}
)
set_target_properties(rti_wrap_waitset
    PROPERTIES
        INCLUDE_DIRECTORIES "${PARENT_SOURCE_DIRECTORY};${NDDS_INCLUDE_DIRS}"
        INTERFACE_INCLUDE_DIRECTORIES "${PARENT_SOURCE_DIRECTORY};${NDDS_INCLUDE_DIRS}"
        COMPILE_DEFINITIONS "RTI_DDS_USE_WAITSET=1;DDS_VARIANT_IS_RTIDDS=1"
        INTERFACE_COMPILE_DEFINITIONS "RTI_DDS_USE_WAITSET=1;DDS_VARIANT_IS_RTIDDS=1"
   )
set_property(TARGET rti_wrap_waitset PROPERTY FOLDER test/stimuli/only_dds_sample)

# Variant 2: Queuet -> rti_wrap_queue
add_library(rti_wrap_queue STATIC
    ${SOURCES_COMMON}
    ${SOURCES_QUEUE}
)
target_link_libraries(rti_wrap_queue
    ${NDDS_LIBS}
)
set_target_properties(rti_wrap_queue
    PROPERTIES
        INCLUDE_DIRECTORIES "${PARENT_SOURCE_DIRECTORY};${NDDS_INCLUDE_DIRS}"
        INTERFACE_INCLUDE_DIRECTORIES "${PARENT_SOURCE_DIRECTORY};${NDDS_INCLUDE_DIRS}"
        COMPILE_DEFINITIONS "RTI_DDS_USE_WAITSET=0;DDS_VARIANT_IS_RTIDDS=1"
        INTERFACE_COMPILE_DEFINITIONS "RTI_DDS_USE_WAITSET=0;DDS_VARIANT_IS_RTIDDS=1"
   )
set_property(TARGET rti_wrap_queue PROPERTY FOLDER test/stimuli/only_dds_sample)

