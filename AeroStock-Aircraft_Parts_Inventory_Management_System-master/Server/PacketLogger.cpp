#include "PacketLogger.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "../Common/Enums.h"
#include "../Common/PayloadUtils.h"
#include "../Common/PacketSerializer.h"
#include "../Common/ProtocolConstants.h"

namespace AeroStock::Server
{
    namespace
    {
        // Returns the checksum value that will actually appear on the wire.
        // For received packets, the header already contains the transmitted checksum.
        // For sent packets, serialize the packet and read the checksum field back out.
        [[nodiscard]] std::uint32_t GetLoggedChecksum(
            const std::string_view direction,
            const AeroStock::Common::Packet& packet)
        {
            if (direction != "SENT")
            {
                return packet.GetHeader().checksum;
            }

            const auto serializedPacket =
                AeroStock::Common::PacketSerializer::Serialize(packet);

            if (serializedPacket.size() <
                AeroStock::Common::ProtocolConstants::FixedHeaderSizeBytes)
            {
                return packet.GetHeader().checksum;
            }

            AeroStock::Common::PacketHeader serializedHeader{};
            const bool headerOk =
                AeroStock::Common::PacketSerializer::DeserializeHeader(
                    serializedPacket,
                    serializedHeader);

            if (!headerOk)
            {
                return packet.GetHeader().checksum;
            }

            return serializedHeader.checksum;
        }
    }

    void PacketLogger::LogReceived(const AeroStock::Common::Packet& packet) noexcept
    {
        try
        {
            const std::string logEntry = BuildLogEntry("RECV", packet);

            try
            {
                std::cout << logEntry << std::endl;
            }
            catch (...)
            {
                // Console logging is non-essential. Ignore failures.
            }

            try
            {
                AppendToLogFile(logEntry);
            }
            catch (...)
            {
                // File logging is non-essential. Ignore failures.
            }
        }
        catch (...)
        {
            // Logging must never terminate request processing.
        }
    }

    void PacketLogger::LogSent(const AeroStock::Common::Packet& packet) noexcept
    {
        try
        {
            const std::string logEntry = BuildLogEntry("SENT", packet);

            try
            {
                std::cout << logEntry << std::endl;
            }
            catch (...)
            {
                // Console logging is non-essential. Ignore failures.
            }

            try
            {
                AppendToLogFile(logEntry);
            }
            catch (...)
            {
                // File logging is non-essential. Ignore failures.
            }
        }
        catch (...)
        {
            // Logging must never terminate request processing.
        }
    }

    std::string PacketLogger::BuildLogEntry(
        const std::string_view direction,
        const AeroStock::Common::Packet& packet)
    {
        const auto& header = packet.GetHeader();
        const auto commandName = AeroStock::Common::ToString(header.commandType);
        const auto statusName = AeroStock::Common::ToString(header.status);
        const std::uint32_t loggedChecksum = GetLoggedChecksum(direction, packet);

        std::ostringstream stream;
        stream
            << "[" << GetCurrentTimestamp() << "] "
            << "[" << direction << "] "
            << "id=" << header.packetId
            << " cmd=" << commandName
            << " status=" << statusName
            << " payloadLen=" << header.payloadLength
            << " checksum=0x" << std::hex << loggedChecksum << std::dec;

        // Log payload as text only if it is short enough to keep the log readable.
        if (packet.HasPayload() && header.payloadLength <= 200U)
        {
            const auto payloadText =
                AeroStock::Common::PayloadUtils::PayloadToString(packet.GetPayload());

            stream << " payload=\"" << payloadText << "\"";
        }

        return stream.str();
    }

    std::string PacketLogger::GetCurrentTimestamp()
    {
        const auto now = std::chrono::system_clock::now();
        const std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

        std::tm localTime{};
#if defined(_WIN32)
        localtime_s(&localTime, &currentTime);
#else
        localTime = *std::localtime(&currentTime);
#endif

        std::ostringstream stream;
        stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
        return stream.str();
    }

    void PacketLogger::AppendToLogFile(const std::string& logEntry)
    {
        std::ofstream logFile(LogFileName, std::ios::app);
        if (!logFile.is_open())
        {
            return;
        }

        logFile << logEntry << '\n';
    }
}