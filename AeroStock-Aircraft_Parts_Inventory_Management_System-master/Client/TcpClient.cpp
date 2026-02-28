#include "TcpClient.h"

#include <climits>
#include <stdexcept>
#include <string>
#include <vector>

#include "../Client.Core/ClientPacketLogger.h"
#include "../Common/PacketHeader.h"
#include "../Common/PacketSerializer.h"
#include "../Common/ProtocolConstants.h"

namespace AeroStock::Client
{
    namespace
    {
        // send/recv take int lengths on WinSock; guard conversion from size_t.
        [[nodiscard]] int ToIntByteCount(const std::size_t value)
        {
            if (value > static_cast<std::size_t>(INT_MAX))
            {
                throw std::runtime_error("Byte count exceeds supported socket transfer size.");
            }

            return static_cast<int>(value);
        }
    }

    TcpClient::TcpClient() = default;

    TcpClient::~TcpClient()
    {
        Disconnect();
    }

    // Connect to the server at the specified IP address and port.
    void TcpClient::Connect(const std::string& serverIpAddress, const std::uint16_t port)
    {
        if (isConnected_)
        {
            throw std::runtime_error("Client is already connected.");
        }

        try
        {
            InitializeWinsock();
            CreateSocket();
            ConnectSocket(serverIpAddress, port);

            isConnected_ = true;
        }
        catch (...)
        {
            // If any step fails after Winsock or socket creation, clean up immediately
            // so the client object is left in a safe reusable state.
            Disconnect();
            throw;
        }
    }

    // Disconnect from the server and clean up resources. Safe to call multiple times.
    void TcpClient::Disconnect() noexcept
    {
        if (clientSocket_ != INVALID_SOCKET)
        {
            closesocket(clientSocket_);
            clientSocket_ = INVALID_SOCKET;
        }

        if (winsockInitialized_)
        {
            WSACleanup();
            winsockInitialized_ = false;
        }

        isConnected_ = false;
    }

    // Returns true if the client is currently connected to the server.
    bool TcpClient::IsConnected() const noexcept
    {
        return isConnected_;
    }

    // Sends a packet to the server. Throws if the client is not connected or if sending fails.
    void TcpClient::SendPacket(const AeroStock::Common::Packet& packet) const
    {
        if (!isConnected_)
        {
            ClientPacketLogger::LogError("Cannot send packet: client is not connected.");
            throw std::runtime_error("Cannot send packet: client is not connected.");
        }

        try
        {
            const auto serializedPacket = AeroStock::Common::PacketSerializer::Serialize(packet);
            if (serializedPacket.empty())
            {
                ClientPacketLogger::LogError("Failed to serialize outgoing packet.");
                throw std::runtime_error("Failed to serialize outgoing packet.");
            }

            SendAll(serializedPacket.data(), ToIntByteCount(serializedPacket.size()));
            ClientPacketLogger::LogSent(packet);
        }
        catch (const std::exception& exception)
        {
            ClientPacketLogger::LogError(
                std::string("Packet send failed: ") + exception.what());
            throw;
        }
    }

    // Receives a packet from the server. Blocks until a full packet is received or an error occurs.
    AeroStock::Common::Packet TcpClient::ReceivePacket() const
    {
        if (!isConnected_)
        {
            ClientPacketLogger::LogError("Cannot receive packet: client is not connected.");
            throw std::runtime_error("Cannot receive packet: client is not connected.");
        }

        try
        {
            using ByteBuffer = AeroStock::Common::PacketSerializer::ByteBuffer;

            // Read the fixed header first so payload length is known before allocating.
            ByteBuffer headerBuffer(AeroStock::Common::ProtocolConstants::FixedHeaderSizeBytes, 0U);
            ReceiveAll(headerBuffer.data(), ToIntByteCount(headerBuffer.size()));

            AeroStock::Common::PacketHeader header{};
            const bool headerReadSuccessfully =
                AeroStock::Common::PacketSerializer::DeserializeHeader(headerBuffer, header);

            if (!headerReadSuccessfully)
            {
                ClientPacketLogger::LogError("Failed to deserialize server packet header.");
                throw std::runtime_error("Failed to deserialize server packet header.");
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
                ClientPacketLogger::LogError("Failed to deserialize full server packet.");
                throw std::runtime_error("Failed to deserialize full server packet.");
            }

            ClientPacketLogger::LogReceived(packet);
            return packet;
        }
        catch (const std::exception& exception)
        {
            ClientPacketLogger::LogError(
                std::string("Packet receive failed: ") + exception.what());
            throw;
        }
    }

    // The following private helper methods encapsulate the steps of initializing Winsock, creating the socket,
    // connecting to the server, and ensuring that all bytes of a packet are sent/received before returning.
    void TcpClient::InitializeWinsock()
    {
        if (winsockInitialized_)
        {
            return;
        }

        WSADATA winsockData{};
        const int result = WSAStartup(MAKEWORD(2, 2), &winsockData);
        if (result != 0)
        {
            ClientPacketLogger::LogError(
                std::string("WSAStartup failed: ") + std::to_string(result));
            throw std::runtime_error("WSAStartup failed: " + std::to_string(result));
        }

        winsockInitialized_ = true;
    }

    // Create a TCP socket for the client. Throws if socket creation fails.
    void TcpClient::CreateSocket()
    {
        clientSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket_ == INVALID_SOCKET)
        {
            ClientPacketLogger::LogError(
                std::string("Failed to create client socket: ") + std::to_string(WSAGetLastError()));
            throw std::runtime_error(
                "Failed to create client socket: " + std::to_string(WSAGetLastError()));
        }
    }

    // Connect the client socket to the server at the specified IP address and port. Throws if connection fails.
    void TcpClient::ConnectSocket(const std::string& serverIpAddress, const std::uint16_t port)
    {
        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);

        const int addressResult = inet_pton(AF_INET, serverIpAddress.c_str(), &serverAddress.sin_addr);
        if (addressResult != 1)
        {
            ClientPacketLogger::LogError("Invalid server IP address format.");
            throw std::runtime_error("Invalid server IP address format.");
        }

        const int connectResult = connect(
            clientSocket_,
            reinterpret_cast<const sockaddr*>(&serverAddress),
            sizeof(serverAddress));

        if (connectResult == SOCKET_ERROR)
        {
            ClientPacketLogger::LogError(
                std::string("Failed to connect to server: ") + std::to_string(WSAGetLastError()));
            throw std::runtime_error(
                "Failed to connect to server: " + std::to_string(WSAGetLastError()));
        }
    }

    // These methods ensure that the full byte count of the packet data is sent or received before returning.
    void TcpClient::SendAll(const void* const buffer, const int byteCount) const
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
                throw std::runtime_error(
                    "Socket send failed: " + std::to_string(WSAGetLastError()));
            }

            totalBytesSent += bytesSent;
        }
    }

    // Receives bytes into the provided buffer until the specified byte count is reached.
    // Throws if receive fails or server disconnects.
    void TcpClient::ReceiveAll(void* const buffer, const int byteCount) const
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
                throw std::runtime_error("Socket receive failed or server disconnected.");
            }

            totalBytesReceived += bytesReceived;
        }
    }
}