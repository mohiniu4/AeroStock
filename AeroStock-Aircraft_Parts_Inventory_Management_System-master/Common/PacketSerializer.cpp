#include "pch.h"
#include "PacketSerializer.h"

#include <cstddef>

#include "Checksum.h"
#include "Enums.h"
#include "ProtocolConstants.h"

namespace
{
    [[nodiscard]] constexpr std::size_t ToSizeType(const std::uint32_t value) noexcept
    {
        // Convert a protocol length field into a container-friendly size type.
        return static_cast<std::size_t>(value);
    }
}

namespace AeroStock::Common
{
    PacketSerializer::ByteBuffer PacketSerializer::Serialize(const Packet& packet)
    {
        // Refuse to serialize invalid packets.
        if (!packet.IsValid())
        {
            return {};
        }

        // Copy the header and clear the checksum so we can calculate it.
        PacketHeader checksumHeader = packet.GetHeader();
        checksumHeader.checksum = 0U;

        ByteBuffer checksumBuffer;
        checksumBuffer.reserve(packet.GetTotalPacketSizeBytes());

        // Build the temporary byte stream used for checksum calculation.
        WriteUInt8(checksumBuffer, checksumHeader.version);
        WriteUInt32(checksumBuffer, checksumHeader.packetId);
        WriteUInt8(checksumBuffer, static_cast<std::uint8_t>(checksumHeader.commandType));
        WriteUInt8(checksumBuffer, static_cast<std::uint8_t>(checksumHeader.status));
        WriteUInt32(checksumBuffer, checksumHeader.payloadLength);
        WriteUInt32(checksumBuffer, checksumHeader.checksum);
        WriteBytes(checksumBuffer, packet.GetPayload());

        const std::uint32_t calculatedChecksum = Checksum::Calculate(checksumBuffer);

        const PacketHeader& header = packet.GetHeader();

        ByteBuffer finalBuffer;
        finalBuffer.reserve(packet.GetTotalPacketSizeBytes());

        // Build the final serialized packet using the real checksum.
        WriteUInt8(finalBuffer, header.version);
        WriteUInt32(finalBuffer, header.packetId);
        WriteUInt8(finalBuffer, static_cast<std::uint8_t>(header.commandType));
        WriteUInt8(finalBuffer, static_cast<std::uint8_t>(header.status));
        WriteUInt32(finalBuffer, header.payloadLength);
        WriteUInt32(finalBuffer, calculatedChecksum);
        WriteBytes(finalBuffer, packet.GetPayload());

        return finalBuffer;
    }

    bool PacketSerializer::Deserialize(const ByteBuffer& buffer, Packet& outputPacket)
    {
        PacketHeader header{};
        if (!DeserializeHeader(buffer, header))
        {
            return false;
        }

        // Reject buffers whose remaining bytes do not match the header payload length.
        if (!HasConsistentPayloadSize(buffer, header))
        {
            return false;
        }

        std::size_t offset = ProtocolConstants::FixedHeaderSizeBytes;
        Packet::PayloadBuffer payload{};

        // Read the payload bytes after the header.
        if (!ReadBytes(buffer, offset, header.payloadLength, payload))
        {
            return false;
        }

        // Recalculate the checksum by zeroing out the checksum field first.
        ByteBuffer checksumBuffer = buffer;
        checksumBuffer[ProtocolConstants::ChecksumOffsetBytes + 0U] = 0U;
        checksumBuffer[ProtocolConstants::ChecksumOffsetBytes + 1U] = 0U;
        checksumBuffer[ProtocolConstants::ChecksumOffsetBytes + 2U] = 0U;
        checksumBuffer[ProtocolConstants::ChecksumOffsetBytes + 3U] = 0U;

        const std::uint32_t calculatedChecksum = Checksum::Calculate(checksumBuffer);
        if (calculatedChecksum != header.checksum)
        {
            return false;
        }

        // Rebuild the packet object from the validated header and payload.
        Packet packet(header.packetId, header.commandType, header.status, std::move(payload));
        packet.SetVersion(header.version);
        packet.SetChecksum(header.checksum);

        if (!packet.IsValid())
        {
            return false;
        }

        outputPacket = std::move(packet);
        return true;
    }

    bool PacketSerializer::CanDeserializeHeader(const ByteBuffer& buffer) noexcept
    {
        return buffer.size() >= GetMinimumPacketSizeBytes();
    }

    bool PacketSerializer::DeserializeHeader(const ByteBuffer& buffer, PacketHeader& outputHeader)
    {
        // A header can only be read if the buffer is large enough.
        if (!CanDeserializeHeader(buffer))
        {
            return false;
        }

        std::size_t offset = 0U;
        return ReadHeader(buffer, offset, outputHeader);
    }

    void PacketSerializer::WriteUInt8(ByteBuffer& buffer, const std::uint8_t value)
    {
        // Append one byte to the buffer.
        buffer.push_back(value);
    }

