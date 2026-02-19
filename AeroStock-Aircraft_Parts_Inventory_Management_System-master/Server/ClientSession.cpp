#include "ClientSession.h"
#include "PacketLogger.h"

#include <climits>
#include <stdexcept>

#include "../Common/Enums.h"
#include "../Common/PacketSerializer.h"
#include "../Common/ProtocolConstants.h"

namespace AeroStock::Server
{
    namespace
    {
        [[nodiscard]] int ToIntByteCount(const std::size_t value)
        {
            if (value > static_cast<std::size_t>(INT_MAX))
            {
                throw std::runtime_error("Byte count exceeds supported socket transfer size.");
            }

            return static_cast<int>(value);
        }
    }

    ClientSession::ClientSession(const SOCKET clientSocket)
        : clientSocket_{ clientSocket },
        stateMachine_{},
        requestHandler_{}
    {
    }

    // receives, handles and responds to packets
    void ClientSession::ProcessSession()
    {
        bool sessionActive = true;

        while (sessionActive)
        {
            try
            {
                const AeroStock::Common::Packet requestPacket = ReceivePacket();
                PacketLogger::LogReceived(requestPacket);

                if (!stateMachine_.CanProcessCommand(requestPacket.GetCommandType()))
                {
                    stateMachine_.MoveToErrorState();

                    const AeroStock::Common::Packet errorResponse =
                        requestHandler_.HandleRequest(requestPacket, stateMachine_);

                    stateMachine_.MoveToSendingDataState();
                    PacketLogger::LogSent(errorResponse);
                    SendPacket(errorResponse);

                    sessionActive = false;
                    continue;
                }

                // Save the last valid session state before entering Processing.
                const AeroStock::Common::ServerState previousState =
                    stateMachine_.GetCurrentState();

                stateMachine_.MoveToProcessingState();

                const AeroStock::Common::Packet responsePacket =
                    requestHandler_.HandleRequest(requestPacket, stateMachine_);

                // Capture the state chosen by the handler.
                AeroStock::Common::ServerState postHandleState =
                    stateMachine_.GetCurrentState();

                stateMachine_.MoveToSendingDataState();

                PacketLogger::LogSent(responsePacket);
                SendPacket(responsePacket);

                // If the handler left us in Processing, that means it returned a normal
                // response without explicitly choosing a new session state.
                // Restore the previous operational state instead of staying stuck.
                if (postHandleState == AeroStock::Common::ServerState::Processing)
                {
                    postHandleState = previousState;
                }

                stateMachine_.RestoreState(postHandleState);

                if (requestPacket.GetCommandType() == AeroStock::Common::CommandType::Disconnect ||
                    postHandleState == AeroStock::Common::ServerState::Error)
                {
                    sessionActive = false;
                }
            }
            catch (...)
            {
                stateMachine_.MoveToErrorState();
                throw;
            }
        }
    }

    // reads the 15byte header then reads the payload based on the headers length field
    AeroStock::Common::Packet ClientSession::ReceivePacket()
    {
        using ByteBuffer = AeroStock::Common::PacketSerializer::ByteBuffer;

        ByteBuffer headerBuffer(AeroStock::Common::ProtocolConstants::FixedHeaderSizeBytes, 0U);
        ReceiveAll(headerBuffer.data(), ToIntByteCount(headerBuffer.size()));

        AeroStock::Common::PacketHeader header{};
        const bool headerReadSuccessfully =
            AeroStock::Common::PacketSerializer::DeserializeHeader(headerBuffer, header);

        if (!headerReadSuccessfully)
        {
            throw std::runtime_error("Failed to deserialize client packet header.");
        }

        const std::size_t payloadLengthBytes = static_cast<std::size_t>(header.payloadLength);

        ByteBuffer fullPacketBuffer{};
        fullPacketBuffer.reserve(
            AeroStock::Common::ProtocolConstants::FixedHeaderSizeBytes + payloadLengthBytes);

        fullPacketBuffer.insert(fullPacketBuffer.end(), headerBuffer.begin(), headerBuffer.end());

        if (payloadLengthBytes > 0U)
        {
            ByteBuffer payloadBuffer(payloadLengthBytes, 0U);
            ReceiveAll(payloadBuffer.data(), ToIntByteCount(payloadBuffer.size()));

            fullPacketBuffer.insert(fullPacketBuffer.end(), payloadBuffer.begin(), payloadBuffer.end());
        }

        AeroStock::Common::Packet packet{};
        const bool packetReadSuccessfully =
            AeroStock::Common::PacketSerializer::Deserialize(fullPacketBuffer, packet);

        if (!packetReadSuccessfully)
        {
            throw std::runtime_error("Failed to deserialize full client packet.");
        }

        return packet;
    }

    void ClientSession::SendPacket(const AeroStock::Common::Packet& packet)
    {
        const auto serializedPacket = AeroStock::Common::PacketSerializer::Serialize(packet);
        if (serializedPacket.empty())
        {
            throw std::runtime_error("Failed to serialize response packet.");
        }

        SendAll(serializedPacket.data(), ToIntByteCount(serializedPacket.size()));
    }

    // loops until all data is received
    void ClientSession::ReceiveAll(void* const buffer, const int byteCount)
    {
        auto* currentBuffer = static_cast<char*>(buffer);
        int totalBytesReceived = 0;

        while (totalBytesReceived < byteCount)
        {
            const int bytesReceived = recv(
                clientSocket_,
                currentBuffer + totalBytesReceived,
                byteCount - totalBytesReceived,
                0);

            if (bytesReceived <= 0)
            {
                throw std::runtime_error("Socket receive failed or client disconnected.");
            }

            totalBytesReceived += bytesReceived;
        }
    }

    void ClientSession::SendAll(const void* const buffer, const int byteCount)
    {
        const auto* currentBuffer = static_cast<const char*>(buffer);
        int totalBytesSent = 0;

        while (totalBytesSent < byteCount)
        {
            const int bytesSent = send(
                clientSocket_,
                currentBuffer + totalBytesSent,
                byteCount - totalBytesSent,
                0);

            if (bytesSent == SOCKET_ERROR)
            {
                throw std::runtime_error("Socket send failed.");
            }

            totalBytesSent += bytesSent;
        }
    }
}