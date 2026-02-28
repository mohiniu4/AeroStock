#pragma once

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <cstdint>
#include <string>

#include "../Common/Packet.h"

#pragma comment(lib, "Ws2_32.lib")

namespace AeroStock::Client
{
    // Wraps a TCP socket and packet-level transport for the client workflow.
    class TcpClient final
    {
    public:
        TcpClient();
        ~TcpClient();

        TcpClient(const TcpClient&) = delete;
        TcpClient& operator=(const TcpClient&) = delete;

        TcpClient(TcpClient&&) = delete;
        TcpClient& operator=(TcpClient&&) = delete;

        void Connect(const std::string& serverIpAddress, std::uint16_t port);
        void Disconnect() noexcept;

        [[nodiscard]] bool IsConnected() const noexcept;

        void SendPacket(const AeroStock::Common::Packet& packet) const;
        [[nodiscard]] AeroStock::Common::Packet ReceivePacket() const;

    private:
        void InitializeWinsock();
        void CreateSocket();
        void ConnectSocket(const std::string& serverIpAddress, std::uint16_t port);

        // Ensure the full packet frame is transferred before returning.
        void SendAll(const void* buffer, int byteCount) const;
        void ReceiveAll(void* buffer, int byteCount) const;

    private:
        SOCKET clientSocket_{ INVALID_SOCKET };
        bool winsockInitialized_{ false };
        bool isConnected_{ false };
    };
}