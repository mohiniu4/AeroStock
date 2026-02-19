#pragma once

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <cstdint>

#pragma comment(lib, "Ws2_32.lib")

namespace AeroStock::Server
{
    // manages the TCP listener socket and Winsock
    class TcpServer final
    {
    public:
        explicit TcpServer(std::uint16_t port);
        ~TcpServer();

        TcpServer(const TcpServer&) = delete;
        TcpServer& operator=(const TcpServer&) = delete;

        //initializes winsock and starts listening
        void Start();

        // once the client connect it will start the session
        void AcceptAndProcessSingleClient();

    private:
        void InitializeWinsock();

        void CreateAndBindSocket();

        void BeginListening();

    private:
        std::uint16_t port_;
        SOCKET listenSocket_{ INVALID_SOCKET };
        bool winsockInitialized_{ false };
    };
}
