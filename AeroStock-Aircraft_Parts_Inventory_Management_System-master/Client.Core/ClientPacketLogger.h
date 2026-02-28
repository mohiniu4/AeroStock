#pragma once

#include <string>
#include <string_view>

#include "Packet.h"

namespace AeroStock::Client
{
    class ClientPacketLogger final
    {
    public:
        ClientPacketLogger() = delete;

        static void LogSent(const AeroStock::Common::Packet& packet) noexcept;
        static void LogReceived(const AeroStock::Common::Packet& packet) noexcept;
        static void LogError(std::string_view message) noexcept;

    private:
        [[nodiscard]] static std::string BuildLogEntry(
            std::string_view direction,
            const AeroStock::Common::Packet& packet);

        [[nodiscard]] static std::string BuildErrorEntry(std::string_view message);

        [[nodiscard]] static std::string GetCurrentTimestamp();

        static void AppendToLogFile(const std::string& logEntry);

    private:
        static inline constexpr const char* LogFileName = "client_packets.log";
    };
}