#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Winsock2.h>

#include "../Common/Packet.h"
#include "RequestHandler.h"
#include "ServerStateMachine.h"

namespace AeroStock::Server
{

    // handles the request and response loop for client
    class ClientSession final
    {
    public:
        explicit ClientSession(SOCKET clientSocket);
        ~ClientSession() = default;

        ClientSession(const ClientSession&) = delete;
        ClientSession& operator=(const ClientSession&) = delete;

        ClientSession(ClientSession&&) = delete;
        ClientSession& operator=(ClientSession&&) = delete;

        // runs until the client disconnects or error
        void ProcessSession();

    private:
        [[nodiscard]] AeroStock::Common::Packet ReceivePacket();
        void SendPacket(const AeroStock::Common::Packet& packet);

        void ReceiveAll(void* buffer, int byteCount);
        void SendAll(const void* buffer, int byteCount);

    private:
        SOCKET clientSocket_;
        ServerStateMachine stateMachine_;
        RequestHandler requestHandler_;
    };
}