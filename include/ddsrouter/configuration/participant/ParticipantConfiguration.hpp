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
 * @file ParticipantConfiguration.hpp
 */

#ifndef _DDSROUTER_CONFIGURATION_PARTICIPANTCONFIGURATION_HPP_
#define _DDSROUTER_CONFIGURATION_PARTICIPANTCONFIGURATION_HPP_

#include <ddsrouter/configuration/BaseConfiguration.hpp>
#include <ddsrouter/types/participant/ParticipantId.hpp>
#include <ddsrouter/types/participant/ParticipantType.hpp>

namespace eprosima {
namespace ddsrouter{
namespace configuration {

/**
 * TODO
 */
class ParticipantConfiguration : public BaseConfiguration
{
public:

    /**
     * TODO
     */
    ParticipantConfiguration(
            const ParticipantId& id,
            const ParticipantType& type) noexcept;

    //! Participant Type associated with this configuration
    ParticipantType type() const noexcept;

    //! Participant Id associated with this configuration
    ParticipantId id() const noexcept;

    /**
     * @brief Equal comparator
     *
     * This comparator should check if the id is equal to the other Configuration and check the yaml equality.
     *
     * @todo: check equality yaml and not identity yaml.
     *
     * @param [in] other: ParticipantConfiguration to compare.
     * @return True if both configurations are the same, False otherwise.
     */
    bool operator ==(
            const ParticipantConfiguration& other) const noexcept;

    virtual bool is_valid() const noexcept override;

protected:

    //! Participant Id associated with this configuration
    const ParticipantId id_;

    //! Participant Type of the Participant that this configuration refers.
    const ParticipantType type_;
};

} /* namespace configuration */
} /* namespace ddsrouter */
} /* namespace eprosima */

#endif /* _DDSROUTER_CONFIGURATION_PARTICIPANTCONFIGURATION_HPP_ */