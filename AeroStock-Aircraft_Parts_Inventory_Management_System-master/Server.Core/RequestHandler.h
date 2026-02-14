#pragma once

#include <cstdint>
#include <string_view>

#include "Packet.h"
#include "Inventory.h"
#include "ServerStateMachine.h"

namespace AeroStock::Server
{
    // dispatches incoming packets to the appropriate handler based on command type
    class RequestHandler final
    {
    public:
        RequestHandler() = default;

        [[nodiscard]] AeroStock::Common::Packet HandleRequest(
            const AeroStock::Common::Packet& requestPacket,
            ServerStateMachine& stateMachine);

    private:
        [[nodiscard]] AeroStock::Common::Packet HandleConnectRequest(
            const AeroStock::Common::Packet& requestPacket,
            ServerStateMachine& stateMachine) const;

        [[nodiscard]] AeroStock::Common::Packet HandleVerifyRequest(
            const AeroStock::Common::Packet& requestPacket,
            ServerStateMachine& stateMachine) const;

        [[nodiscard]] AeroStock::Common::Packet HandleSearchByPartNumber(
            const AeroStock::Common::Packet& requestPacket,
            ServerStateMachine& stateMachine);

        [[nodiscard]] AeroStock::Common::Packet HandleSearchByPartName(
            const AeroStock::Common::Packet& requestPacket,
            ServerStateMachine& stateMachine);

        [[nodiscard]] AeroStock::Common::Packet HandleGetPartDetails(
            const AeroStock::Common::Packet& requestPacket,
            ServerStateMachine& stateMachine);

        [[nodiscard]] AeroStock::Common::Packet HandleUpdateStock(
            const AeroStock::Common::Packet& requestPacket,
            ServerStateMachine& stateMachine);

        [[nodiscard]] AeroStock::Common::Packet HandleRequestFile(
            const AeroStock::Common::Packet& requestPacket,
            ServerStateMachine& stateMachine);

        [[nodiscard]] AeroStock::Common::Packet CreateSuccessResponse(
            std::uint32_t packetId,
            std::string_view responseText) const;

        [[nodiscard]] AeroStock::Common::Packet CreateErrorResponse(
            std::uint32_t packetId,
            AeroStock::Common::StatusCode status,
            std::string_view errorText) const;

    private:
        Inventory inventory_{};
    };
}