#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "Packet.h"

namespace AeroStock::Common
{
    // Converts between Packet objects and raw byte buffers.
    class PacketSerializer final
    {
    public:
        using ByteBuffer = std::vector<std::uint8_t>;

        PacketSerializer() = delete;

        // Serializes a Packet into a byte buffer ready for transmission.
        [[nodiscard]] static ByteBuffer Serialize(const Packet& packet);

        // Deserializes a byte buffer into a Packet if the buffer is valid.
        [[nodiscard]] static bool Deserialize(
            const ByteBuffer& buffer,
            Packet& outputPacket);

        // Returns the smallest possible serialized packet size.
        [[nodiscard]] static constexpr std::size_t GetMinimumPacketSizeBytes() noexcept
        {
            return ProtocolConstants::FixedHeaderSizeBytes;
        }

        // Returns true if the buffer is large enough to contain a header.
        [[nodiscard]] static bool CanDeserializeHeader(const ByteBuffer& buffer) noexcept;

        // Reads and validates only the header portion of a buffer.
        [[nodiscard]] static bool DeserializeHeader(
            const ByteBuffer& buffer,
            PacketHeader& outputHeader);

    private:
        // Writes a single byte into the output buffer.
        static void WriteUInt8(ByteBuffer& buffer, std::uint8_t value);

        // Writes a 32-bit value into the output buffer in network byte order.
        static void WriteUInt32(ByteBuffer& buffer, std::uint32_t value);

        // Appends payload bytes to the output buffer.
        static void WriteBytes(ByteBuffer& buffer, const Packet::PayloadBuffer& payload);

        // Reads a single byte from the current buffer offset.
        [[nodiscard]] static bool ReadUInt8(
            const ByteBuffer& buffer,
            std::size_t& offset,
            std::uint8_t& outputValue) noexcept;

        // Reads a 32-bit value from the current buffer offset.
        [[nodiscard]] static bool ReadUInt32(
            const ByteBuffer& buffer,
            std::size_t& offset,
            std::uint32_t& outputValue) noexcept;

        // Reads a payload block from the current buffer offset.
        [[nodiscard]] static bool ReadBytes(
            const ByteBuffer& buffer,
            std::size_t& offset,
            std::uint32_t length,
            Packet::PayloadBuffer& outputPayload);

        // Reads all header fields from the current offset and validates them.
        [[nodiscard]] static bool ReadHeader(
            const ByteBuffer& buffer,
            std::size_t& offset,
            PacketHeader& outputHeader);

        // Returns true if the header payload length matches the remaining buffer size.
        [[nodiscard]] static bool HasConsistentPayloadSize(
            const ByteBuffer& buffer,
            const PacketHeader& header) noexcept;
    };
}