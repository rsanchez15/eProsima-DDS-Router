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
 * @file RealTopic.hpp
 */

#ifndef _DATABROKER_TYPES_TOPIC_REALTOPIC_HPP_
#define _DATABROKER_TYPES_TOPIC_REALTOPIC_HPP_

#include <databroker/types/topic/DatabrokerTopic.hpp>

namespace eprosima {
namespace databroker {

/**
 * TODO
 */
struct RealTopic : public DatabrokerTopic
{
    // Inherit parent constructors
    using DatabrokerTopic::DatabrokerTopic;

    static bool is_real_topic(
            const std::string& topic_name,
            const std::string& type_name);
};

} /* namespace databroker */
} /* namespace eprosima */

#endif /* _DATABROKER_TYPES_TOPIC_REALTOPIC_HPP_ */
