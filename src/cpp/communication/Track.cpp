// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Track.cpp
 *
 */

#include <ddsrouter/communication/Track.hpp>
#include <ddsrouter/exceptions/UnsupportedException.hpp>

namespace eprosima {
namespace ddsrouter {

// TODO: Add logs

Track::Track(
        const RealTopic& topic,
        std::shared_ptr<IReader> reader,
        std::map<ParticipantId, std::shared_ptr<IWriter>>&& writers,
        bool enable /* = false */)
    : topic_(topic)
    , reader_(reader)
    , writers_(writers)
    , enabled_(false)
    , exit_(false)
    , is_data_available_(false)
{
    // Set this track to on_data_available lambda call
    reader_->set_on_data_available_callback(std::bind(&Track::data_available, this));

    // Activate transmit thread even without being enabled
    transmit_thread_ = std::thread(&Track::transmit_thread_function_, this);

    if (enable)
    {
        // Activate Track
        this->enable();
    }
}

Track::~Track()
{
    // Take mutex to avoid calling enable / disable while in destruction
    std::lock_guard<std::recursive_mutex> lock(track_mutex_);

    // Disable reader
    disable();

    // Set exit status and call transmit thread to terminate
    exit_.store(true);
    available_data_condition_variable_.notify_all();
    transmit_thread_.join();
}

void Track::enable()
{
    std::lock_guard<std::recursive_mutex> lock(track_mutex_);

    if (!enabled_)
    {
        // Enable writers
        for (auto& writer_it : writers_)
        {
            writer_it.second->enable();
        }

        // Enable reader
        reader_->enable();

        enabled_ = true;
    }
}

void Track::disable()
{
    std::lock_guard<std::recursive_mutex> lock(track_mutex_);

    if (enabled_)
    {
        // Disabling Writer
        for (auto& writer_it : writers_)
        {
            writer_it.second->disable();
        }

        // Disabling Reader
        reader_->disable();

        enabled_ = false;
    }
}

void Track::no_more_data_available_()
{
    std::unique_lock<std::mutex> lock(available_data_mutex_);
    is_data_available_.store(false);
}

bool Track::should_transmit_()
{
    std::lock_guard<std::recursive_mutex> lock(track_mutex_);
    return !exit_ && enabled_;
}

void Track::data_available()
{
    {
        std::unique_lock<std::mutex> lock(available_data_mutex_);
        is_data_available_.store(true);
    }
    available_data_condition_variable_.notify_one();
}

void Track::transmit_thread_function_()
{
    while (!exit_)
    {
        std::unique_lock<std::mutex> lock(available_data_mutex_);
        available_data_condition_variable_.wait(
            lock,
            [this]
            {
                return this->is_data_available_ || this->exit_;
            });

        // Avoid start transmitting if it was awakened to terminate
        if (should_transmit_())
        {
            // If it was awakened because new data arrived, transmit it
            transmit_();
        }
    }
}

void Track::transmit_()
{
    while (should_transmit_())
    {
        // Get data received
        std::unique_ptr<DataReceived> data;
        ReturnCode ret = reader_->take(data);

        if (ret == ReturnCode::RETCODE_NO_DATA)
        {
            // There is no more data, so finish loop and wait for data
            no_more_data_available_();
            break;
        }
        else if (!ret)
        {
            // Error reading data
            // TODO: Add Log
            continue;
        }

        // Send data through writers
        for (auto& writer_it : writers_)
        {
            ret = writer_it.second->write(data);

            if (!ret)
            {
                // TODO: Add Log
                continue;
            }
        }
    }
}

} /* namespace ddsrouter */
} /* namespace eprosima */