    void PacketSerializer::WriteUInt32(ByteBuffer& buffer, const std::uint32_t value)
    {
        // Write a 32-bit value in big-endian order.
        buffer.push_back(static_cast<std::uint8_t>((value >> 24U) & 0xFFU));
        buffer.push_back(static_cast<std::uint8_t>((value >> 16U) & 0xFFU));
        buffer.push_back(static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
        buffer.push_back(static_cast<std::uint8_t>(value & 0xFFU));
    }

    void PacketSerializer::WriteBytes(ByteBuffer& buffer, const Packet::PayloadBuffer& payload)
    {
        // Nothing to do for an empty payload.
        if (payload.empty())
        {
            return;
        }

        // Append all payload bytes to the buffer.
        static_cast<void>(buffer.insert(buffer.end(), payload.begin(), payload.end()));
    }

    bool PacketSerializer::ReadUInt8(
        const ByteBuffer& buffer,
        std::size_t& offset,
        std::uint8_t& outputValue) noexcept
    {
        // Reject reads that go beyond the buffer.
        if (offset >= buffer.size())
        {
            return false;
        }

        outputValue = buffer[offset];
        ++offset;
        return true;
    }

    bool PacketSerializer::ReadUInt32(
        const ByteBuffer& buffer,
        std::size_t& offset,
        std::uint32_t& outputValue) noexcept
    {
        constexpr std::size_t bytesRequired = sizeof(std::uint32_t);

        // Reject reads that would go beyond the buffer.
        if ((offset + bytesRequired) > buffer.size())
        {
            return false;
        }

        // Reconstruct the 32-bit value from big-endian bytes.
        outputValue =
            (static_cast<std::uint32_t>(buffer[offset]) << 24U) |
            (static_cast<std::uint32_t>(buffer[offset + 1U]) << 16U) |
            (static_cast<std::uint32_t>(buffer[offset + 2U]) << 8U) |
            static_cast<std::uint32_t>(buffer[offset + 3U]);

        offset += bytesRequired;
        return true;
    }

    bool PacketSerializer::ReadBytes(
        const ByteBuffer& buffer,
        std::size_t& offset,
        const std::uint32_t length,
        Packet::PayloadBuffer& outputPayload)
    {
        const std::size_t payloadLengthBytes{ ToSizeType(length) };

        // Reject reads that would go past the available bytes.
        if ((offset > buffer.size()) || (payloadLengthBytes > (buffer.size() - offset)))
        {
            return false;
        }

        // Copy the payload bytes into the output payload buffer.
        outputPayload.assign(
            buffer.begin() + static_cast<std::ptrdiff_t>(offset),
            buffer.begin() + static_cast<std::ptrdiff_t>(offset + payloadLengthBytes));

        offset += payloadLengthBytes;
        return true;
    }

    bool PacketSerializer::ReadHeader(
        const ByteBuffer& buffer,
        std::size_t& offset,
        PacketHeader& outputHeader)
    {
        std::uint8_t rawVersion = 0U;
        std::uint32_t rawPacketId = 0U;
        std::uint8_t rawCommandType = 0U;
        std::uint8_t rawStatus = 0U;
        std::uint32_t rawPayloadLength = 0U;
        std::uint32_t rawChecksum = 0U;

        // Read each header field in protocol order.
        if (!ReadUInt8(buffer, offset, rawVersion))
        {
            return false;
        }

        if (!ReadUInt32(buffer, offset, rawPacketId))
        {
            return false;
        }

        if (!ReadUInt8(buffer, offset, rawCommandType))
        {
            return false;
        }

        if (!ReadUInt8(buffer, offset, rawStatus))
        {
            return false;
        }

        if (!ReadUInt32(buffer, offset, rawPayloadLength))
        {
            return false;
        }

        if (!ReadUInt32(buffer, offset, rawChecksum))
        {
            return false;
        }

        // Store the parsed values in the output header.
        outputHeader.version = rawVersion;
        outputHeader.packetId = rawPacketId;
        outputHeader.commandType = static_cast<CommandType>(rawCommandType);
        outputHeader.status = static_cast<StatusCode>(rawStatus);
        outputHeader.payloadLength = rawPayloadLength;
        outputHeader.checksum = rawChecksum;

        // Header is only accepted if it passes validation.
        return outputHeader.IsValid();
    }

    bool PacketSerializer::HasConsistentPayloadSize(
        const ByteBuffer& buffer,
        const PacketHeader& header) noexcept
    {
        // Buffer must at least contain a full header.
        if (buffer.size() < ProtocolConstants::FixedHeaderSizeBytes)
        {
            return false;
        }

        const std::size_t remainingBytes = buffer.size() - ProtocolConstants::FixedHeaderSizeBytes;

        // Reject packets whose payload exceeds the configured maximum.
        if (remainingBytes > static_cast<std::size_t>(ProtocolConstants::MaxPayloadSizeBytes))
        {
            return false;
        }

        const std::uint32_t remainingBytes32 = static_cast<std::uint32_t>(remainingBytes);

        // Remaining bytes must match the payload length declared in the header.
        return remainingBytes32 == header.payloadLength;
    }
}