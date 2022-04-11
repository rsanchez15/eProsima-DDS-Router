// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file LatencyPublisher.hpp
 *
 */

#include "LatencyTestTypes.hpp"

#include <condition_variable>

#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/TypeDescriptor.h>

#include <ddsrouter_core/core/DDSRouter.hpp>
#include <ddsrouter_core/types/dds/DomainId.hpp>


using namespace eprosima::ddsrouter::core;
using namespace eprosima::ddsrouter::core::types;

class LatencyTestDDSRouter
{
public:

    LatencyTestDDSRouter(
        bool reliable,
        uint32_t seed,
        bool hostname,
        std::tuple<DomainId, DomainId> domains);

    bool init();

    void run();

private:

    void create_topics_();

    template<class Predicate>
    void wait_for_command(
            Predicate pred)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        command_msg_cv_.wait(lock, pred);
        command_msg_count_ = 0;
    }

    /* DDS Router */
    std::unique_ptr<DDSRouter> ddsrouter_;

    /* Entities */
    eprosima::fastdds::dds::DomainParticipant* participant_ = nullptr;
    eprosima::fastdds::dds::Subscriber* subscriber_ = nullptr;
    eprosima::fastdds::dds::DataReader* command_reader_ = nullptr;

    /* Topics */
    eprosima::fastdds::dds::Topic* latency_command_sub_topic_ = nullptr;
    eprosima::fastdds::dds::Topic* latency_command_pub_topic_ = nullptr;

    /* Types */
    LatencyType* latency_data_ = nullptr;
    eprosima::fastdds::dds::TypeSupport latency_command_type_;

    /* Test synchronization */
    std::mutex mutex_;
    std::condition_variable command_msg_cv_;
    int command_msg_count_ = 0;
    int test_status_ = 0;

    bool reliable_;
    uint32_t seed_;
    bool hostname_;
    std::tuple<DomainId, DomainId> domains_;
    bool initialized_;

    // Map to save the allowed topics
    std::vector<std::string> command_topics_;
    std::vector<std::string> data_topics_;

    class CommandReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
        LatencyTestDDSRouter* latency_ddsrouter_;
        int matched_;

    public:

        CommandReaderListener(
                LatencyTestDDSRouter* latency_ddsrouter)
            : latency_ddsrouter_(latency_ddsrouter)
            , matched_(0)
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        int get_matches() const
        {
            return matched_;
        }

        void reset()
        {
            matched_ = 0;
        }

    }
    command_reader_listener_;
};