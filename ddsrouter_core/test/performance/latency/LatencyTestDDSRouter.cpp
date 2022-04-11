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
 * @file LatencyPublisher.cpp
 *
 */

#include "LatencyTestDDSRouter.hpp"
#include "LatencyTestTypes.hpp"

#include <asio.hpp>

#include <fastdds/dds/log/Colors.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>

#include <ddsrouter_core/configuration/participant/SimpleParticipantConfiguration.hpp>
#include <ddsrouter_core/core/DDSRouter.hpp>
#include <ddsrouter_core/types/dds/DomainId.hpp>
#include <ddsrouter_core/types/topic/FilterTopic.hpp>
#include <ddsrouter_core/types/topic/WildcardTopic.hpp>
#include <ddsrouter_utils/Log.hpp>


using namespace eprosima::fastdds::dds;
using namespace eprosima::ddsrouter::core;
using namespace eprosima::ddsrouter::core::types;


LatencyTestDDSRouter::LatencyTestDDSRouter(
        bool reliable,
        uint32_t seed,
        bool hostname,
        std::tuple<DomainId, DomainId> domains)
    : reliable_(reliable)
    , seed_(seed)
    , hostname_(hostname)
    , domains_(domains)
    , initialized_(false)
    , latency_command_type_(new TestCommandDataType())
    , command_reader_listener_(this)

{
}

bool LatencyTestDDSRouter::init()
{
    initialized_ = true;

    create_topics_();

    if (std::get<0>(domains_) == std::get<1>(domains_))
    {
        logError(LatencyTest_DDSRouter, "Equal DDS Router Participants domains.");
        initialized_ = false;
    }
    else if ((!std::get<0>(domains_).is_valid()) || (!std::get<1>(domains_).is_valid()))
    {
        logError(LatencyTest_DDSRouter, "Invalid DDS Router Participants domains.");
        initialized_ = false;
    }

    std::set<std::shared_ptr<FilterTopic>> allowlist;
    std::set<std::shared_ptr<FilterTopic>> blocklist;
    std::set<std::shared_ptr<RealTopic>> builtin_topics;

    for(std::string topic : command_topics_)
    {
        allowlist.insert(std::make_shared<WildcardTopic>(topic, TestCommandDataType::type_name()));
        builtin_topics.insert(std::make_shared<RealTopic>(topic, TestCommandDataType::type_name(), false, true));
    }

    for(std::string topic : data_topics_)
    {
        allowlist.insert(std::make_shared<WildcardTopic>(topic, LatencyDataType::type_name()));
        builtin_topics.insert(std::make_shared<RealTopic>(topic, LatencyDataType::type_name(), false, reliable_));
    }

    // Two simple participants
    std::set<std::shared_ptr<configuration::ParticipantConfiguration>> participants_configurations(
                    {
                        std::make_shared<configuration::SimpleParticipantConfiguration>(
                            ParticipantId("participant_0"),
                            ParticipantKind(ParticipantKind::SIMPLE_RTPS),
                            DomainId(std::get<0>(domains_))
                            ),
                        std::make_shared<configuration::SimpleParticipantConfiguration>(
                            ParticipantId("participant_1"),
                            ParticipantKind(ParticipantKind::SIMPLE_RTPS),
                            DomainId(std::get<1>(domains_))
                            )
                    }
        );

    configuration::DDSRouterConfiguration ddsrouter_configuration(
        allowlist,
        blocklist,
        builtin_topics,
        participants_configurations);

    ddsrouter_ = std::make_unique<DDSRouter>(ddsrouter_configuration);

    /* Create Command DataReader*/
    DomainParticipantQos pqos;

    // Default participant name
    pqos.name("latency_test_ddsrouter");

    // Create the participant
    participant_ = DomainParticipantFactory::get_instance()->create_participant(
            std::get<0>(domains_).domain_id(), pqos);
    if (participant_ == nullptr)
    {
        return false;
    }

    // Register the command type
    if (ReturnCode_t::RETCODE_OK != latency_command_type_.register_type(participant_))
    {
        logError(LatencyTest_Subscriber, "ERROR registering the COMMAND type");
        return false;
    }

    // Create Command Reader
    DataReaderQos cr_qos;
    cr_qos.history().kind = HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
    cr_qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    cr_qos.durability().kind = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;

    command_reader_ = subscriber_->create_datareader(
        latency_command_sub_topic_,
        cr_qos,
        &command_reader_listener_);

    if (command_reader_ == nullptr)
    {
        return false;
    }

    return initialized_;
}

