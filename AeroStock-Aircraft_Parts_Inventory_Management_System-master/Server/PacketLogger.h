#pragma once

#include <string>
#include <string_view>

#include "../Common/Packet.h"

namespace AeroStock::Server
{
    // Logs transmitted and received packets for communication tracing.
    // Entries are written to both the console and a server-side log file.
    // Logging failures are intentionally treated as non-fatal so that
    // communication can continue even if tracing is unavailable.
    class PacketLogger final
    {
    public:
        PacketLogger() = delete;

        static void LogReceived(const AeroStock::Common::Packet& packet) noexcept;
        static void LogSent(const AeroStock::Common::Packet& packet) noexcept;

    private:
        [[nodiscard]] static std::string BuildLogEntry(
            std::string_view direction,
            const AeroStock::Common::Packet& packet);

        [[nodiscard]] static std::string GetCurrentTimestamp();

        static void AppendToLogFile(const std::string& logEntry);

    private:
        static inline constexpr const char* LogFileName = "server_packets.log";
    };
}