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
 * @file Time.cpp
 *
 */

#include <ddsrouter_utils/time/Timer.hpp>

namespace eprosima {
namespace ddsrouter {
namespace utils {

Timer::Timer() noexcept
    : start_time_(std::chrono::high_resolution_clock::now())
{
}

void Timer::reset() noexcept
{
    start_time_ = std::chrono::high_resolution_clock::now();
}

double Timer::elapsed() const noexcept
{
    return std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start_time_).count();
}

Duration_ms Timer::elapsed_ms() const noexcept
{
    return static_cast<Duration_ms>(elapsed());
}

} /* namespace utils */
} /* namespace ddsrouter */
} /* namespace eprosima */