void LatencyTestDDSRouter::run()
{
    if (!initialized_)
    {
        logError(LatencyTest_DDSRouter, "DDS Router not initialized. Exiting ...");
        return;
    }

    ddsrouter_->start();

    // Wait for the STOP or STOP_ERROR commands
    wait_for_command(
        [this]()
        {
            return command_msg_count_ != 0;
        });

    // Wait 5 seconds before closing the DDS Router to let the subscribers stop gracefully
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

void LatencyTestDDSRouter::create_topics_()
{
    std::ostringstream topic_name;

    // Create pub2sub command topic name
    topic_name.str("");
    topic_name.clear();
    topic_name << "LatencyTest_Command_";

    if (hostname_)
    {
        topic_name << asio::ip::host_name() << "_";
    }
    topic_name << seed_ << "_PUB2SUB";
    command_topics_.push_back(topic_name.str());

    // Crete the topic instance for this topic as is the only topic with an actual DataReader.
    latency_command_sub_topic_ = participant_->create_topic(
            topic_name.str(),
            TestCommandDataType::type_name(),
            TOPIC_QOS_DEFAULT);


    // Create sub2pub command topic name
    topic_name.str("");
    topic_name.clear();
    topic_name << "LatencyTest_Command_";

    if (hostname_)
    {
        topic_name << asio::ip::host_name() << "_";
    }
    topic_name << seed_ << "_SUB2PUB";
    command_topics_.push_back(topic_name.str());

    // Create pub2sub data topic name
    topic_name.str("");
    topic_name.clear();
    topic_name << "LatencyTest_";
    if (hostname_)
    {
        topic_name << asio::ip::host_name() << "_";
    }
    topic_name << seed_ << "_PUB2SUB";
    data_topics_.push_back(topic_name.str());

    // Create sub2pub data topic name
    topic_name.str("");
    topic_name.clear();
    topic_name << "LatencyTest_";

    if (hostname_)
    {
        topic_name << asio::ip::host_name() << "_";
    }
    topic_name << seed_ << "_SUB2PUB";
    data_topics_.push_back(topic_name.str());
}

void LatencyTestDDSRouter::CommandReaderListener::on_data_available(
        DataReader* reader)
{
    TestCommandType command;
    SampleInfo info;
    std::ostringstream log;
    bool notify = false;

    if (reader->take_next_sample(
                &command, &info) == ReturnCode_t::RETCODE_OK
            && info.valid_data)
    {
        std::unique_lock<std::mutex> lock(latency_ddsrouter_->mutex_);

        log << "RCOMMAND: " << command.command_;
        switch ( command.command_ )
        {
            case STOP:
                log << "Publisher has stopped the test";
                ++latency_ddsrouter_->command_msg_count_;
                notify = true;
                break;
            case STOP_ERROR:
                log << "Publisher has canceled the test";
                latency_ddsrouter_->test_status_ = -1;
                ++latency_ddsrouter_->command_msg_count_;
                notify = true;
                break;
            default:
                break;
        }

        lock.unlock();
        if (notify)
        {
            latency_ddsrouter_->command_msg_cv_.notify_one();
        }
    }
    else
    {
        log << "Problem reading command message";
    }

    logInfo(LatencyTest, log.str());
}
