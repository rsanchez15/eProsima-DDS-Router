# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

###########################################################################
# Create and link executable                                              #
###########################################################################
set(
    LATENCYTEST_SOURCE
    LatencyTestPublisher.cpp
    LatencyTestSubscriber.cpp
    LatencyTestTypes.cpp
    main_LatencyTest.cpp
)
add_executable(LatencyTest ${LATENCYTEST_SOURCE})

target_compile_definitions(LatencyTest PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    FASTDDS_ENFORCE_LOG_INFO
    HAVE_LOG_NO_INFO=0
)

target_include_directories(LatencyTest PRIVATE ${Asio_INCLUDE_DIR})

target_link_libraries(
    LatencyTest
    fastrtps
    fastcdr
    foonathan_memory
    fastdds::optionparser
    ddsrouter_utils
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
)
