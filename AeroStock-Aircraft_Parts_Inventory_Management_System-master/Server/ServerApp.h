#pragma once

#include <memory>

namespace AeroStock::Server
{
    class TcpServer;

    // Coordinates top-level server startup and execution flow.
    // For Sprint 1, this class is responsible for starting the
    // TCP server and waiting for an initial client connection.
    class ServerApp final
    {
    public:
        ServerApp();
        ~ServerApp();

        ServerApp(const ServerApp&) = delete;
        ServerApp& operator=(const ServerApp&) = delete;

        // Starts the server application and returns a process exit code.
        [[nodiscard]] int Run();

    private:
        // Prints a simple startup banner for visibility in the server console.
        void PrintStartupBanner() const;

    private:
        std::unique_ptr<TcpServer> tcpServer_;
    };
}
