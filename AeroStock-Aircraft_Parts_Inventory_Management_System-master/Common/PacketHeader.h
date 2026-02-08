#pragma once

#include <cstdint>

#include "Enums.h"
#include "ProtocolConstants.h"

namespace AeroStock::Common
{
    // Represents the fixed header that appears at the start of every packet.
    struct PacketHeader final
    {
        std::uint32_t packetId{ ProtocolConstants::InvalidPacketId };
        std::uint32_t payloadLength{ 0U };
        std::uint32_t checksum{ 0U };
        std::uint8_t version{ ProtocolConstants::ProtocolVersion };
        CommandType commandType{ CommandType::None };
        StatusCode status{ StatusCode::None };

        // Returns true if the header version matches the current protocol version.
        [[nodiscard]] constexpr bool HasValidVersion() const noexcept
        {
            return version == ProtocolConstants::ProtocolVersion;
        }

        // Returns true if the packet uses a valid non-reserved packet ID.
        [[nodiscard]] constexpr bool HasValidPacketId() const noexcept
        {
            return packetId != ProtocolConstants::InvalidPacketId;
        }

        // Returns true if the command type is recognized.
        [[nodiscard]] constexpr bool HasValidCommandType() const noexcept
        {
            return IsValidCommandType(commandType);
        }

        // Returns true if the status code is recognized.
        [[nodiscard]] constexpr bool HasValidStatusCode() const noexcept
        {
            return IsValidStatusCode(status);
        }

        // Returns true if the payload length does not exceed the protocol limit.
        [[nodiscard]] constexpr bool HasValidPayloadLength() const noexcept
        {
            return payloadLength <= ProtocolConstants::MaxPayloadSizeBytes;
        }

        // Returns true if all header fields pass basic validation.
        [[nodiscard]] constexpr bool IsValid() const noexcept
        {
            return HasValidVersion() &&
                HasValidPacketId() &&
                HasValidCommandType() &&
                HasValidStatusCode() &&
                HasValidPayloadLength();
        }
    };

    // Confirms the logical serialized header size remains aligned with the protocol constants.
    static_assert(
        ProtocolConstants::FixedHeaderSizeBytes == 15U,
        "ProtocolConstants::FixedHeaderSizeBytes must remain 15 bytes."
        );
}