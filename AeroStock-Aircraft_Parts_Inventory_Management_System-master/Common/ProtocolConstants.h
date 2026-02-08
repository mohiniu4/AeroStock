#pragma once

#include <cstddef>
#include <cstdint>

namespace AeroStock::Common::ProtocolConstants
{
    // Current shared protocol version.
    inline constexpr std::uint8_t ProtocolVersion = 1U;

    // Serialized size of each header field.
    inline constexpr std::size_t VersionFieldSizeBytes = sizeof(std::uint8_t);
    inline constexpr std::size_t PacketIdFieldSizeBytes = sizeof(std::uint32_t);
    inline constexpr std::size_t CommandFieldSizeBytes = sizeof(std::uint8_t);
    inline constexpr std::size_t StatusFieldSizeBytes = sizeof(std::uint8_t);
    inline constexpr std::size_t PayloadLengthFieldSizeBytes = sizeof(std::uint32_t);
    inline constexpr std::size_t ChecksumFieldSizeBytes = sizeof(std::uint32_t);

    // Total serialized header size.
    inline constexpr std::size_t FixedHeaderSizeBytes =
        VersionFieldSizeBytes +
        PacketIdFieldSizeBytes +
        CommandFieldSizeBytes +
        StatusFieldSizeBytes +
        PayloadLengthFieldSizeBytes +
        ChecksumFieldSizeBytes;

    // Byte offsets for each field inside the serialized header.
    inline constexpr std::size_t VersionOffsetBytes = 0U;
    inline constexpr std::size_t PacketIdOffsetBytes = VersionOffsetBytes + VersionFieldSizeBytes;
    inline constexpr std::size_t CommandOffsetBytes = PacketIdOffsetBytes + PacketIdFieldSizeBytes;
    inline constexpr std::size_t StatusOffsetBytes = CommandOffsetBytes + CommandFieldSizeBytes;
    inline constexpr std::size_t PayloadLengthOffsetBytes = StatusOffsetBytes + StatusFieldSizeBytes;
    inline constexpr std::size_t ChecksumOffsetBytes = PayloadLengthOffsetBytes + PayloadLengthFieldSizeBytes;

    // Default TCP port used during development/testing.
    inline constexpr std::uint16_t DefaultPort = 54000U;

    // Allowed payload size range.
    inline constexpr std::uint32_t MaxPayloadSizeBytes = 5U * 1024U * 1024U;
    inline constexpr std::uint32_t MinPayloadSizeBytes = 0U;

    // Default filename used for transferred inventory data.
    inline constexpr const char* DefaultInventoryFileName = "inventory_catalog.dat";

    // Packet ID values used by the protocol.
    inline constexpr std::uint32_t InvalidPacketId = 0U;
    inline constexpr std::uint32_t FirstGeneratedPacketId = 1U;

    // Shared verification strings used during connection handshake.
    inline constexpr const char* VerificationRequestText = "AEROSTOCK_VERIFY";
    inline constexpr const char* VerificationSuccessText = "VERIFIED";
    inline constexpr const char* VerificationFailureText = "VERIFICATION_FAILED";

    // Generic text used for invalid request responses.
    inline constexpr const char* InvalidRequestText = "INVALID_REQUEST";
}