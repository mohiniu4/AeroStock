#include "pch.h"
#include "ClientPacketLogger.h"

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include "Enums.h"
#include "PayloadUtils.h"
#include "PacketSerializer.h"
#include "ProtocolConstants.h"

namespace AeroStock::Client
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

    // The ClientPacketLogger class provides static methods for logging sent and received packets,
    // as well as error messages, to a log file. Each log entry includes a timestamp, packet details,
    // and any relevant payload information for easier debugging and monitoring of client-server communication.
    void ClientPacketLogger::LogSent(const AeroStock::Common::Packet& packet) noexcept
    {
        try
        {
            const std::string logEntry = BuildLogEntry("SENT", packet);
            AppendToLogFile(logEntry);
        }
        catch (...)
        {
        }
    }

    // Logs details of a received packet to the log file. Any exceptions during logging are caught and ignored
    // to avoid disrupting client operation.
    void ClientPacketLogger::LogReceived(const AeroStock::Common::Packet& packet) noexcept
    {
        try
        {
            const std::string logEntry = BuildLogEntry("RECV", packet);
            AppendToLogFile(logEntry);
        }
        catch (...)
        {
        }
    }

    // Logs an error message to the log file with an "ERROR" tag. Exceptions during logging are caught and ignored
    void ClientPacketLogger::LogError(const std::string_view message) noexcept
    {
        try
        {
            const std::string logEntry = BuildErrorEntry(message);
            AppendToLogFile(logEntry);
        }
        catch (...)
        {
        }
    }

    // Builds a log entry string for a given packet, including its direction (sent or received), header details
    std::string ClientPacketLogger::BuildLogEntry(
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

        if (packet.HasPayload() && header.payloadLength <= 200U)
        {
            const auto payloadText =
                AeroStock::Common::PayloadUtils::PayloadToString(packet.GetPayload());

            stream << " payload=\"" << payloadText << "\"";
        }

        return stream.str();
    }

    // Builds a log entry string for an error message, including a timestamp and an "ERROR" tag.
    std::string ClientPacketLogger::BuildErrorEntry(const std::string_view message)
    {
        std::ostringstream stream;
        stream
            << "[" << GetCurrentTimestamp() << "] "
            << "[ERROR] "
            << message;

        return stream.str();
    }

    // Retrieves the current local timestamp as a formatted string for log entries.
    std::string ClientPacketLogger::GetCurrentTimestamp()
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

    // Appends a log entry string to the log file. If the file cannot be opened, the method returns without throwing.
    void ClientPacketLogger::AppendToLogFile(const std::string& logEntry)
    {
        std::ofstream logFile(LogFileName, std::ios::app);
        if (!logFile.is_open())
        {
            return;
        }

        logFile << logEntry << '\n';
    }
}