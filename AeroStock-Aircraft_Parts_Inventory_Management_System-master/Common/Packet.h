#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#include "PacketHeader.h"

namespace AeroStock::Common
{
    // Represents a full protocol packet made up of a fixed header and a payload.
    class Packet final
    {
    public:
        using PayloadBuffer = std::vector<std::uint8_t>;

        Packet() = default;

        Packet(
            std::uint32_t packetId,
            CommandType commandType,
            StatusCode status,
            const PayloadBuffer& payload = {});

        Packet(
            std::uint32_t packetId,
            CommandType commandType,
            StatusCode status,
            PayloadBuffer&& payload) noexcept;

        // Returns the full packet header.
        [[nodiscard]] const PacketHeader& GetHeader() const noexcept;

        // Returns individual header fields.
        [[nodiscard]] std::uint8_t GetVersion() const noexcept;
        [[nodiscard]] std::uint32_t GetPacketId() const noexcept;
        [[nodiscard]] CommandType GetCommandType() const noexcept;
        [[nodiscard]] StatusCode GetStatus() const noexcept;
        [[nodiscard]] std::uint32_t GetPayloadLength() const noexcept;
        [[nodiscard]] std::uint32_t GetChecksum() const noexcept;

        // Updates header fields.
        void SetVersion(std::uint8_t version) noexcept;
        void SetPacketId(std::uint32_t packetId) noexcept;
        void SetCommandType(CommandType commandType) noexcept;
        void SetStatus(StatusCode status) noexcept;

        // Sets the stored checksum value.
        void SetChecksum(std::uint32_t checksum) noexcept;

        // Clears the stored checksum so it can be regenerated later.
        void ClearChecksum() noexcept;

        // Returns the packet payload buffer.
        [[nodiscard]] const PayloadBuffer& GetPayload() const noexcept;

        // Replaces the payload and updates packet metadata.
        void SetPayload(const PayloadBuffer& payload);
        void SetPayload(PayloadBuffer&& payload) noexcept;

        // Replaces the payload using text data.
        void SetPayloadFromString(std::string_view text);

        // Appends raw bytes to the payload if the final size is still allowed.
        void AppendPayloadData(const std::uint8_t* data, std::size_t length);

        // Appends another payload buffer if the final size is still allowed.
        void AppendPayloadData(const PayloadBuffer& data);

        // Removes all payload data.
        void ClearPayload() noexcept;

        // Returns true if the packet currently contains payload data.
        [[nodiscard]] bool HasPayload() const noexcept;

        // Updates the stored payload length field to match the payload buffer.
        void RefreshPayloadLength() noexcept;

        // Returns true if the header is valid and the payload size matches the header.
        [[nodiscard]] bool IsValid() const noexcept;

        // Returns the total conceptual packet size in bytes.
        [[nodiscard]] std::size_t GetTotalPacketSizeBytes() const noexcept;

    private:
        PacketHeader header_{};
        PayloadBuffer payload_{};
    };
}