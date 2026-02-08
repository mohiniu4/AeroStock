#include "pch.h"
#include "Packet.h"

#include <utility>

#include "ProtocolConstants.h"

namespace AeroStock::Common
{
    Packet::Packet(
        const std::uint32_t packetId,
        const CommandType commandType,
        const StatusCode status,
        const PayloadBuffer& payload)
        : header_{},
        payload_{ payload }
    {
        // Initialize the header fields for this packet.
        header_.packetId = packetId;
        header_.commandType = commandType;
        header_.status = status;

        // Keep header metadata synchronized with the payload.
        RefreshPayloadLength();
        ClearChecksum();
    }

    Packet::Packet(
        const std::uint32_t packetId,
        const CommandType commandType,
        const StatusCode status,
        PayloadBuffer&& payload) noexcept
        : header_{},
        payload_{ std::move(payload) }
    {
        // Initialize the header fields for this packet.
        header_.packetId = packetId;
        header_.commandType = commandType;
        header_.status = status;

        // Keep header metadata synchronized with the payload.
        RefreshPayloadLength();
        ClearChecksum();
    }

    const PacketHeader& Packet::GetHeader() const noexcept
    {
        return header_;
    }

    std::uint8_t Packet::GetVersion() const noexcept
    {
        return header_.version;
    }

    std::uint32_t Packet::GetPacketId() const noexcept
    {
        return header_.packetId;
    }

    CommandType Packet::GetCommandType() const noexcept
    {
        return header_.commandType;
    }

    StatusCode Packet::GetStatus() const noexcept
    {
        return header_.status;
    }

    std::uint32_t Packet::GetPayloadLength() const noexcept
    {
        return header_.payloadLength;
    }

    std::uint32_t Packet::GetChecksum() const noexcept
    {
        return header_.checksum;
    }

    void Packet::SetVersion(const std::uint8_t version) noexcept
    {
        header_.version = version;
    }

    void Packet::SetPacketId(const std::uint32_t packetId) noexcept
    {
        header_.packetId = packetId;
    }

    void Packet::SetCommandType(const CommandType commandType) noexcept
    {
        header_.commandType = commandType;
    }

    void Packet::SetStatus(const StatusCode status) noexcept
    {
        header_.status = status;
    }

    void Packet::SetChecksum(const std::uint32_t checksum) noexcept
    {
        header_.checksum = checksum;
    }

    void Packet::ClearChecksum() noexcept
    {
        // Reset checksum when payload or header content changes.
        header_.checksum = 0U;
    }

    const Packet::PayloadBuffer& Packet::GetPayload() const noexcept
    {
        return payload_;
    }

    void Packet::SetPayload(const PayloadBuffer& payload)
    {
        // Replace payload with a copy.
        payload_ = payload;
        RefreshPayloadLength();
        ClearChecksum();
    }

    void Packet::SetPayload(PayloadBuffer&& payload) noexcept
    {
        // Replace payload using move semantics.
        payload_ = std::move(payload);
        RefreshPayloadLength();
        ClearChecksum();
    }

    void Packet::SetPayloadFromString(const std::string_view text)
    {
        // Store text directly as raw payload bytes.
        payload_.assign(text.begin(), text.end());
        RefreshPayloadLength();
        ClearChecksum();
    }

    void Packet::AppendPayloadData(const std::uint8_t* data, const std::size_t length)
    {
        // Ignore invalid append requests.
        if ((data == nullptr) || (length == 0U))
        {
            return;
        }

        // Refuse to exceed the configured payload size limit.
        const std::size_t newSize = payload_.size() + length;
        if (newSize > static_cast<std::size_t>(ProtocolConstants::MaxPayloadSizeBytes))
        {
            return;
        }

        // Append the new bytes to the payload.
        static_cast<void>(payload_.insert(payload_.end(), data, data + length));
        RefreshPayloadLength();
        ClearChecksum();
    }

    void Packet::AppendPayloadData(const PayloadBuffer& data)
    {
        // Ignore empty input buffers.
        if (data.empty())
        {
            return;
        }

        // Refuse to exceed the configured payload size limit.
        const std::size_t newSize = payload_.size() + data.size();
        if (newSize > static_cast<std::size_t>(ProtocolConstants::MaxPayloadSizeBytes))
        {
            return;
        }

        // Append the new buffer to the existing payload.
        static_cast<void>(payload_.insert(payload_.end(), data.begin(), data.end()));
        RefreshPayloadLength();
        ClearChecksum();
    }

    void Packet::ClearPayload() noexcept
    {
        // Remove all payload bytes.
        payload_.clear();
        RefreshPayloadLength();
        ClearChecksum();
    }

    bool Packet::HasPayload() const noexcept
    {
        return !payload_.empty();
    }

    void Packet::RefreshPayloadLength() noexcept
    {
        const std::size_t payloadSize = payload_.size();

        // Clamp the header length if the payload somehow exceeds the protocol limit.
        if (payloadSize > static_cast<std::size_t>(ProtocolConstants::MaxPayloadSizeBytes))
        {
            header_.payloadLength = ProtocolConstants::MaxPayloadSizeBytes;
            return;
        }

        // Store the current payload size in the header.
        header_.payloadLength = static_cast<std::uint32_t>(payloadSize);
    }

    bool Packet::IsValid() const noexcept
    {
        const std::size_t actualPayloadSize = payload_.size();

        // Reject packets whose payload buffer exceeds the configured limit.
        if (actualPayloadSize > static_cast<std::size_t>(ProtocolConstants::MaxPayloadSizeBytes))
        {
            return false;
        }

        const std::uint32_t actualPayloadSize32 = static_cast<std::uint32_t>(actualPayloadSize);

        // A packet is valid only if the header is valid and the stored payload length matches.
        return header_.IsValid() &&
            (header_.payloadLength == actualPayloadSize32);
    }

    std::size_t Packet::GetTotalPacketSizeBytes() const noexcept
    {
        // Total packet size is the header size plus the current payload size.
        return ProtocolConstants::FixedHeaderSizeBytes + payload_.size();
    }
}