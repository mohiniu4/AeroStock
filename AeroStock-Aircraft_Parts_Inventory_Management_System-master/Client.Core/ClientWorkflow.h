#pragma once

#include <cstdint>
#include <string>

#include "Packet.h"

namespace AeroStock::Client
{
    class ClientWorkflow final
    {
    public:
        ClientWorkflow() = delete;

        [[nodiscard]] static AeroStock::Common::Packet CreateConnectPacket(std::uint32_t packetId);

        [[nodiscard]] static AeroStock::Common::Packet CreateVerifyPacket(std::uint32_t packetId);

        [[nodiscard]] static AeroStock::Common::Packet CreateSearchByPartNumberPacket(
            std::uint32_t packetId,
            const std::string& partNumber);

        [[nodiscard]] static AeroStock::Common::Packet CreateSearchByPartNamePacket(
            std::uint32_t packetId,
            const std::string& partName);

        [[nodiscard]] static AeroStock::Common::Packet CreateGetPartDetailsPacket(
            std::uint32_t packetId,
            const std::string& partNumber);

        [[nodiscard]] static AeroStock::Common::Packet CreateUpdateStockPacket(
            std::uint32_t packetId,
            const std::string& partNumber,
            std::uint32_t newQuantity);

        [[nodiscard]] static AeroStock::Common::Packet CreateRequestFilePacket(std::uint32_t packetId);

        [[nodiscard]] static AeroStock::Common::Packet CreateDisconnectPacket(std::uint32_t packetId);

        [[nodiscard]] static bool IsSuccessfulResponse(const AeroStock::Common::Packet& responsePacket);

        [[nodiscard]] static bool IsErrorResponse(const AeroStock::Common::Packet& responsePacket);

        [[nodiscard]] static std::string GetPayloadText(const AeroStock::Common::Packet& packet);

        [[nodiscard]] static std::string FormatResponseForDisplay(
            const AeroStock::Common::Packet& packet);

    private:
        [[nodiscard]] static std::string FormatSingleRecordForDisplay(const std::string& recordText);
        [[nodiscard]] static std::string FormatMultiRecordResponseForDisplay(const std::string& payloadText);
    };
}